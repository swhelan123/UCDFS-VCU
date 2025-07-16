/**
 * @file main_test.cpp
 * @brief Minimal test file for standalone motor controller testing
 * @author Shane Whelan (UCD Formula Student)
 * @date 2025-06-24
 */

#include "bamocar-due.h"
#include "can_manager.h"
#include "header.h"
#include <Arduino.h>

// Global Bamocar instance
extern Bamocar bamocar;

// Simple CAN tracking function
bool sendTorqueWithTracking(float torque, int &success_count, int &fail_count) {
  bool result = bamocar.setTorque(torque);
  if (result) success_count++;
  else fail_count++;
  return result;
}

bool requestStatusWithTracking(int &success_count, int &fail_count) {
  bool result = bamocar.requestStatus(INTVL_IMMEDIATE);
  if (result) success_count++;
  else fail_count++;
  return result;
}

//------------------------------------------------------------------------------
// SETUP FUNCTION - Minimal for MC Testing
//------------------------------------------------------------------------------
void setup() {
  // Initialize Serial Communication
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait max 3s for serial
  
  Serial.println("=== Motor Controller Standalone Test ===");

  // Initialize CAN Communication
  if (!can_manager.initialize(CAN_BPS_500K)) {
    Serial.println("ERROR: CAN initialization failed!");
    while (1); // Halt on CAN failure
  }
  
  Serial.println("CAN initialized successfully");
  Serial.println("Starting motor controller test...");
  Serial.println("Commands: 's'=stop, '1'-'9'=torque %, 'r'=request status");
}

