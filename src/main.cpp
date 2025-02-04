/*

main.cpp
UCD Formula Student

*/

#include "header.h"

void setup() {
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);
  Serial.begin(9600);

  // Initialize CAN0 with 500 kbps baud rate
  if (!Can0.begin(CAN_BPS_500K)) {
    Serial.println("CAN Initialization failed! Stopping setup...");
    while (1);  // Halt execution if CAN fails to initialize
  }

  Serial.println("CAN Initialized on Arduino Due!");

  // initialise dashboard
  dash_setup();
}

void loop() {
  // // function call to read brake pressure and handle brake light logic
  brake_light();

  // function to read CAN messages and update variables
  read_CAN_data();

  // function to handle APPS and store returned value in torque request var
  double apps_voltage = get_apps_reading();

  // send torque request + handle implausibility
  if (apps_voltage >= 0) {
      send_torque_request(apps_voltage);
  } else {
      if (DEBUG_MODE)
        Serial.println("No torque request sent due to APPS implausibility.");
  }

  // update dashboard
  dash_loop();

  if (DEBUG_MODE) {
    delay(800);  // small delay to stabilize readings and not overwhelm the serial output
    Serial.println("");
  }
  
}
