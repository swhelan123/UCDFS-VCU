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

// Keep brakePressure if it's read directly via analogRead and used globally
// If brake pressure reading moves into a dedicated module, remove this too.
extern int brakePressure; // Raw ADC value from brake pressure sensor

// Removed extern float cellVoltage; -> Now accessed via
// bms_handler.get_bms_data().low_cell_voltage etc. Removed extern int
// stateOfCharge; -> Now accessed via bms_handler.get_bms_data().pack_soc

// Add other necessary global variables here

#endif // GLOBALS_H
