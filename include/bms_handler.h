/**
 * @file bms_handler.h
 * @brief Defines the BMSHandler class for processing Orion BMS 2 CAN messages
 * and storing the relevant battery status.
 * NOTE: Parsing logic is a placeholder and MUST be updated based on
 * actual BMS configuration and CAN specification.
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

// TODO:
// - Review and update the BMSData struct fields to match the exact data
//   you need from your Orion BMS 2 configuration.
// - Ensure placeholder comments reflect the actual implementation status.

#ifndef BMS_HANDLER_H
#define BMS_HANDLER_H

#include <due_can.h> // For CAN_FRAME type
#include <stdint.h>

// Structure to hold BMS data
// TODO: Verify/Add/Remove fields as needed based on your requirements and BMS
// config
typedef struct {
  // Core Status
  float pack_soc;      // State of Charge (%)
  float pack_voltage;  // Total pack voltage (V)
  float pack_current;  // Pack current (A) (Positive=Discharge, Negative=Charge)
  bool relay_state_ok; // Example: Status of BMS relays (often multiple flags)

  // Limits
  float discharge_current_limit; // Max discharge current allowed (A)
  float charge_current_limit;    // Max charge current allowed (A)

  // Cell Status
  float high_cell_voltage; // Highest cell voltage (V)
  float low_cell_voltage;  // Lowest cell voltage (V)
  float avg_cell_voltage;  // Average cell voltage (V)

  // Temperatures
  int8_t high_temperature; // Highest cell temperature (°C)
  int8_t low_temperature;  // Lowest cell temperature (°C)
  int8_t avg_temperature;  // Average cell temperature (°C)

  // Faults / Status Flags (Use appropriate types, e.g., bitfields or enums)
  // TODO: Replace these examples with actual fault flags/codes from BMS CAN
  // spec
  bool voltage_fault;          // Example fault flag
  bool temperature_fault;      // Example fault flag
  bool communication_fault;    // Flag set by this handler if messages stop
  bool charge_interlock_fault; // Example fault flag
  uint16_t general_fault_code; // Example general fault code field (BMS often
                               // sends specific codes)

  // Timestamp of last received message (useful for checking communication
  // health)
  unsigned long last_message_millis;

} BMSData;

class BMSHandler {
public:
  /**
   * @brief Constructor for BMSHandler. Initializes BMS data.
   */
  BMSHandler();

  /**
   * @brief Processes an incoming CAN frame potentially containing BMS data.
   * Decodes the data based on the CAN ID and updates the internal state.
   * *** THIS IS A PLACEHOLDER - IMPLEMENT ACTUAL DECODING ***
   * @param frame The received CAN_FRAME.
   */
  void handle_incoming_frame(const CAN_FRAME &frame);

  /**
   * @brief Gets the current BMS data.
   * @return A constant reference to the internal BMSData struct.
   */
  const BMSData &get_bms_data() const;

  /**
   * @brief Checks if the BMS is reporting any critical faults.
   * *** THIS IS A PLACEHOLDER - IMPLEMENT ACTUAL FAULT CHECKING ***
   * @return True if a critical fault is active, false otherwise.
   */
  bool has_critical_fault() const;

  /**
   * @brief Checks if BMS communication is active (recent messages received).
   * @param timeout_ms The maximum allowed time in milliseconds since the last
   * message.
   * @return True if communication is active, false otherwise.
   */
  bool is_communication_active(unsigned long timeout_ms = 1000) const;

private:
  BMSData current_bms_data; // Internal storage for BMS state

  /**
   * @brief Placeholder function to parse BMS message ID 1 (e.g., 0x420).
   * *** IMPLEMENT ACTUAL DECODING BASED ON BMS SPEC ***
   * @param frame The received CAN_FRAME with matching ID.
   */
  void parse_bms_message_1(const CAN_FRAME &frame);

  /**
   * @brief Placeholder function to parse BMS message ID 2 (e.g., 0x421).
   * *** IMPLEMENT ACTUAL DECODING BASED ON BMS SPEC ***
   * @param frame The received CAN_FRAME with matching ID.
   */
  void parse_bms_message_2(const CAN_FRAME &frame);

  // TODO: Add more private parsing functions for other BMS message IDs...
};

// Declare a global instance (or manage instantiation differently if preferred)
extern BMSHandler bms_handler;

#endif // BMS_HANDLER_H
