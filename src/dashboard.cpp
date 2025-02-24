// dashboard.cpp
// Written by Shane Whelan
// UCD Formula Student

#include "header.h"

// Define Nextion text objects for variables
NexText e1 = NexText(1, 4, "t1");  
NexText e2 = NexText(1, 5, "t2"); 
NexText e3 = NexText(1, 6, "t3");  
NexText e4 = NexText(1, 7, "t4");  
NexText e5 = NexText(1, 8, "t5");  

// Nextion touch event list (not used yet, could be removed if touch function unused in final code)
NexTouch *nex_listen_list[] = { NULL };

// Variables for elapsed time
unsigned long elapsedMillis = 0;
unsigned long previousMillis = 0;

// Buffer for text conversion
char buffer[40];

void dash_setup() {
  Serial1.begin(9600);  // Serial for Nextion communication
  
  // Initialize the Nextion display
  nexInit();
  
  Serial.println("Nextion display initialized!");
}

void dash_loop() {
  // Calculate elapsed time
  unsigned long currentMillis = millis();
  elapsedMillis += currentMillis - previousMillis;
  previousMillis = currentMillis;

  // Convert elapsed time into hours, minutes, and seconds
  unsigned long totalSeconds = elapsedMillis / 1000;
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;
  int fracSecs = (millis()%1000) / 10;
  int mod_millis = millis() % 5000;

  // Update e1 on the Nextion display
  sprintf(buffer, "%d", brakePressure);
  e1.setText(buffer);

  // Update milliseconds mod value on the Nextion display
  sprintf(buffer, "%d", mod_millis);
  e2.setText(buffer);

  // Update the text component on the Nextion display
  sprintf(buffer, "millie");
  e3.setText(buffer);

  // Update the time in the e4 field
  sprintf(buffer, "%02d:%02d:%02d.%02d", hours, minutes, seconds, fracSecs);
  e4.setText(buffer);

  int speed = (rand() % (101)) + 100;
  sprintf(buffer, "%d", brakePressure);
  e5.setText(buffer);
}