/**
 * @file brake_light.cpp
 * @brief Handles brake light activation based on brake pressure and potentially
 * tilt/deceleration.
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

// TODO:
// - Implement deceleration calculation using the MPU6050 sensor data
// (a.acceleration.x).
// - Modify the brake light activation logic to turn the light ON if:
//   (brakePressure > BRAKE_LIGHT_THRESHOLD) OR (calculated_deceleration > 1.0
//   m/s^2). Ensure the 1.0 m/s^2 threshold is correctly implemented (Rule
//   T6.3.1).
// - Consider filtering MPU6050 acceleration data to get a stable deceleration
// value.
// - Re-evaluate the necessity and logic of using the TILT_THRESHOLD_DEG; direct
// deceleration
//   measurement is generally preferred and more aligned with rule T6.3.1.
// - Move MPU initialization to setup() in main.cpp for robustness.

#include "header.h"

// Global variable for brake pressure (raw ADC) - updated here
int brakePressure = 0;

// Global MPU object (assuming it's declared extern in header.h and defined in
// main.cpp)
extern Adafruit_MPU6050 mpu;

// MPU Initialization state flag
extern bool mpuInitialized; // Make this extern if defined/set in main.cpp after
                            // initialization

//------------------------------------------------------------------------------
// Initialize MPU6050 Sensor
//------------------------------------------------------------------------------
// TODO: Move this function's implementation and call to setup() in main.cpp
// Keep declaration here if needed by other functions in this file, or make
// static if only used here.
void initializeMPU() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 sensor!");
    // Avoid infinite loop in production code; set an error flag or retry
    // mechanism while (1) { delay(10); } // Halt is bad during operation
    // mpuInitialized = false; // Set by main.cpp
  } else {
    if (DEBUG_MODE) {
      Serial.println("MPU6050 sensor initialized.");
    }
    // mpuInitialized = true; // Set by main.cpp
    // Optionally set the sensor range (e.g., higher range if needed for
    // accel/decel)
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G); // Example: +/- 4G range
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); // Example: Apply some filtering
  }
}

//------------------------------------------------------------------------------
// Brake Light Control Function
//------------------------------------------------------------------------------
void brake_light() {
  // Read brake pressure from the sensor
  brakePressure = analogRead(BRAKE_PRESSURE_SENSOR_PIN);
  if (DEBUG_MODE >= 2) { // Reduce frequency of this print
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
      Serial.print("Brake Pressure (Raw): ");
      Serial.println(brakePressure);
      lastPrint = millis();
    }
  }

  // TODO: Remove this check if initializeMPU() is reliably called in setup()
  // if (!mpuInitialized) {
  //   initializeMPU(); // Initialization should happen in setup()
  // }

  float deceleration_m_s2 = 0.0f;
  float tiltAngle =
      0.0f; // Keep tilt calculation if still desired for other purposes

  if (mpuInitialized) { // Check if MPU was successfully initialized in setup()
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp); // Read sensor data

    // TODO: Calculate Deceleration (Negative Acceleration along the vehicle's
    // forward axis) Assuming a.acceleration.x is the forward/backward axis.
    // Verify sensor orientation. Positive x might be acceleration, negative x
    // deceleration, or vice-versa. CALIBRATE/VERIFY! Example: Assuming negative
    // x is deceleration
    deceleration_m_s2 = -a.acceleration.x;
    // Add filtering here if needed (e.g., simple moving average)

    // Keep tilt calculation if needed, but prioritize deceleration for brake
    // light
    float ax_g = a.acceleration.x / 9.81f;
    float ay_g = a.acceleration.y / 9.81f;
    float az_g = a.acceleration.z / 9.81f;
    // TODO: Verify tilt calculation math and necessity
    tiltAngle = atan2(ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / PI;

    if (DEBUG_MODE >= 2) {
      static unsigned long lastMPUPrint = 0;
      if (millis() - lastMPUPrint > 500) {
        Serial.print("MPU Accel X: ");
        Serial.print(a.acceleration.x, 2);
        Serial.print(" m/s^2 -> Decel: ");
        Serial.print(deceleration_m_s2, 2);
        Serial.print(" m/s^2 | Tilt: ");
        Serial.print(tiltAngle, 1);
        Serial.println(" deg");
        lastMPUPrint = millis();
      }
    }
  }

  // Determine brake light state
  bool activate_brake_light = false;

  // Condition 1: Hydraulic pressure threshold (Rule T6.3.1)
  if (brakePressure > BRAKE_LIGHT_THRESHOLD) {
    activate_brake_light = true;
  }

  // Condition 2: Deceleration threshold due to regen (Rule T6.3.1)
  // TODO: Implement this check using calculated deceleration
  const float REGEN_DECEL_THRESHOLD =
      1.0f; // m/s^2 +/- 0.3 m/s^2 tolerance implied by rule
  if (deceleration_m_s2 > REGEN_DECEL_THRESHOLD) {
    activate_brake_light = true;
    if (DEBUG_MODE >= 2)
      Serial.println("Brake Light ON (Regen Decel)");
  }

  // Condition 3: Tilt (Optional - Re-evaluate necessity vs Rule T6.3.1)
  // TODO: Decide if tilt activation is still required/desired. Remove if not.
  // if (tiltAngle >= TILT_THRESHOLD_DEG) {
  //    activate_brake_light = true;
  //    if (DEBUG_MODE >= 2) Serial.println("Brake Light ON (Tilt)");
  // }

  // Apply hysteresis for turning the light OFF
  static bool brake_light_on = false;
  if (activate_brake_light) {
    if (!brake_light_on) { // Print only when state changes to ON
      if (DEBUG_MODE)
        Serial.println("Brake Light ON");
    }
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
    brake_light_on = true;

  } else {
    // Only turn off if pressure is below threshold minus hysteresis AND
    // deceleration is below threshold AND tilt is below threshold (if used)
    // TODO: Add hysteresis for deceleration condition if implemented
    bool turn_off =
        brakePressure < (BRAKE_LIGHT_THRESHOLD - BRAKE_LIGHT_HYSTERESIS);
    // if (deceleration_m_s2 >= REGEN_DECEL_THRESHOLD * 0.8f) turn_off = false;
    // // Example hysteresis if (tiltAngle >= (TILT_THRESHOLD_DEG - 1.0f))
    // turn_off = false; // Example hysteresis

    if (brake_light_on && turn_off) {
      digitalWrite(BRAKE_LIGHT_PIN, LOW);
      brake_light_on = false;
      if (DEBUG_MODE)
        Serial.println("Brake Light OFF");
    }
    // If already off, do nothing. If hysteresis conditions not met, keep it on.
  }
}
