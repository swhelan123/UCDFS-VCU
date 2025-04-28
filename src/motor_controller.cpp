/**
 * @file motor_controller.cpp
 * @brief Handles motor control logic, including APPS reading, safety checks,
 * off-throttle regenerative braking, and sending torque commands.
 * @author Shane Whelan (UCD Formula Student) // Original: ShengXin Chen
 * (Jerry), Shane Whelan
 * @date 2025-04-27
 */

// TODO:
// - Implement checks for critical error pins (IMD, BSPD faults, etc.) in
// section 5
//   of motor_control_update(). Define which pins correspond to which faults
//   and ensure they trigger zero torque when active.
// - Verify the logic for clearing latched fault states (APPS plausibility,
// APPS/Brake)
//   matches FSUK rule requirements (e.g., requiring LVMS cycle for some
//   faults).
// - Consider adding checks for Bamocar status flags (received via CAN) if
// needed
//   for safety interlocks (e.g., check bamocar.getStatus() for fault bits).
// - Calibrate REGEN_DESIRED_TORQUE_FRACTION for desired off-throttle feel.
// - Verify motor speed to rad/s conversion factor if needed.
// - Add logic to brake_light.cpp to activate light based on deceleration during
// regen (Rule T6.3.1).

#include "bamocar-due.h" // Bamocar library interface
#include "bms_handler.h" // To get BMS status for safety checks
#include "header.h"
#include <Arduino.h> // For millis(), PI
#include <cmath>     // For std::fabs

// Define the global Bamocar instance (used by CANManager and potentially
// elsewhere)
Bamocar bamocar;

// State variables for safety checks
static bool apps_implausibility_active = false;
static unsigned long apps_implausibility_start_time = 0;
static bool apps_brake_implausibility_active = false;
static unsigned long apps_brake_implausibility_start_time = 0;

// Regen Configuration
// TODO: Calibrate this value for desired off-throttle braking feel
const float REGEN_DESIRED_TORQUE_FRACTION =
    -0.15f; // e.g., -15% torque for regen
const float APPS_REGEN_THRESHOLD =
    5.0f; // APPS % below which off-throttle regen is considered
const float MIN_SPEED_FOR_REGEN_RPM =
    100.0f; // Minimum motor RPM to apply regen (prevent issues at stall)

//------------------------------------------------------------------------------
// Motor Control Update Function
//------------------------------------------------------------------------------
/**
 * @brief Main function to update motor control state.
 * Reads APPS, performs safety checks (APPS Plausibility, APPS/Brake, BMS),
 * calculates desired torque (positive or negative for regen), limits regen
 * based on BMS CCL, and sends the appropriate torque command to the Bamocar.
 * This should be called repeatedly in the main loop.
 */
