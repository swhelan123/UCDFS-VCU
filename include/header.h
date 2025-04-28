/**
 * @file header.h
 * @brief Central header file including libraries, constants, pin definitions,
 * and function prototypes.
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
#include <limits>
#include <stdint.h> // For fixed-width integer types

// ------------ ARDUINO LIBRARIES ------------
#include <Arduino.h> // Include Arduino core functionality (pinMode, analogRead, etc.)
#include <SPI.h>  // Required by due_can? Keep if needed.
#include <Wire.h> // For I2C devices like MPU6050

// ------------ EXTERNAL LIBRARIES ------------
#include <Adafruit_MPU6050.h> // For MPU6050 sensor
#include <Adafruit_Sensor.h>  // Required by Adafruit MPU6050 library
#include <Nextion.h>          // For Nextion display (if used)
#include <due_can.h>          // CAN library for Arduino Due

// ------------ PROJECT MODULES ------------
#include "apps.h"        // APPS reading constants/functions
#include "bamocar-due.h" // Bamocar motor controller library
#include "bms_handler.h" // BMS data handler
#include "can_manager.h" // CAN bus manager
#include "globals.h"     // Global variable declarations

// ------------ CONSTANTS ------------
// --- General ---
const int DEBUG_MODE = 1; // 0=Off, 1=On: Enables Serial print messages

// --- Pins ---
// Analog Pins
const int BRAKE_PRESSURE_SENSOR_PIN = A0;
const int APPS_1_PIN = A6;
const int APPS_2_PIN = A7;
// Digital Pins
const int BRAKE_LIGHT_PIN = 7;
// TODO: Define pins used for monitoring critical errors (IMD, BSPD, etc.)
const int ERROR_PIN_START = 22; // Example start pin for error monitoring
const int ERROR_PIN_END = 37;   // Example end pin for error monitoring
// const int IMD_FAULT_PIN = 22; // Example specific pin
// const int BSPD_FAULT_PIN = 23; // Example specific pin

// --- Thresholds & Parameters ---
// Brake System
// TODO: Calibrate these thresholds based on sensor readings
const int BRAKE_LIGHT_THRESHOLD = 500; // Raw ADC value - Calibrate!
const int BRAKE_LIGHT_HYSTERESIS = 15; // Raw ADC value - Calibrate!
// TODO: Verify necessity/logic/value for tilt activation
const float TILT_THRESHOLD_DEG = 5.8; // For MPU6050 brake light activation -
                                      // Verify necessity/logic (Rule T6.3.1)
const int APPS_BRAKE_PLAUSIBILITY_THRESHOLD =
    25; // % APPS request threshold for brake plausibility check (Rule EV.5.7)

// APPS
const float APPS_PLAUSIBILITY_THRESHOLD =
    10.0f; // % difference threshold (Rule EV.5.6)

// Timing
const unsigned long APPS_PLAUSIBILITY_TIMEOUT_MS =
    100; // Max time for APPS implausibility (Rule EV.5.6.3)
const unsigned long APPS_BRAKE_PLAUSIBILITY_TIMEOUT_MS =
    500; // Max time for APPS/Brake implausibility (Rule EV.2.3.1)

// ------------ GLOBAL OBJECT INSTANCES (declared extern here) ------------
// These are defined in their respective .cpp files or main.cpp
extern CANManager can_manager;
extern BMSHandler bms_handler;
extern Bamocar bamocar;
extern Adafruit_MPU6050 mpu; // If MPU6050 is used globally

// ------------ FUNCTION PROTOTYPES ------------

// --- Core Modules ---
// void can_manager_initialize(uint32_t baudrate); // Now part of CANManager
// class void can_manager_process_incoming();           // Now part of
// CANManager class void bms_handler_initialize();                 // Now part
// of BMSHandler class

// --- Sensor/Input Modules ---
double
get_apps_reading(); // Returns pedal position (%) or -1.0 on implausibility
void brake_light(); // Reads brake pressure, MPU, controls brake light

// --- Actuator/Control Modules ---
void motor_control_update(); // New function to handle motor control logic
                             // including safety checks
// void send_torque_request(double torqueRequest); // Integrated into
// motor_control_update

// --- Monitoring/Dashboard Modules ---
void monitor_errors_setup(); // Renamed from monitor_pins_setup
void monitor_errors_loop();  // Renamed from monitor_pins_loop
void dash_setup();           // Setup for Nextion display (if used)
void dash_loop();            // Update loop for Nextion display (if used)

// --- Utility Functions ---
// Add any other helper function prototypes here

#endif // HEADER_H
