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
    // Decide if failure to set filters is critical
    // return false;
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
  // Disable all mailboxes by default before configuring
  Can0.disable_all_mailboxes();

  /*
   * Filter Configuration Strategy:
   * Use specific mailboxes for high-priority messages (like Bamocar responses)
   * Use other mailboxes with broader masks if needed for BMS or other nodes.
   * Mailbox 0-7 available on CAN0 for SAM3X.
   * We need to receive:
   * - Bamocar responses (ID: BAMOCAR_TX_ID)
   * - BMS messages (IDs: ORION_BMS_ID_1, ORION_BMS_ID_2, potentially others)
   * Add filters for any other nodes as needed.
   */

  // Example: Mailbox 0 for Bamocar responses (exact ID match)
  // Mask 0x7FF means all ID bits must match.
  // TODO: Verify BAMOCAR_TX_ID is correct for your setup
  if (Can0.init_filter(0, BAMOCAR_TX_ID, CAN_STDID) != 1)
    return false; // Filter for Bamocar TX ID
  // Can0.set_filter_mask(0, 0x7FF, CAN_STDID); // Ensure exact match (often
  // default)

  // Example: Mailbox 1 for BMS Message ID 1 (exact ID match)
  // TODO: Replace ORION_BMS_ID_1 with the actual configured ID
  if (Can0.init_filter(1, ORION_BMS_ID_1, CAN_STDID) != 1)
    return false; // Filter for BMS ID 1
  // Can0.set_filter_mask(1, 0x7FF, CAN_STDID); // Ensure exact match

  // Example: Mailbox 2 for BMS Message ID 2 (exact ID match)
  // TODO: Replace ORION_BMS_ID_2 with the actual configured ID
  if (Can0.init_filter(2, ORION_BMS_ID_2, CAN_STDID) != 1)
    return false; // Filter for BMS ID 2
  // Can0.set_filter_mask(2, 0x7FF, CAN_STDID); // Ensure exact match

  // --- IMPORTANT ---
  // TODO: Add filters here for ALL BMS message IDs you expect to receive from
  // your Orion BMS. If there are many BMS messages, you might use a mask to
  // accept a range, but specific filters are generally better if feasible.
  // Example Mask for a range (e.g., 0x420 to 0x42F):
  // int mailbox_index = 3; // Use next available mailbox
  // uint32_t id_base = 0x420;
  // uint32_t mask = 0x7F0; // Match upper bits, ignore lower 4 bits
  // if (Can0.init_filter(mailbox_index, id_base, CAN_STDID) != 1) return false;
  // if (Can0.set_filter_mask(mailbox_index, mask, CAN_STDID) != 1) return
  // false;
  // --- --- --- --- ---

  // Add filters for other devices as needed...

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
        if (DEBUG_MODE) {
          Serial.print("CANManager: Received unexpected filtered ID: 0x");
          Serial.println(incoming_frame.id, HEX);
        }
        break;
      }
    } else {
      if (DEBUG_MODE) {
        Serial.println(
            "CANManager: Can0.available() > 0 but Can0.read() failed.");
      }
    }
  }
}

//------------------------------------------------------------------------------
// Send CAN Message
//------------------------------------------------------------------------------
bool CANManager::send_message(const CAN_FRAME &frame) {
  // Use the due_can library function to send the frame
  return Can0.sendFrame(frame);
}
