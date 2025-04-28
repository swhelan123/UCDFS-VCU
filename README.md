# UCD Formula Student EV Controller - TODO List

This document summarizes the outstanding tasks, calibration points, and verification steps required for the Arduino Due based EV controller software.

---

## File: `src/can_manager.cpp`

- [ ] **Verify and add CAN filters:** In `setup_filters()`, add filters for ALL required BMS message IDs based on your specific Orion BMS configuration.
- [ ] **Confirm `Can0.read()` behavior:** Check the `due_can` library documentation or test its behavior regarding reading from multiple filtered mailboxes.

## File: `include/bms_handler.h`

- [ ] **Review `BMSData` struct:** Verify, add, or remove fields in the `BMSData` struct to match the exact data you need from your Orion BMS 2 configuration.
- [ ] **Update placeholder comments:** Ensure comments accurately reflect the implementation status after changes.

## File: `src/bms_handler.cpp`

- [ ] **Implement BMS Parsing:** Replace ALL placeholder parsing logic in `parse_bms_message_X` functions with the correct decoding (byte order, data types, scaling, offsets) based on your specific Orion BMS 2 configuration and CAN documentation.
- [ ] **Implement Fault Checking:** Implement the `has_critical_fault()` function based on the actual fault flags and critical limits defined by the BMS and FSUK rules (EV5.8.7, EV5.8.10).
- [ ] **Add Parsing Functions:** Add parsing functions (`parse_bms_message_X`) for all required BMS message IDs.
- [ ] **Initialize `BMSData`:** Review and set appropriate default/safe initial values in the `BMSHandler` constructor.

## File: `lib/bamocar-due/bamocar-due.cpp`

- [ ] **Verify `getCurrent()`:** Verify the calculation logic and scaling factors used in `getCurrent()` against the specific Bamocar D3 CAN documentation for registers `REG_I_ACTUAL`, `REG_I_DEVICE`, `REG_I_200PC`.
- [ ] **Verify `setSoftEnable()`:** Verify the exact data bytes required for the `setSoftEnable()` command (`REG_ENABLE`, 0x51) based on the Bamocar D3 manual. The current implementation uses example values.
- [ ] **Verify `getSpeed()`:** Verify the scaling and interpretation of `N_ACTUAL` in `getSpeed()` against the Bamocar manual.
- [ ] **Verify Parsing Lengths:** Verify the message length assumptions (`msg.length <= 4` for 16-bit, `> 4` for 32-bit) in `_parseMessage()` against the Bamocar manual if communication issues arise.

## File: `include/header.h`

- [ ] **Calibrate `BRAKE_LIGHT_THRESHOLD`:** Determine the appropriate raw ADC value based on sensor readings and desired light activation point.
- [ ] **Calibrate `BRAKE_LIGHT_HYSTERESIS`:** Set the hysteresis value for desired brake light off behavior.
- [ ] **Verify Tilt Logic:** Verify the necessity and logic of using `TILT_THRESHOLD_DEG` for brake light activation; consider using deceleration directly from MPU if required by rules (T6.3.1).
- [ ] **Define Error Pins:** Define constants for pins used for monitoring critical errors (IMD, BSPD etc.) and implement logic to use them.

## File: `src/apps.cpp`

- [ ] **Calibrate APPS Voltages:** Accurately calibrate `PEDAL_VOLTAGE_MIN` and `PEDAL_VOLTAGE_MAX` by measuring the actual voltage output from each APPS sensor at 0% and 100% pedal travel.
- [ ] **Verify ADC Settings:** Verify `ADC_MAX_VALUE` and `ADC_REF_VOLTAGE` match the Arduino Due's configuration (default is 10-bit ADC (0-1023) with 3.3V reference, but can be changed).

## File: `src/motor_controller.cpp`

- [ ] **Implement Error Pin Checks:** Implement checks for critical error signals (IMD, BSPD faults, etc.) in section 5 of `motor_control_update()`. Define which pins correspond to which faults and ensure they trigger zero torque when active.
- [ ] **Verify Fault Reset Logic:** Verify the logic for clearing latched fault states (APPS plausibility, APPS/Brake) matches FSUK rule requirements (e.g., requiring LVMS cycle for some faults).
- [ ] **Check Bamocar Status:** Consider adding checks for Bamocar status flags (received via CAN using `bamocar.getStatus()`) if needed for safety interlocks.
- [ ] **Calibrate Regen Torque:** Calibrate `REGEN_DESIRED_TORQUE_FRACTION` for the desired off-throttle feel.
- [ ] **Verify Speed Conversion:** Verify the motor speed RPM to rad/s conversion factor if needed for accurate torque calculations.
- [ ] **Implement/Verify `getMaxTorqueNm()`:** Implement the `Bamocar::getMaxTorqueNm()` helper function or replace its usage with a constant representing the motor's nominal maximum torque in Nm, needed for scaling the regen torque limit correctly.

## File: `src/brake_light.cpp`

- [ ] **Implement Deceleration Calculation:** Calculate deceleration using the MPU6050 sensor data (e.g., `a.acceleration.x`). Verify sensor orientation and sign convention.
- [ ] **Implement Deceleration Brake Light:** Modify the brake light activation logic to turn the light ON if `(brakePressure > THRESHOLD)` OR `(calculated_deceleration > 1.0 m/s^2)`. Ensure the 1.0 m/s^2 threshold is correctly implemented (Rule T6.3.1).
- [ ] **Filter MPU Data:** Consider filtering MPU6050 acceleration data (e.g., using a moving average or low-pass filter) to get a stable deceleration value.
- [ ] **Re-evaluate Tilt Logic:** Re-evaluate the necessity of using `TILT_THRESHOLD_DEG`; direct deceleration measurement is generally preferred (Rule T6.3.1).
- [ ] **Move MPU Initialization:** Move the `initializeMPU()` call to `setup()` in `main.cpp` for robustness and handle potential initialization failures gracefully.

---
