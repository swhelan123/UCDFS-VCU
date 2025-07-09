/**
 * @file can_manager.cpp
 * @brief Implements the CANManager class for handling CAN bus communication.
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-04-27
 */

// TODO:
// - Verify and add CAN filters in setup_filters() for ALL required BMS message
// IDs based on your specific Orion BMS configuration.
// - Confirm Can0.read() behavior in the due_can library regarding reading from
// multiple filtered mailboxes.

#include "can_manager.h"
#include "bamocar-due.h" // Include Bamocar header
#include "bms_handler.h" // Include BMS handler header

// Define the global instance
CANManager can_manager;

// Reference to the global Bamocar instance (assuming it's defined elsewhere,
// e.g., motor_controller.cpp) If not global, this needs to be passed in or
// handled differently.
extern Bamocar bamocar; // Make sure Bamocar object is accessible

//------------------------------------------------------------------------------
// Constructor
//------------------------------------------------------------------------------
CANManager::CANManager() {
  // Initialize pointers if used, e.g.:
  // bamocar_handler = nullptr;
  // bms_handler = nullptr;
}

//------------------------------------------------------------------------------
// Initialize CAN Interface and Filters
//------------------------------------------------------------------------------
bool CANManager::initialize(uint32_t baudrate) {
  // Initialize CAN0
  if (!Can0.begin(baudrate)) {
    if (DEBUG_MODE) {
      Serial.println("CAN0 Initialization Failed!");
    }
    return false;
  }

  // Set up hardware filters
  if (!setup_filters()) {
    if (DEBUG_MODE) {
      Serial.println("CAN0 Filter Setup Failed!");
    }
    // Failure to set filters is critical for receiving any messages.
    return false;
  }

  if (DEBUG_MODE) {
    Serial.println("CAN0 Initialized Successfully with Filters.");
  }
  return true;
}

//------------------------------------------------------------------------------
// Configure Hardware Filters
//------------------------------------------------------------------------------
bool CANManager::setup_filters() {
  // Reset all mailboxes by default before configuring
  Can0.reset_all_mailbox();

  /*
   * Filter Configuration Strategy:
   * Use specific mailboxes for high-priority messages (like Bamocar responses)
   * and BMS data. Mailboxes 0-7 are available on CAN0 for SAM3X.
   * We need to receive:
   * - Bamocar responses (ID: BAMOCAR_TX_ID)
   * - BMS messages (IDs: ORION_BMS_ID_1, ORION_BMS_ID_2, etc.)
   *
   * Using _setFilterSpecific for exact ID matching. This is generally safer.
   * The mask 0x7FF ensures we only receive frames with an exact ID match.
   * The last parameter 'false' specifies a standard (11-bit) CAN ID.
   * Make sure the CAN IDs in can_manager.h are correct for your devices.
   */

  // Mailbox 0: Bamocar responses (exact ID match)
  // TODO: Verify BAMOCAR_TX_ID in can_manager.h is correct (0x181 is default).
  // if (DEBUG_MODE >= 2) {
  //   Serial.print("Setting filter for mailbox 0, ID: 0x");
  //   Serial.println(BAMOCAR_TX_ID, HEX);
  // }
  int result0 = Can0._setFilterSpecific(0, BAMOCAR_TX_ID, 0x7FF, false);
  // if (DEBUG_MODE >= 2) {
  //   Serial.print("_setFilterSpecific(0) returned: ");
  //   Serial.println(result0);
  // }
  // Function might return mailbox number on success (0 for mailbox 0)
  if (result0 != 0) {
    // if (DEBUG_MODE >= 2) Serial.println("Failed to set filter for Bamocar TX.");
    return false;
  }

  // Mailbox 1: BMS Message ID 1 (exact ID match)
  // TODO: Verify ORION_BMS_ID_1 in can_manager.h is correct.
  // if (DEBUG_MODE >= 2) {
  //   Serial.print("Setting filter for mailbox 1, ID: 0x");
  //   Serial.println(ORION_BMS_ID_1, HEX);
  // }
  int result1 = Can0._setFilterSpecific(1, ORION_BMS_ID_1, 0x7FF, false);
  // if (DEBUG_MODE >= 2) {
  //   Serial.print("_setFilterSpecific(1) returned: ");
  //   Serial.println(result1);
  // }
  // Function might return mailbox number on success (1 for mailbox 1)
  if (result1 != 1) {
    // if (DEBUG_MODE >= 2) Serial.println("Failed to set filter for BMS ID 1.");
    return false;
  }

  // Mailbox 2: BMS Message ID 2 (exact ID match)
  // TODO: Verify ORION_BMS_ID_2 in can_manager.h is correct.
  // if (DEBUG_MODE >= 2) {
  //   Serial.print("Setting filter for mailbox 2, ID: 0x");
  //   Serial.println(ORION_BMS_ID_2, HEX);
  // }
  int result2 = Can0._setFilterSpecific(2, ORION_BMS_ID_2, 0x7FF, false);
  // if (DEBUG_MODE >= 2) {
  //   Serial.print("_setFilterSpecific(2) returned: ");
  //   Serial.println(result2);
  // }
  // Function might return mailbox number on success (2 for mailbox 2)
  if (result2 != 2) {
    // if (DEBUG_MODE >= 2) Serial.println("Failed to set filter for BMS ID 2.");
    return false;
  }

  // --- IMPORTANT ---
  // If you have more BMS messages to receive, add more filters here.
  // Example for another mailbox:
  // if (Can0._setFilterSpecific(3, ANOTHER_BMS_ID, 0x7FF, false) != 0) return false;
  // --- --- --- ---

  return true; // Indicate success
}

