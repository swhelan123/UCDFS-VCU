/**
 * @file bms_handler.cpp
 * @brief Implements the BMSHandler class for processing Orion BMS 2 CAN
 * messages. NOTE: Parsing logic is a placeholder and MUST be updated based on
 * actual BMS configuration and CAN specification.
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

// TODO:
// - Replace ALL placeholder parsing logic in parse_bms_message_X functions
//   with the correct decoding (byte order, data types, scaling, offsets)
//   based on your specific Orion BMS 2 configuration and CAN documentation.
// - Implement the has_critical_fault() function based on the actual fault
//   flags and critical limits defined by the BMS and FSUK rules.
// - Add parsing functions for all required BMS message IDs.
// - Initialize BMSData struct in the constructor with appropriate defaults.

#include "bms_handler.h"
#include "can_manager.h" // Include CAN manager for IDs if needed
#include "header.h"      // For DEBUG_MODE, Serial
#include <Arduino.h>     // For millis()

// Define the global instance
BMSHandler bms_handler;

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
BMSHandler::BMSHandler() {
  // Initialize BMS data structure with default/safe values
  // TODO: Review these default values
  current_bms_data.pack_soc = 0.0f;
  current_bms_data.pack_voltage = 0.0f;
  current_bms_data.pack_current = 0.0f;
  current_bms_data.relay_state_ok = false; // Assume relays are not OK initially
  current_bms_data.discharge_current_limit =
      0.0f; // Default to no discharge allowed
  current_bms_data.charge_current_limit = 0.0f; // Default to no charge allowed
  current_bms_data.high_cell_voltage = 0.0f;
  current_bms_data.low_cell_voltage =
      5.0f; // Init low voltage high to avoid false positives
  current_bms_data.avg_cell_voltage = 0.0f;
  current_bms_data.high_temperature = -127;
  current_bms_data.low_temperature = 127; // Init low temp high
  current_bms_data.avg_temperature = 0;
  current_bms_data.voltage_fault =
      true; // Default to fault state until proven otherwise
  current_bms_data.temperature_fault = true;
  current_bms_data.communication_fault = true;
  current_bms_data.charge_interlock_fault = true;
  current_bms_data.general_fault_code = 0xFFFF; // Example fault code
  current_bms_data.last_message_millis = 0;
}

//------------------------------------------------------------------------------
// Handle Incoming Frame
//------------------------------------------------------------------------------
void BMSHandler::handle_incoming_frame(const CAN_FRAME &frame) {
  // Update timestamp for communication health check
  current_bms_data.last_message_millis = millis();
  current_bms_data.communication_fault = false; // We received something

  // Call the appropriate parsing function based on ID
  switch (frame.id) {
  // TODO: Replace example IDs with actual configured IDs
  case ORION_BMS_ID_1: // Example ID 0x420
    parse_bms_message_1(frame);
    break;

  case ORION_BMS_ID_2: // Example ID 0x421
    parse_bms_message_2(frame);
    break;

    // TODO: Add cases for other BMS message IDs here...

  default:
    // Should not happen if filters are set correctly, but handle defensively
    if (DEBUG_MODE) {
      Serial.print("BMSHandler: Received unexpected ID: 0x");
      Serial.println(frame.id, HEX);
    }
    break;
  }

  // After parsing, update overall fault status (example)
  // TODO: Implement your actual fault logic based on parsed data & BMS fault
  // codes Example thresholds - ADJUST THESE based on cell datasheet & safety
  // margins
  const float MIN_CELL_VOLTAGE = 2.5f;
  const float MAX_CELL_VOLTAGE = 4.2f;
  const int8_t MAX_CELL_TEMP =
      60; // FSUK Rule EV5.8.5 limit or datasheet, whichever is lower

  current_bms_data.voltage_fault =
      (current_bms_data.low_cell_voltage < MIN_CELL_VOLTAGE ||
       current_bms_data.high_cell_voltage > MAX_CELL_VOLTAGE);
  current_bms_data.temperature_fault =
      (current_bms_data.high_temperature > MAX_CELL_TEMP);
  // Update other fault flags based on specific BMS fault codes received...
}

//------------------------------------------------------------------------------
// Get BMS Data
//------------------------------------------------------------------------------
const BMSData &BMSHandler::get_bms_data() const { return current_bms_data; }

//------------------------------------------------------------------------------
// Check for Critical Faults
//------------------------------------------------------------------------------
bool BMSHandler::has_critical_fault() const {
  // TODO: Implement ACTUAL critical fault checking logic.
  // This should check flags set during parsing based on BMS fault codes
  // and critical operating limits (voltage, temp, current limits, relay state).
  // Rule EV5.8.7: AMS must switch off TS for critical voltage, temp, current.
  // Rule EV6.3.7: IMD must switch off TS for insulation failure (IMD handles
  // this directly in SDC). Rule EV5.8.10: Loss of measurement connection must
  // open SDC (handled by comms timeout check).

  // Example placeholder logic:
  if (current_bms_data.communication_fault)
    return true; // Treat comms loss as critical
  if (current_bms_data.voltage_fault)
    return true;
  if (current_bms_data.temperature_fault)
    return true;
  if (current_bms_data.charge_interlock_fault)
    return true; // Example
  if (current_bms_data.general_fault_code != 0)
    return true; // Check specific critical fault codes from BMS
  if (!current_bms_data.relay_state_ok)
    return true; // If relays are commanded open by BMS

  // Check against discharge current limit (DCL)
  // Note: Pack current is often negative for charge, positive for discharge
  if (current_bms_data.pack_current >
      current_bms_data.discharge_current_limit) {
    // Add a small tolerance if needed
    // return true; // Uncomment if exceeding DCL is considered critical here
  }

  return false; // No critical fault detected by this logic
}

//------------------------------------------------------------------------------
// Check Communication Activity
//------------------------------------------------------------------------------
bool BMSHandler::is_communication_active(unsigned long timeout_ms) const {
  if (current_bms_data.last_message_millis == 0 && millis() > timeout_ms) {
    // If we haven't received *any* messages after the timeout period, assume
    // comms failure
    return false;
  }
  // If we have received messages, check if the last one was within the timeout
  // period
  return (millis() - current_bms_data.last_message_millis) < timeout_ms;
}

//------------------------------------------------------------------------------
// Placeholder Parsing Functions - *** REPLACE WITH ACTUAL IMPLEMENTATION ***
//------------------------------------------------------------------------------

// TODO: Implement actual parsing based on your Orion BMS CAN Spec
void BMSHandler::parse_bms_message_1(const CAN_FRAME &frame) {
  // --- Placeholder for ID 0x420 (Example based on search result snippet) ---
  // WARNING: Actual Orion BMS 2 CAN spec needed for correct byte order,
  //          scaling, offsets, and data types (int vs uint, float
  //          representation).
  // This example assumes Little Endian and simple scaling/no offsets.

  if (frame.length == 8) { // Check data length code
    // Example: Pack SOC (byte 0, uint8, scale 0.5, offset 0 -> range 0-127.5%)
    // TODO: Verify scaling and offset from BMS Tool
    current_bms_data.pack_soc = (float)frame.data.bytes[0] * 0.5f;

    // Example: Pack DCL (Discharge Current Limit) (bytes 1-2, uint16, scale 1,
    // offset 0) Assuming Little Endian: LSB = byte 1, MSB = byte 2
    // TODO: Verify byte order, scaling, offset
    uint16_t raw_dcl = frame.data.bytes[1] | (frame.data.bytes[2] << 8);
    current_bms_data.discharge_current_limit =
        (float)raw_dcl; // Apply scaling/offset if needed

    // Example: High Cell Voltage (bytes 4-5, uint16, scale 0.0001, offset 0)
    // TODO: Verify byte order, scaling, offset
    uint16_t raw_high_v = frame.data.bytes[4] | (frame.data.bytes[5] << 8);
    current_bms_data.high_cell_voltage = (float)raw_high_v * 0.0001f;

    // Example: Low Cell Voltage (bytes 6-7, uint16, scale 0.0001, offset 0)
    // TODO: Verify byte order, scaling, offset
    uint16_t raw_low_v = frame.data.bytes[6] | (frame.data.bytes[7] << 8);
    current_bms_data.low_cell_voltage = (float)raw_low_v * 0.0001f;

    // TODO: Parse other fields from this ID (State of Health, High/Avg Temp
    // etc.) Example: High Temp (byte 2, int8, scale 1, offset 0)
    // TODO: Verify byte order, scaling, offset
    current_bms_data.high_temperature =
        (int8_t)frame.data.bytes[2]; // Cast needed for signed

    // TODO: Parse BMS Relay Status (often part of a status/flags byte/word)
    // Example: current_bms_data.relay_state_ok =
    // check_relay_status_bits(frame.data.bytes[X]);

    // TODO: Parse BMS Fault Codes (often part of a status/flags byte/word)
    // Example: current_bms_data.general_fault_code =
    // get_fault_code(frame.data.bytes[Y], frame.data.bytes[Z]);

  } else {
    if (DEBUG_MODE) {
      Serial.print("BMSHandler: Incorrect DLC for ID 0x");
      Serial.print(frame.id, HEX);
      Serial.print(", expected 8, got ");
      Serial.println(frame.length);
    }
  }
}

// TODO: Implement actual parsing based on your Orion BMS CAN Spec
void BMSHandler::parse_bms_message_2(const CAN_FRAME &frame) {
  // --- Placeholder for ID 0x421 (Example based on search result snippet) ---
  // WARNING: Actual Orion BMS 2 CAN spec needed.

  if (frame.length == 8) { // Check data length code
    // Example: Pack CCL (Charge Current Limit) (bytes 0-1, uint16, scale 1,
    // offset 0)
    // TODO: Verify byte order, scaling, offset
    uint16_t raw_ccl = frame.data.bytes[0] | (frame.data.bytes[1] << 8);
    current_bms_data.charge_current_limit = (float)raw_ccl;

    // Example: Pack Voltage (bytes 2-3, uint16, scale 0.1, offset 0)
    // TODO: Verify byte order, scaling, offset
    uint16_t raw_pack_v = frame.data.bytes[2] | (frame.data.bytes[3] << 8);
    current_bms_data.pack_voltage = (float)raw_pack_v * 0.1f;

    // Example: Pack Current (bytes 4-5, int16, scale 0.1, offset 0)
    // Note: int16 for signed current
    // TODO: Verify byte order, scaling, offset, sign convention
    int16_t raw_pack_c = frame.data.bytes[4] | (frame.data.bytes[5] << 8);
    current_bms_data.pack_current = (float)raw_pack_c * 0.1f;

    // Example: Average Cell Voltage (bytes 6-7, uint16, scale 0.0001, offset 0)
    // TODO: Verify byte order, scaling, offset
    uint16_t raw_avg_v = frame.data.bytes[6] | (frame.data.bytes[7] << 8);
    current_bms_data.avg_cell_voltage = (float)raw_avg_v * 0.0001f;

    // TODO: Parse other fields from this ID...

  } else {
    if (DEBUG_MODE) {
      Serial.print("BMSHandler: Incorrect DLC for ID 0x");
      Serial.print(frame.id, HEX);
      Serial.print(", expected 8, got ");
      Serial.println(frame.length);
    }
  }
}

// TODO: Add implementations for other parsing functions...
