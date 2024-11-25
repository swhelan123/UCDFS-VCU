/*

dashboard.cpp
Written by Shane Whelan
UCD Formula Student

*/

#include "header.h"

// Define the Nextion text component for brake pressure display
NexText brakePressureText = NexText(0, 1, "brakeTxt");

// Initialize previous update time for timing updates to the Nextion display
unsigned long previousDisplayUpdateTime = 0;
const unsigned long displayUpdateInterval = 100; // Update every 100 ms

void setup_dashboard() {
  // Initialize the Nextion communication
  Serial1.begin(9600);
  nexInit();
  Serial.println("Nextion display initialized!");
}

void update_dashboard() {
  // Get the current time
  unsigned long currentTime = millis();

  // Update the Nextion display if enough time has passed
  if (currentTime - previousDisplayUpdateTime >= displayUpdateInterval) {
    previousDisplayUpdateTime = currentTime;

    // Update the brake pressure value on the Nextion display
    char buffer[10];
    sprintf(buffer, "%d", brakePressure);  // Display brake pressure as an integer
    brakePressureText.setText(buffer);
  }
}

