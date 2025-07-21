/**
 * @file calc.cpp
 * @brief Contains utility functions for calculations related to vehicle speed and other metrics
 * @details This file provides functions to calculate vehicle speed based on motor RPM and other related metrics.
 *          It is used in the main control loop to update vehicle speed for display and control logic.
 *          The calculations are based on the vehicle's gearing and wheel size.
 *          The vehicle speed is calculated using a fixed conversion factor derived from the motor RPM.
 * @author Shane Whelan (UCD Formula Student)
 * @date 18-07-2025
*/

#include "header.h"

int calculateVehicleSpeed(int motorRPM) {
  // TODO: Adjust this calculation based on your vehicle's gearing and wheel size
  const float RPM_TO_SPEED_FACTOR = 0.01831; // Convert motor RPM to km/h using: km/h = motorRPM * 0.01831 (gear ratio 5.25:1, 510mm tyre diameter)
  return static_cast<int>(motorRPM * RPM_TO_SPEED_FACTOR);
}