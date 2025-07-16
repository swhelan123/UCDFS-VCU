/**
 * @file globals.h
 * @brief Declares global variables used across the project.
 * Removed BMS variables as they are now encapsulated in BMSHandler.
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

#ifndef GLOBALS_H
#define GLOBALS_H

// ------------ GLOBAL VARS ------------

extern int brakePressureFront;
extern int brakePressureRear; 
extern int brakePressureCombined;
extern int vehicleSpeed;
extern int motorRPM;
extern float batteryVoltage;
extern int motorTemperature;
extern bool mpuInitialized; 

#endif // GLOBALS_H
