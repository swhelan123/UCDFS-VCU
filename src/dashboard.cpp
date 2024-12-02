// dashboard.cpp
// Written by Shane Whelan
// UCD Formula Student

#include "header.h"

int currentPage = 0;  // Current page number
const int totalPages = 3;  // Total number of pages

unsigned long previousMillis = 0;
unsigned long elapsedMillis = 0;

// Define button pin
#define BUTTON_PIN 2

// Define Nextion text objects for variables
NexText brake_pres = NexText(1, 0, "brake_pres");  
NexText t1 = NexText(1, 0, "t1"); 
NexText millis_c = NexText(1, 0, "millis_c");  
NexText p1_text = NexText(1, 0, "t2");  
NexText speed_text = NexText(2, 0, "t0");  

// Nextion touch event list (not used yet, could be removed if touch function unused in final code)
NexTouch *nex_listen_list[] = { NULL };

void dash_setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Configure button pin with internal pull-up
  Serial1.begin(9600);               // Serial for Nextion communication
  
  // Initialize the Nextion display
  nexInit();
  
  Serial.println("Nextion display initialized!");
}

void dash_loop() {
  static bool lastButtonState = HIGH;  // Track the last button state
  bool buttonState = digitalRead(BUTTON_PIN);

  // Check for a button press (detect a transition from HIGH to LOW)
  if (lastButtonState == HIGH && buttonState == LOW) {
    // Button was pressed
    currentPage = (currentPage + 1) % totalPages;  // Cycle to the next page
    changePage(currentPage);  // Change the Nextion page
  }

  lastButtonState = buttonState;  // Update button state

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

  // Update brake pressure on the Nextion display
  char buffer[40];
  sprintf(buffer, "%d", brakePressure);
  brake_pres.setText(buffer);

  // Update milliseconds mod value on the Nextion display
  sprintf(buffer, "%d", mod_millis);
  t1.setText(buffer);

  // Update the text component on the Nextion display
  sprintf(buffer, "hello world");
  millis_c.setText(buffer);

  // Update the time in the p1_text field
  sprintf(buffer, "%02d:%02d:%02d.%02d", hours, minutes, seconds, fracSecs);
  p1_text.setText(buffer);

  int speed = (rand() % (101)) + 100;
  sprintf(buffer, "%d", speed);
  speed_text.setText(buffer);
}

void changePage(int page) {
  char command[20];
  sprintf(command, "page %d", page);
  Serial1.print(command);  // Send page change command
  Serial1.print("\xFF\xFF\xFF");  // Nextion command terminator
}