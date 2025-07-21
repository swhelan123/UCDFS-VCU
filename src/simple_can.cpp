/**
 * @file simple_can.cpp
 * @brief Simplified CAN interface for Formula Student VCU.
 * @author UCD Formula Student (Original)
 * @author Gemini AI (Refactoring)
 * @date 2025-07-19
 *
 * @details
 * This file implements the SimpleCAN class for managing CAN bus communications.
 * It has been modified to accept all incoming CAN messages without using
 * hardware filters, as per user request.
 */

#include "simple_can.h"

// Global instance of the SimpleCAN class
SimpleCAN can;

// ------------ CONSTRUCTOR ------------
SimpleCAN::SimpleCAN() {
  // Constructor can be used for default initializations if needed.
}

// ------------ INITIALIZATION ------------
/**
 * @brief Initializes the CAN0 bus.
 * @param baudrate The desired CAN bus speed (e.g., 500000).
 * @return True if initialization is successful, false otherwise.
 */
bool SimpleCAN::begin(uint32_t baudrate) {
  // Initialize the CAN0 controller
  if (!Can0.begin(baudrate)) {
    if (debugLevel > 0) {
      Serial.println("CAN0 initialization failed!");
    }
    return false;
  }

  // --- Hardware Filtering Removed ---
  // Per request, no hardware filters are set. The CAN controller will accept
  // all incoming messages into its buffer. The update() loop is responsible
  // for processing and dispatching them.

  if (debugLevel > 1) {
    Serial.println("CAN0 initialized without hardware filtering.");
  }

  return true;
}

// ------------ MAIN UPDATE LOOP ------------
/**
 * @brief Processes all pending incoming CAN messages.
 * This should be called on every iteration of the main loop().
 */
void SimpleCAN::update() {
  CAN_FRAME incoming;

  // Check if a message is available in the CAN buffer
  if (Can0.available() > 0) {
    // Read the message
    Can0.read(incoming);

    // Dispatch the message to the correct handler based on its ID
    switch (incoming.id) {
    case BAMOCAR_TX_ID:
      handleBamocarMessage(incoming);
      break;
    case BMS_ID_PACK_INFO:
    case BMS_ID_LIMITS_TEMP:
      handleBMSMessage(incoming);
      break;
    default:
      // If we receive an ID we don't explicitly handle, we can log it.
      if (debugLevel > 2) {
        Serial.print("Received unhandled CAN ID: 0x");
        Serial.println(incoming.id, HEX);
      }
      break;
    }
  }
}

// ------------ MESSAGE HANDLERS ------------

/**
 * @brief Handles messages received from the Bamocar motor controller.
 * @param frame The incoming CAN_FRAME.
 */
void SimpleCAN::handleBamocarMessage(const CAN_FRAME &frame) {
  uint8_t regID = frame.data.bytes[0];
  int16_t rawValue = (frame.data.bytes[2] << 8) | frame.data.bytes[1];

  // TODO: VERIFY ALL SCALING FACTORS against Bamocar documentation.
  // These are examples and may not be correct.
  switch (regID) {
  case REG_N_ACTUAL:
    motorRPM = rawValue * 1.0f; // Example scaling: 1 LSB = 1 RPM
    break;
  case REG_TORQUE:
    motorTorque = rawValue * 0.1f; // Example scaling: 1 LSB = 0.1 Nm
    break;
  case REG_TEMP_MOTOR:
    motorTemp = rawValue * 0.1f; // Example scaling: 1 LSB = 0.1 °C
    break;
  case REG_STATUS:
    // Handle status bits if necessary
    break;
  // Add other registers as needed
  }
}

/**
 * @brief Handles messages received from the BMS.
 * @param frame The incoming CAN_FRAME.
 */
void SimpleCAN::handleBMSMessage(const CAN_FRAME &frame) {
  // Mark that we have received a BMS message recently
  lastBmsMessageTime = millis();

  // TODO: Implement BMS checksum validation for message integrity.
  // A corrupted message could provide dangerously incorrect data.

  if (frame.id == BMS_ID_PACK_INFO) {
    // Example parsing for pack info - VERIFY WITH BMS DOCUMENTATION
    uint16_t pack_voltage_raw = (frame.data.bytes[1] << 8) | frame.data.bytes[0];
    int16_t  pack_current_raw = (frame.data.bytes[3] << 8) | frame.data.bytes[2];
    uint16_t pack_soc_raw     = (frame.data.bytes[5] << 8) | frame.data.bytes[4];
    
    packVoltage = pack_voltage_raw * 0.1f;  // 1 LSB = 0.1V
    packCurrent = pack_current_raw * 0.1f;  // 1 LSB = 0.1A
    packSOC     = pack_soc_raw * 0.5f;      // 1 LSB = 0.5%
    relayState  = frame.data.bytes[6];
  }
  else if (frame.id == BMS_ID_LIMITS_TEMP) {
    // Example parsing for limits - VERIFY WITH BMS DOCUMENTATION
    uint16_t dcl_raw = (frame.data.bytes[1] << 8) | frame.data.bytes[0];

    packDCL  = dcl_raw * 1.0f; // 1 LSB = 1A
    highTemp = frame.data.bytes[2];
    lowTemp  = frame.data.bytes[3];
  }
}


// ------------ SENDING FUNCTIONS ------------

/**
 * @brief Sends a generic CAN message.
 * @param id The CAN ID for the message.
 * @param length The length of the data payload (1-8 bytes).
 * @param data A pointer to the data byte array.
 * @return True if the message was sent successfully.
 */
bool SimpleCAN::send(uint32_t id, uint8_t length, uint8_t *data) {
  CAN_FRAME outgoing;
  outgoing.id = id;
  outgoing.length = length;
  for (int i = 0; i < length; i++) {
    outgoing.data.bytes[i] = data[i];
  }
  return Can0.sendFrame(outgoing);
}

/**
 * @brief Sends a 16-bit command to a specific Bamocar register.
 * @param regID The register ID to write to.
 * @param value The 16-bit value to send.
 * @return True if the message was sent successfully.
 */
bool SimpleCAN::sendBamocarCmd(uint8_t regID, uint16_t value) {
  CAN_FRAME outgoing;
  outgoing.id = BAMOCAR_RX_ID;
  outgoing.length = 3;
  outgoing.data.bytes[0] = regID;
  outgoing.data.bytes[1] = value & 0xFF;
  outgoing.data.bytes[2] = (value >> 8) & 0xFF;
  return Can0.sendFrame(outgoing);
}

/**
 * @brief Sends a data request to the Bamocar.
 * @param regID The register ID to request data from.
 * @param interval The desired update interval (e.g., INTVL_100MS).
 * @return True if the request was sent successfully.
 */
bool SimpleCAN::requestBamocarData(uint8_t regID, uint8_t interval) {
  CAN_FRAME outgoing;
  outgoing.id = BAMOCAR_RX_ID;
  outgoing.length = 3;
  outgoing.data.bytes[0] = REG_REQUEST; // Bamocar's standard request register
  outgoing.data.bytes[1] = regID;
  outgoing.data.bytes[2] = interval;
  return Can0.sendFrame(outgoing);
}
