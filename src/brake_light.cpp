/*

brake_light.cpp
Written by Shane Whelan
UCD Formula Student

*/

#include "header.h"

int brakePressure;

// Create an Adafruit MPU6050 object
Adafruit_MPU6050 mpu;
bool mpuInitialized = false;

// Function to initialize the MPU6050 sensor
void initializeMPU() {
  if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 sensor!");
      while (1) { delay(10); }
    mpuInitialized = false;
  } else {
    if (DEBUG_MODE) {
      Serial.println("MPU6050 sensor initialized.");
    }
    mpuInitialized = true;
    // Optionally set the sensor range (default is ±2g)
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  }
}

void brake_light() {
  // Read brake pressure from the sensor
  brakePressure = analogRead(BRAKE_PRESSURE_SENSOR_PIN);
  if (DEBUG_MODE) {
    Serial.print("Brake Pressure: ");
    Serial.println(brakePressure);
  }

  // Initialize the MPU6050 if it hasn't been initialized yet
  if (!mpuInitialized) {
    initializeMPU();
  }
  
  float tiltAngle = 0.0;
  if (mpuInitialized) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    // Convert acceleration from m/s^2 to g's (1g ~ 9.81 m/s^2)
    float ax_g = a.acceleration.x / 9.81;
    float ay_g = a.acceleration.y / 9.81;
    float az_g = a.acceleration.z / 9.81;
    
    // Calculate the tilt angle around the X-axis:
    // tilt = arctan(ax / sqrt(ay^2 + az^2)) converted to degrees.
    tiltAngle = atan2(ax_g, sqrt(ay_g * ay_g + az_g * az_g)) * 180.0 / PI;
    
    if (DEBUG_MODE) {
      Serial.print("Tilt Angle: ");
      Serial.print(tiltAngle);
      Serial.println(" deg");
    }
  }
  
  // Activate the brake light if either:
  // - Brake pressure is above the defined threshold, OR
  // - The car is tilted forward by at least TILT_THRESHOLD degrees.
  if (brakePressure > BRAKE_LIGHT_THRESHOLD || tiltAngle >= TILT_THRESHOLD) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
    if (DEBUG_MODE) Serial.println("Brake Light On");
  } else if (brakePressure < BRAKE_LIGHT_THRESHOLD - HYSTERESIS) {
    digitalWrite(BRAKE_LIGHT_PIN, LOW);
    if (DEBUG_MODE) Serial.println("Brake Light Off");
  }
}
