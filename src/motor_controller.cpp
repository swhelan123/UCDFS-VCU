/*

motor_controller.cpp
Written by ShengXin Chen (Jerry), Shane Whelan
UCD Formula Student

*/

#include "header.h"
#include "globals.h"

void send_torque_request(double torque_request) {
  // convert torque request to integer number that will fill 1 byte
  int torque_request_byte = floor((torque_request / 100) * 255);
  
  CAN_FRAME outgoing;

  // Set CAN ID for Bamocar motor controller
  outgoing.id = 0x200;  // Update ID for Bamocar if needed
  outgoing.extended = 0; // Standard frame

  // Construct the CAN frame data
  outgoing.data.byte[0] = (torque_request_byte >> 8) & 0xFF;  // High byte
  outgoing.data.byte[1] = torque_request_byte & 0xFF;          // Low byte
  outgoing.data.byte[2] = 0;                             // Reserved or additional data, if needed
  outgoing.data.byte[3] = 0;
  outgoing.data.byte[4] = 0;
  outgoing.data.byte[5] = 0;
  outgoing.data.byte[6] = 0;
  outgoing.data.byte[7] = 0;

  outgoing.length = 8; // Length of the CAN data frame

  // Send the CAN frame
  Can0.sendFrame(outgoing);

  // Optional: Debug output
  if (DEBUG_MODE) {
    Serial.print("Torque Request Sent: ");
    Serial.println(torque_request);
  }
}
