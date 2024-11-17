/*

main.cpp
UCD Formula Student

*/

#include "header.h"

void setup() {
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);
  Serial.begin(9600);

  // Initialize CAN0 with 500 kbps baud rate
  Can0.begin(CAN_BPS_500K);

  Serial.println("CAN Initialized on Arduino Due!");

  // Setup CAN filters (optional, can be removed if we don't have a busy canbus with unwanted IDs/messages)
  Can0.watchFor(0x100); // Watch for ID 0x100 (cell voltage)
  Can0.watchFor(0x101); // Watch for ID 0x101 (state of charge)
}

void loop() {
  // function call to read brake pressure and handle brake light logic
  brake_light();

  // function to read CAN messages and update variables
  read_CAN_data();

  // function to handle APPS and store returned value in torque request var
  double torque_request = get_apps_reading();

  // send torque request 
  send_torque_request(torque_request);

  if (DEBUG_MODE) {
    delay(200);  // small delay to stabilize readings and not overwhelm the serial output
  }
}
