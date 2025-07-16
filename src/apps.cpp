/**
 * @file apps.cpp
 * @brief Handles reading and validating Accelerator Pedal Position Sensors (APPS)
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

// TODO:
// - Calibrate PEDAL_VOLTAGE_MIN and PEDAL_VOLTAGE_MAX by measuring actual voltage output
// - Verify ADC_MAX_VALUE and ADC_REF_VOLTAGE match Arduino Due configuration

#include "header.h"
#include <cmath>  // For std::fabs
#include <limits> // For std::numeric_limits

// Define pedal calibration constants
const double PEDAL_VOLTAGE_MIN = 0; // Voltage at 0% pedal travel - CALIBRATE!
const double PEDAL_VOLTAGE_MAX = 3.2; // Voltage at 100% pedal travel - CALIBRATE!
const double ADC_MAX_VALUE = 1023.0; // Max value for Arduino Due's 10-bit ADC
const double ADC_REF_VOLTAGE = 3.3; // ADC reference voltage

/**
 * @brief Reads APPS sensors, checks for plausibility, and returns average pedal position
 * @return Pedal position (0.0 to 100.0) if sensors are plausible, -1.0 if implausible
 */
double get_apps_reading() {
  int apps_1_raw = analogRead(APPS_1_PIN);
  int apps_2_raw = analogRead(APPS_2_PIN);

  // Convert raw ADC values to voltagesn
  double apps_1_voltage = apps_1_raw * ADC_REF_VOLTAGE / ADC_MAX_VALUE;
  double apps_2_voltage = apps_2_raw * ADC_REF_VOLTAGE / ADC_MAX_VALUE;

  // Convert voltages to percentage (0-100) based on calibration
  double pedal_range = PEDAL_VOLTAGE_MAX - PEDAL_VOLTAGE_MIN;
  if (std::fabs(pedal_range) < std::numeric_limits<double>::epsilon()) {
    if (DEBUG_MODE) {
      Serial.println("APPS Error: PEDAL_VOLTAGE_MAX == PEDAL_VOLTAGE_MIN! "
                     "Check Calibration.");
    }
    return -1.0; // Prevent division by zero
  }

  double apps_1_percent = (apps_1_voltage - PEDAL_VOLTAGE_MIN) * 100.0 / pedal_range;
  double apps_2_percent = (apps_2_voltage - PEDAL_VOLTAGE_MIN) * 100.0 / pedal_range;

  // Clamp values to 0-100 range
  apps_1_percent = constrain(apps_1_percent, 0.0, 100.0);
  apps_2_percent = constrain(apps_2_percent, 0.0, 100.0);

  // Check for implausibility (FSUK EV.5.6: deviation > 10%)
  if (std::fabs(apps_1_percent - apps_2_percent) >
      APPS_PLAUSIBILITY_THRESHOLD) {
    if (DEBUG_MODE) {
      Serial.print("APPS Implausibility Detected! APPS1: ");
      Serial.print(apps_1_percent);
      Serial.print("%, APPS2: ");
      Serial.print(apps_2_percent);
      Serial.println("%");
    }
    return -1.0; // Indicate implausibility
  }

  // Return the average percentage if plausible
  double average_percent = (apps_1_percent + apps_2_percent) / 2.0;

  if (DEBUG_MODE >= 4) {
    static unsigned long last_apps_print = 0;
    if (millis() - last_apps_print > 1000) {
      Serial.print("APPS Readings - Raw: ");
      Serial.print(apps_1_raw);
      Serial.print(", ");
      Serial.print(apps_2_raw);
      Serial.print(" | Volts: ");
      Serial.print(apps_1_voltage, 3);
      Serial.print(", ");
      Serial.print(apps_2_voltage, 3);
      Serial.print(" | Percent: ");
      Serial.print(apps_1_percent, 1);
      Serial.print(", ");
      Serial.print(apps_2_percent, 1);
      Serial.print(" | Avg: ");
      Serial.println(average_percent, 1);
      last_apps_print = millis();
    }
  }

  return average_percent;
}
