/**
 * @file main.cpp
 * @brief Main entry point for the UCD Formula Student Electric Vehicle Controller
 * @details Initializes hardware, processes CAN messages, and manages vehicle control logic.
 *          Handles brake light activation based on brake pressure and deceleration.
 *          Monitors motor controller status and updates dashboard display.
 *          Implements basic motor control logic based on pedal position and brake status.
 *          Uses Adafruit MPU6050 for deceleration detection.
 *          Integrates with Nextion display for user interface.
 * @author Shane Whelan (UCD Formula Student)
 * @date 18-07-2025
*/

#include "header.h"

// MPU6050 variables
Adafruit_MPU6050 mpu;
bool mpuInitialized = false;

// Motor controller test mode
// TODO: SET TO FALSE FOR VEHICLE OPERATION
bool BAMOCAR_TEST_MODE = true;

// Global variables
int brakePressure = 0;
int vehicleSpeed = 0;
int motorRPM = 0;
float batteryVoltage = 0;
int motorTemperature = 0;

void setup() {
  WDT_Enable(WDT, WDT_MR_WDRSTEN | WDT_MR_WDDBGHLT | WDT_MR_WDIDLEHLT | WDT_MR_WDV(4095));

  Serial.begin(9600);
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

  if (BAMOCAR_TEST_MODE) {
    Serial.println("BAMOCAR TEST: Requesting controller data...");
    can.requestBamocarData(0x40, 0x64); // STATUS every 100ms
    can.requestBamocarData(0x30, 0x64); // RPM every 100ms  
    can.requestBamocarData(0x90, 0x64); // TORQUE every 100ms
    can.requestBamocarData(0xE2, 0x64); // READY state every 100ms
    can.requestBamocarData(0x49, 0xC8); // MOTOR TEMP every 200ms
    
    // Enable controller for testing
    Serial.println("BAMOCAR TEST: Enabling controller...");
    can.sendBamocarCmd(0x51, 0x01);
    Serial.println("BAMOCAR TEST: Move pedal to send commands");
    Serial.println("-----------------------------------------");
  }
  
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

  nextion_setup(); // Initialize Nextion display
  
  Serial.println("--- Setup Complete ---");
}

void loop() {
  // Reset watchdog (must be called regularly to avoid reset)
  WDT_Restart(WDT);

  static int lastEnableTime = 0;
  if (millis() - lastEnableTime > 1000) {  // Every 1 second
  can.sendBamocarCmd(0x51, 0x01);  // Enable command
  lastEnableTime = millis();
}

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

  // calculate vehicle speed from RPM
  vehicleSpeed = calculateVehicleSpeed(motorRPM);
  
  if (DEBUG_MODE >= 4){
  // Debug output
  static unsigned long last_debug = 0;
  if (millis() - last_debug > 0) {
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
    Serial.print(can.getSystemError() ? "ERROR" : "OK");  // Replace with actual status when available
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
    
      Serial.println();
      Serial.println();
      Serial.println("\n----- BAMOCAR TEST STATUS -----");
      Serial.print("Motor RPM: ");
      Serial.print(can.getMotorRPM());
      Serial.print(" | Torque: ");
      Serial.print(can.getMotorTorque());
      Serial.print("Nm | Temp: ");
      Serial.print(can.getMotorTemp());
      Serial.println("°C");
      Serial.print("Controller Status: 0x");
      Serial.print(can.getSystemError() ? "ERROR" : "OK");
      Serial.print(" | Ready State: ");
      Serial.println(can.getSystemError() ? "NOT READY" : "READY");
      Serial.println("-----------------------------\n");
      last_debug = millis();
    }
  }

    // --- Update Nextion Displays (throttled to a reasonable rate) ---
  static unsigned long last_display_update = 0;
  if (millis() - last_display_update > 20) { // Update displays 50 times a second

    // Update the main driver-facing values
    nextion_update_driver_page();
    
    // Update all the fields on the dev page
    nextion_update_dev_page();
    
    last_display_update = millis();
  }
}