void motor_control_update() {
  double torque_request_percent =
      0.0; // APPS reading (0-100) or -1.0 if implausible
  bool send_zero_torque =
      false; // Flag to force sending zero torque due to faults
  float final_torque_fraction = 0.0f; // Final command (-1.0 to 1.0)

  // --- 1. Read APPS Sensor ---
  torque_request_percent = get_apps_reading();

  // --- 2. APPS Plausibility Check (Rule EV.5.6) ---
  if (torque_request_percent < 0.0) { // Implausibility detected
    if (!apps_implausibility_active) {
      apps_implausibility_active = true;
      apps_implausibility_start_time = millis();
      if (DEBUG_MODE)
        Serial.println("MOTOR CTRL: APPS Plausibility Fault Started.");
    }
    if (millis() - apps_implausibility_start_time >=
        APPS_PLAUSIBILITY_TIMEOUT_MS) {
      send_zero_torque = true;
      if (DEBUG_MODE)
        Serial.println(
            "MOTOR CTRL: APPS Plausibility Timeout - Zero Torque Latched.");
      // TODO: Consider LVMS cycle requirement for reset
    } else {
      send_zero_torque = true; // Immediate zero torque within timeout
    }
  } else { // APPS Plausible
    if (apps_implausibility_active) {
      if (DEBUG_MODE)
        Serial.println("MOTOR CTRL: APPS Plausibility Fault Cleared.");
      // TODO: Verify reset logic if latching requires LVMS cycle
    }
    apps_implausibility_active = false;
  }

  // --- 3. APPS / Brake Plausibility Check (Rule EV.2.3.1 / EV.5.7) ---
  bool brake_active = (brakePressure > BRAKE_LIGHT_THRESHOLD);
  // Use plausible APPS value for this check, default to 0 if implausible but
  // not yet timed out
  double apps_for_brake_check =
      (torque_request_percent >= 0.0) ? torque_request_percent : 0.0;

  if (!send_zero_torque && brake_active &&
      apps_for_brake_check > APPS_BRAKE_PLAUSIBILITY_THRESHOLD) {
    if (!apps_brake_implausibility_active) {
      apps_brake_implausibility_active = true;
      apps_brake_implausibility_start_time = millis();
      if (DEBUG_MODE)
        Serial.println("MOTOR CTRL: APPS/Brake Plausibility Fault Started.");
    }
    if (millis() - apps_brake_implausibility_start_time >=
        APPS_BRAKE_PLAUSIBILITY_TIMEOUT_MS) {
      send_zero_torque = true;
      if (DEBUG_MODE)
        Serial.println("MOTOR CTRL: APPS/Brake Plausibility Timeout - Zero "
                       "Torque Latched.");
    } else {
      send_zero_torque = true; // Immediate zero torque
    }
  } else { // Condition not met OR condition cleared
    if (apps_brake_implausibility_active) {
      // Only clear if APPS < 5% (Rule EV.2.3.2)
      if (torque_request_percent >= 0.0 && torque_request_percent < 5.0) {
        apps_brake_implausibility_active = false;
        if (DEBUG_MODE)
          Serial.println(
              "MOTOR CTRL: APPS/Brake Plausibility Fault Cleared (APPS < 5%).");
      } else {
        // Still braking or APPS > 5%, keep forcing zero torque if latched
        if (millis() - apps_brake_implausibility_start_time >=
            APPS_BRAKE_PLAUSIBILITY_TIMEOUT_MS) {
          send_zero_torque = true;
          if (DEBUG_MODE >= 2)
            Serial.println("MOTOR CTRL: APPS/Brake Fault Active, APPS >= 5%");
        }
      }
    }
    // If fault wasn't latched (i.e. just started), it clears immediately when
    // condition is no longer met.
  }

  // --- 4. Check BMS Status (Rule EV5.8) ---
  const BMSData &bms_data = bms_handler.get_bms_data();
  // TODO: Ensure bms_handler.has_critical_fault() is correctly implemented
  if (!send_zero_torque && (bms_handler.has_critical_fault() ||
                            !bms_handler.is_communication_active())) {
    send_zero_torque = true;
    if (DEBUG_MODE) {
      if (bms_handler.has_critical_fault())
        Serial.println(
            "MOTOR CTRL: BMS Critical Fault Detected - Zero Torque.");
      if (!bms_handler.is_communication_active(1000))
        Serial.println("MOTOR CTRL: BMS Communication Lost - Zero Torque.");
    }
  }

  // --- 5. Check Monitored Error Pins ---
  // TODO: Implement checks for critical error signals (IMD, BSPD faults, etc.)
  // Example:
  // extern int error22; // IMD_FAULT_PIN
  // extern int error23; // BSPD_FAULT_PIN
  // if (!send_zero_torque && error22 == 1) {
  //     send_zero_torque = true;
  //     if (DEBUG_MODE) Serial.println("MOTOR CTRL: IMD Fault Active - Zero
  //     Torque.");
  // }
  // if (!send_zero_torque && error23 == 1) {
  //     send_zero_torque = true;
  //     if (DEBUG_MODE) Serial.println("MOTOR CTRL: BSPD Fault Active - Zero
  //     Torque.");
  // }

  // --- 6. Determine Torque Command (Acceleration or Regen) ---
  if (send_zero_torque) {
    final_torque_fraction = 0.0f;
  } else if (torque_request_percent < APPS_REGEN_THRESHOLD) {
    // --- Off-Throttle Regen Logic ---
    float motor_speed_rpm = bamocar.getSpeed(); // Get speed in RPM

    // Only apply regen if speed is sufficient and no faults active
    if (motor_speed_rpm > MIN_SPEED_FOR_REGEN_RPM) {
      // Get Limits from BMS
      float current_ccl = bms_data.charge_current_limit; // Amps
      float pack_voltage = bms_data.pack_voltage;        // Volts

      // Basic check for valid BMS data
      if (current_ccl > 0 && pack_voltage > 0) {
        // Calculate Max Regen Power based on CCL
        float max_regen_power = current_ccl * pack_voltage; // Watts

        // Calculate Max Regen Torque based on Power and Speed
        // Power = Torque (Nm) * Speed (rad/s)
        // TODO: Verify this conversion factor if needed
        float motor_speed_rad_s = motor_speed_rpm * (2.0f * PI / 60.0f);
        float max_regen_torque_limit = 0.0f;
        if (std::fabs(motor_speed_rad_s) >
            0.1f) { // Avoid division by zero/tiny numbers
          max_regen_torque_limit =
              max_regen_power / std::fabs(motor_speed_rad_s);
        }

        // Limit the desired regen torque by the calculated max allowed torque
        // Ensure final torque is negative or zero
        // TODO: Implement/Verify bamocar.getMaxTorqueNm() or use constant
        float max_motor_torque = bamocar.getMaxTorqueNm();
        if (max_motor_torque <= 0)
          max_motor_torque =
              80.0f; // Safety default if function fails/not implemented
        final_torque_fraction = max(REGEN_DESIRED_TORQUE_FRACTION,
                                    -max_regen_torque_limit / max_motor_torque);

        if (DEBUG_MODE >= 2) {
          Serial.print("Regen Calc: Desired=");
          Serial.print(REGEN_DESIRED_TORQUE_FRACTION);
          Serial.print(", CCL=");
          Serial.print(current_ccl);
          Serial.print(", Vpack=");
          Serial.print(pack_voltage);
          Serial.print(", RPM=");
          Serial.print(motor_speed_rpm);
          Serial.print(", MaxP=");
          Serial.print(max_regen_power);
          Serial.print(", MaxT_Nm=");
          Serial.print(max_regen_torque_limit);
          Serial.print(", FinalT_Frac=");
          Serial.println(final_torque_fraction);
        }

      } else {
        // Invalid BMS data for calculation, default to zero regen
        final_torque_fraction = 0.0f;
        if (DEBUG_MODE)
          Serial.println(
              "MOTOR CTRL: Regen skipped - Invalid BMS CCL/Voltage data.");
      }
    } else {
      // Speed too low for regen
      final_torque_fraction = 0.0f;
      if (DEBUG_MODE >= 2)
        Serial.println("MOTOR CTRL: Regen skipped - Speed too low.");
    }

  } else {
    // --- Acceleration Logic ---
    // APPS is pressed and no faults active
    final_torque_fraction = torque_request_percent / 100.0f;
    // Clamp just in case
    final_torque_fraction = constrain(final_torque_fraction, 0.0f, 1.0f);
  }

  // Final safety clamp
  final_torque_fraction = constrain(final_torque_fraction, -1.0f, 1.0f);

  // --- 7. Send Torque Command to Bamocar ---
  if (!bamocar.setTorque(final_torque_fraction)) {
    if (DEBUG_MODE) {
      Serial.println(
          "MOTOR CTRL: Failed to send torque command via CANManager.");
    }
  }

  if (DEBUG_MODE) {
    Serial.print("MOTOR CTRL: Final Torque Command: ");
    Serial.print(final_torque_fraction * 100.0, 1);
    Serial.println("%");
  }

  // --- 8. Request Feedback from Bamocar (Periodic) ---
  static unsigned long last_status_request_time = 0;
  unsigned long now = millis();
  // TODO: Adjust request frequency as needed
  if (now - last_status_request_time > 200) { // Request status every 200ms
    bamocar.requestStatus(INTVL_IMMEDIATE);
    bamocar.requestMotorTemp(INTVL_IMMEDIATE);
    bamocar.requestControllerTemp(INTVL_IMMEDIATE);
    bamocar.requestSpeed(
        INTVL_IMMEDIATE); // Request speed needed for regen calc
    // Add other requests as needed
    last_status_request_time = now;
  }
}