//------------------------------------------------------------------------------
// Process Incoming Messages
//------------------------------------------------------------------------------
void CANManager::process_incoming_messages() {
  CAN_FRAME incoming_frame;

  // Check if a message is available in any configured mailbox
  if (Can0.available() > 0) {
    // Read the message
    // Note: Can0.read() might need adjustment if the library doesn't
    // automatically check all filtered mailboxes. Check library docs.
    // Assuming Can0.read pulls from the next available filtered mailbox.
    if (Can0.read(incoming_frame)) {

      // Dispatch based on CAN ID
      switch (incoming_frame.id) {
      case BAMOCAR_TX_ID:
        // Pass the frame to the Bamocar handler
        // Ensure 'bamocar' object is accessible here
        bamocar.handle_incoming_frame(incoming_frame);
        break;

      // TODO: Add cases for ALL expected BMS IDs here
      case ORION_BMS_ID_1:
      case ORION_BMS_ID_2:
        // Pass the frame to the BMS handler
        bms_handler.handle_incoming_frame(incoming_frame);
        break;

        // Add cases for other device IDs here...

      default:
        // Handle unexpected but filtered messages if necessary
        // Show ALL incoming messages during testing
        Serial.print("RX: ID=0x");
        Serial.print(incoming_frame.id, HEX);
        Serial.print(" Len=");
        Serial.print(incoming_frame.length);
        Serial.print(" Data=[");
        for (int i = 0; i < incoming_frame.length; i++) {
          if (i > 0) Serial.print(",");
          Serial.print("0x");
          if (incoming_frame.data.bytes[i] < 16) Serial.print("0");
          Serial.print(incoming_frame.data.bytes[i], HEX);
        }
        Serial.println("]");
        break;
      }
    } else {
      if (DEBUG_MODE >= 2) {
        Serial.println("CAN: Available but read failed");
      }
    }
  }
}

//------------------------------------------------------------------------------
// Send CAN Message
//------------------------------------------------------------------------------
bool CANManager::send_message(const CAN_FRAME &frame) {
  // Create a non-const copy for the due_can library
  CAN_FRAME mutable_frame = frame;
  
  // Reduce debug verbosity to prevent output spam
  if (DEBUG_MODE >= 4) {  // Only show at maximum debug level
    Serial.print("CANManager: Attempting to send frame ID: 0x");
    Serial.print(frame.id, HEX);
    Serial.print(", Length: ");
    Serial.println(frame.length);
  }
  
  bool result = Can0.sendFrame(mutable_frame);
  
  // Only show send result at maximum debug level to reduce output spam
  if (DEBUG_MODE >= 4) {
    Serial.print("CANManager: sendFrame() returned: ");
    Serial.println(result ? "SUCCESS" : "FAILED");
  }
  
  // If send fails consistently, try to recover CAN bus
  static int consecutive_failures = 0;
  static unsigned long last_recovery_attempt = 0;
  if (!result) {
    consecutive_failures++;
    if (consecutive_failures >= 20 && millis() - last_recovery_attempt > 5000) { // Increased threshold and time delay
      if (DEBUG_MODE) {
        Serial.println("CAN: Bus recovery attempt");
      }
      // Reset CAN controller to recover from bus-off state
      Can0.reset_all_mailbox();
      setup_filters(); // Re-setup filters after reset
      consecutive_failures = 0;
      last_recovery_attempt = millis();
    }
  } else {
    consecutive_failures = 0; // Reset counter on success
  }
  
  return result;
}
