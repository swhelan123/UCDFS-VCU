#include "header.h"

// Global variables
int brakePressure = 0;
int vehicleSpeed = 0;
int motorRPM = 0;
float batteryVoltage = 0;
int motorTemperature = 0;

// MPU6050 variables
Adafruit_MPU6050 mpu;
bool mpuInitialized = false;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000);  // Wait max 2 seconds
  Serial.println("\n--- UCD FS EV Controller Starting ---");
  
  // Initialize pins
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);
  digitalWrite(BRAKE_LIGHT_PIN, LOW);
  
  // Initialize CAN
  can.setDebugLevel(0);  // Maximum debug output initially
  if (!can.begin(500000)) {
    Serial.println("FATAL: CAN initialization failed!");
  } else {
    Serial.println("CAN initialized successfully");
  }
  
  // Initialize error monitoring pins
  monitor_errors_setup();
  
  // Initialize MPU6050
  mpuInitialized = initializeMPU();
  if (!mpuInitialized) {
    Serial.println("WARNING: MPU6050 initialization failed - brake light deceleration detection disabled");
  }
  
  // Request initial data from motor controller
  Serial.println("Requesting initial motor data...");
  can.requestBamocarData(0x30, 0x64);  // RPM
  can.requestBamocarData(0x90, 0x64);  // Torque
  can.requestBamocarData(0x49, 0xC8);  // Motor temp
  can.requestBamocarData(0x40, 0x64);  // Status
  
  Serial.println("--- Setup Complete ---");
}

void loop() {
  // Process CAN messages
  can.update();
  
  // Handle brake light logic
  brake_light();
  
  // Read pedal position
  double pedal_position = get_apps_reading();
  
  // Update global variables from CAN
  motorRPM = can.getMotorRPM();
  batteryVoltage = can.getPackVoltage();
  motorTemperature = can.getMotorTemp();
  
  // Basic motor control
  bool brake_active = (brakePressureCombined > dynamicBrakeThreshold);
  if (pedal_position >= 0 && !brake_active && !can.getSystemError()) {
    // Only allow torque when pedal is valid and brake is not active
    uint16_t torqueCmd = (pedal_position / 100.0) * 32767;
    can.sendBamocarCmd(0x90, torqueCmd);
  } else {
    // Safety condition: set zero torque
    can.sendBamocarCmd(0x90, 0);
  }
  
  // Monitor errors
  monitor_errors_loop();
  
  // Debug output
  static unsigned long last_debug = 0;
  if (millis() - last_debug > 1000) {
    Serial.print("[STATUS] BMS: ");
    Serial.print(can.getPackVoltage(), 1);
    Serial.print("V, ");
    Serial.print(can.getPackSOC());
    Serial.print("%, Fault:");
    Serial.print(can.getSystemError() ? "YES" : "NO");
    Serial.print(" | Rear Brake:");
    Serial.print(brakePressureRear);
    Serial.print(" | Front Brake:");
    Serial.print(brakePressureFront);
    Serial.print(" | Bamocar:0x");
    Serial.print("STATUS");  // Replace with actual status when available
    Serial.print(" | Brake Light: ");
    Serial.print(digitalRead(BRAKE_LIGHT_PIN) ? "ON" : "OFF");
    Serial.print(" | APPS: ");
    Serial.print(pedal_position);
    Serial.print(" |  Torque Request: ");
    int torqueInt = (int)((pedal_position / 100.0) * 32767);
    Serial.print(torqueInt);
    Serial.print("  | Combined Brakes: ");
    Serial.print(brakePressureCombined);
    Serial.print("  | Brake Light: ");
    Serial.print(digitalRead(BRAKE_LIGHT_PIN) ? "ON" : "OFF");
    Serial.print("  | MPU Angle: ");
    if (mpuInitialized) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      Serial.print(a.acceleration.x, 1);
      Serial.print(", ");
      Serial.print(a.acceleration.y, 1);
      Serial.print(", ");
      Serial.print(a.acceleration.z, 1);
    } else {
      Serial.println("MPU not initialized");
    }
    Serial.println("  | Deceleration: ");

    last_debug = millis();
  }
}