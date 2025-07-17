/**
 * @file header.h
 * @brief Central header file including libraries, constants, pin definitions, and function prototypes
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

// TODO:
// - Calibrate BRAKE_LIGHT_THRESHOLD based on sensor readings and desired light
// activation point.
// - Calibrate BRAKE_LIGHT_HYSTERESIS for desired behavior.
// - Verify the necessity and logic of using TILT_THRESHOLD_DEG for brake light
// activation;
//   consider using deceleration directly from MPU if required by rules
//   (T6.3.1).
// - Define pins and logic for monitoring critical errors (IMD, BSPD etc.) via
// digital inputs.

#ifndef HEADER_H
#define HEADER_H

// ------------ STANDARD LIBRARIES ------------
#include <cmath>
#include <stdint.h> // For fixed-width integer types

// ------------ ARDUINO LIBRARIES ------------
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

// Undefine Arduino's min/max macros to avoid conflicts with C++ standard library
#undef max
#undef min

#include <limits>

// ------------ EXTERNAL LIBRARIES ------------
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Nextion.h>
#include <due_can.h>

// ------------ PROJECT MODULES ------------
#include "apps.h"        // APPS reading constants/functions
#include "bamocar-due.h" // Bamocar motor controller library
#include "simple_can.h"  // For standardising a central module for CAN comms

// ------------ CONSTANTS ------------
// --- General ---
const int DEBUG_MODE = 4; // 0=Off, 1=Essential, 2=Verbose, 3=Very Verbose, 4=Max Debug

// VCU-HACK: Set to true to bypass BMS checks for bench testing without a BMS.
// MUST BE FALSE FOR VEHICLE OPERATION.
const bool BENCH_TESTING_MODE = true;

// --- Pins ---
// Analog Pins
const int BRAKE_PRESSURE_SENSOR_PIN_FRONT = A0;
const int BRAKE_PRESSURE_SENSOR_PIN_REAR = A1;
const int APPS_1_PIN = A6;
const int APPS_2_PIN = A7;
// Digital Pins
const int BRAKE_LIGHT_PIN = 7;
const int ERROR_PIN_START = 22; // Start pin for error monitoring
const int ERROR_PIN_END = 37;   // End pin for error monitoring

// --- Thresholds & Parameters ---
// Brake System
const int BRAKE_LIGHT_THRESHOLD = 109; // Calibrated
const int BRAKE_LIGHT_HYSTERESIS = 2; // Calibrated
// TODO: Verify necessity/logic/value for tilt activation
const float TILT_THRESHOLD_DEG = 5.8; // For MPU6050 brake light activation -
                                      // Verify necessity/logic (Rule T6.3.1)
const int APPS_BRAKE_PLAUSIBILITY_THRESHOLD =
    25; // % APPS request threshold for brake plausibility check (Rule EV.5.7)

// APPS
const float APPS_PLAUSIBILITY_THRESHOLD = 10.0f; // % difference threshold (Rule EV.5.6)

// Timing
const unsigned long APPS_PLAUSIBILITY_TIMEOUT_MS = 100; // Max time for APPS implausibility (Rule EV.5.6.3)
const unsigned long APPS_BRAKE_PLAUSIBILITY_TIMEOUT_MS = 500; // Max time for APPS/Brake implausibility (Rule EV.2.3.1)

// ------------ GLOBAL OBJECT INSTANCES (declared extern here) ------------
extern Adafruit_MPU6050 mpu;

// ------------ GLOBAL VARIABLES ------------
extern int brakePressureFront;
extern int brakePressureRear;
extern int brakePressureCombined;
extern int brakeIdleValueFront;
extern int brakeIdleValueRear;
extern int brakeIdleValueCombined;
extern int dynamicBrakeThreshold;
extern bool brakeInitialized;
extern int vehicleSpeed;
extern int motorRPM;
extern float batteryVoltage;
extern int motorTemperature;
extern bool mpuInitialized;


// ------------ FUNCTION PROTOTYPES ------------

// --- Sensor/Input Modules ---
double get_apps_reading(); // Returns pedal position (%) or -1.0 on implausibility
void brake_light(); // Reads brake pressure, MPU, controls brake light
void recalibrate_brake_idle(); // Recalibrate brake idle values

// --- Actuator/Control Modules ---
void motor_control_update(); // Handle motor control logic including safety checks

// --- Monitoring/Dashboard Modules ---
void monitor_errors_setup(); // Setup error monitoring
void monitor_errors_loop();  // Error monitoring loop
void dash_setup();           // Setup for Nextion display
void dash_loop();            // Update loop for Nextion display

// --- Utility Functions ---
bool initializeMPU(); // Initialize MPU6050 sensor

#endif // HEADER_H