//------------------------------------------------------------------------------
// MAIN LOOP - Simple MC Test
//------------------------------------------------------------------------------
void loop() {
  static unsigned long last_status_request = 0;
  static unsigned long last_heartbeat = 0;
  static unsigned long last_can_status = 0;
  static int can_success_count = 0;
  static int can_fail_count = 0;
  
  // Process incoming CAN messages and show any we receive
  static unsigned long last_incoming_check = 0;
  can_manager.process_incoming_messages();
  
  // Check for ANY incoming CAN traffic periodically
  if (millis() - last_incoming_check > 5000) {
    // This will trigger can_manager to print any unexpected messages
    last_incoming_check = millis();
  }
  
  // Track CAN performance (simple way)
  if (millis() - last_can_status > 2000) { // Every 2 seconds
    Serial.print("[CAN Stats] Success: ");
    Serial.print(can_success_count);
    Serial.print(" | Failed: ");
    Serial.println(can_fail_count);
    can_success_count = 0;
    can_fail_count = 0;
    last_can_status = millis();
  }
  
  // Handle serial commands for testing
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 's': // Stop motor
        Serial.println("CMD: Stop motor (0% torque)");
        sendTorqueWithTracking(0.0f, can_success_count, can_fail_count);
        break;
        
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9': {
        float torque = (cmd - '0') * 0.1f; // Convert to 10%, 20%, etc.
        Serial.print("CMD: Set torque to ");
        Serial.print(torque * 100, 0);
        Serial.println("%");
        sendTorqueWithTracking(torque, can_success_count, can_fail_count);
        break;
      }
      
      case 'r': // Request status
        Serial.println("CMD: Request Bamocar status");
        requestStatusWithTracking(can_success_count, can_fail_count);
        break;
        
      case '0': // Raw 0% torque
        Serial.println("CMD: Raw 0% torque (ID 0x201, reg 0x90)");
        {
          CAN_FRAME torque_frame;
          torque_frame.id = 0x201;
          torque_frame.length = 3;
          torque_frame.data.bytes[0] = 0x90; // Torque register
          torque_frame.data.bytes[1] = 0x00; // Torque low byte (0%)
          torque_frame.data.bytes[2] = 0x00; // Torque high byte
          bool result = can_manager.send_message(torque_frame);
          Serial.print("Raw torque send: ");
          Serial.println(result ? "SUCCESS" : "FAILED");
          if (result) can_success_count++;
          else can_fail_count++;
        }
        break;
        
      case 'q': case 'w': case 'e': case 'y': case 'u': case 'o': case 'p': case 'z': case 'c': case 'v': {
        // Raw torque commands: q=10%, w=20%, e=30%, y=40%, u=50%, o=60%, p=70%, z=80%, c=90%, v=100%
        int torque_percent;
        switch(cmd) {
          case 'q': torque_percent = 10; break;
          case 'w': torque_percent = 20; break;
          case 'e': torque_percent = 30; break;
          case 'y': torque_percent = 40; break;
          case 'u': torque_percent = 50; break;
          case 'o': torque_percent = 60; break;
          case 'p': torque_percent = 70; break;
          case 'z': torque_percent = 80; break;
          case 'c': torque_percent = 90; break;
          case 'v': torque_percent = 100; break;
          default: torque_percent = 0; break;
        }
        
        Serial.print("CMD: Raw ");
        Serial.print(torque_percent);
        Serial.println("% torque (ID 0x201, reg 0x90)");
        
        {
          CAN_FRAME torque_frame;
          torque_frame.id = 0x201;
          torque_frame.length = 3;
          torque_frame.data.bytes[0] = 0x90; // Torque register
          
          // Convert percentage to 16-bit value (0-32767 range for positive torque)
          int16_t torque_value = (int16_t)((torque_percent * 327.67f)); // Scale to ~32767 max
          torque_frame.data.bytes[1] = torque_value & 0xFF;        // Low byte
          torque_frame.data.bytes[2] = (torque_value >> 8) & 0xFF; // High byte
          
          bool result = can_manager.send_message(torque_frame);
          Serial.print("Raw torque send (value=");
          Serial.print(torque_value);
          Serial.print("): ");
          Serial.println(result ? "SUCCESS" : "FAILED");
          if (result) can_success_count++;
          else can_fail_count++;
        }
        break;
      }
        
      case 'h': // Help
        Serial.println("Commands:");
        Serial.println("  s = Stop (0% torque)");
        Serial.println("  1-9 = Set torque (10%-90%)");
        Serial.println("  0 = Raw 0% torque (ID 0x201, reg 0x90)");
        Serial.println("  q/w/e/y/u/o/p/z/c/v = Raw torque 10%-100% (direct CAN)");
        Serial.println("  r = Request status (manual)");
        Serial.println("  t = Raw CAN test (bypass Bamocar)");
        Serial.println("  i = Show CAN info");
        Serial.println("  l = Listen only (read incoming messages)");
        Serial.println("  x = Reset CAN controller");
        Serial.println("  a = Test alternative CAN IDs");
        Serial.println("  h = This help");
        break;
        
      case 'i': // CAN info
        Serial.println("=== CAN Configuration ===");
        Serial.println("Sending to Bamocar: 0x201");
        Serial.println("Expecting from Bamocar: 0x181");
        Serial.println("CAN Speed: 500kbps");
        Serial.println("Termination: 60 ohms (Good!)");
        Serial.println("Message Size: 3 bytes");
        Serial.println("Format: [RegID, DataLo, DataHi]");
        Serial.print("Current Bamocar Status: 0x");
        Serial.println(bamocar.getStatus(), HEX);
        break;
        
      case 't': // Raw CAN test
        Serial.println("CMD: Raw CAN test");
        {
          CAN_FRAME test_frame;
          test_frame.id = 0x123;  // Test ID
          test_frame.length = 8;
          for (int i = 0; i < 8; i++) test_frame.data.bytes[i] = i;
          bool result = can_manager.send_message(test_frame);
          Serial.print("Raw CAN send: ");
          Serial.println(result ? "SUCCESS" : "FAILED");
          if (result) can_success_count++;
          else can_fail_count++;
        }
        break;
        
      case 'l': // Listen only mode
        Serial.println("CMD: Listen mode (no sending, just read)");
        {
          static unsigned long last_listen_report = 0;
          if (millis() - last_listen_report > 3000) {
            Serial.println("Listening for incoming CAN messages...");
            last_listen_report = millis();
          }
        }
        break;
        
      case 'x': // Reset CAN controller
        Serial.println("CMD: Reset CAN controller");
        if (can_manager.initialize(CAN_BPS_500K)) {
          Serial.println("CAN reset successful");
        } else {
          Serial.println("CAN reset failed");
        }
        break;
        
      case 'a': // Alternative CAN IDs test
        Serial.println("CMD: Test alternative CAN IDs");
        {
          // Try common Bamocar ID variations
          uint16_t test_ids[] = {0x200, 0x202, 0x600, 0x601};
          for (int i = 0; i < 4; i++) {
            CAN_FRAME test_frame;
            test_frame.id = test_ids[i];
            test_frame.length = 3;
            test_frame.data.bytes[0] = 0x3D; // Status request
            test_frame.data.bytes[1] = 0x40; // N_ACTUAL register
            test_frame.data.bytes[2] = 0x01; // Immediate
            bool result = can_manager.send_message(test_frame);
            Serial.print("Test ID 0x");
            Serial.print(test_ids[i], HEX);
            Serial.print(": ");
            Serial.println(result ? "SUCCESS" : "FAILED");
            delay(100); // Small delay between tests
          }
        }
        break;
        
      default:
        Serial.println("Unknown command. Send 'h' for help.");
        break;
    }
  }
  
  // Auto-request status every 1 second (DISABLED FOR TESTING)
  // if (millis() - last_status_request > 1000) {
  //   requestStatusWithTracking(can_success_count, can_fail_count);
  //   last_status_request = millis();
  // }
  
  // Heartbeat every 5 seconds
  if (millis() - last_heartbeat > 5000) {
    Serial.print("Heartbeat - Bamocar Status: 0x");
    Serial.println(bamocar.getStatus(), HEX);
    last_heartbeat = millis();
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
