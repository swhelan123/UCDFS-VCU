/*
 * monitorErrors.cpp
 * written by Shane Whelan
 * UCDFS
*/

#include "header.h"

// Declare individual global error flags (extern if declared in a header file)
int error22 = 0;
int error23 = 0;
int error24 = 0;
int error25 = 0;
int error26 = 0;
int error27 = 0;
int error28 = 0;
int error29 = 0;
int error30 = 0;
int error31 = 0;
int error32 = 0;
int error33 = 0;
int error34 = 0;
int error35 = 0;
int error36 = 0;
int error37 = 0;

void monitor_errors_setup() {
  for (int pin = 22; pin <= 37; pin++) {
    pinMode(pin, INPUT);
  }
}


// Function to check pins 22 to 37 and set the corresponding error flag
void monitor_pins_loop() {
  for (int pin = 22; pin <= 37; pin++) {
    int state = digitalRead(pin);
    switch(pin) {
      case 22: error22 = (state == HIGH) ? 1 : 0; break;
      case 23: error23 = (state == HIGH) ? 1 : 0; break;
      case 24: error24 = (state == HIGH) ? 1 : 0; break;
      case 25: error25 = (state == HIGH) ? 1 : 0; break;
      case 26: error26 = (state == HIGH) ? 1 : 0; break;
      case 27: error27 = (state == HIGH) ? 1 : 0; break;
      case 28: error28 = (state == HIGH) ? 1 : 0; break;
      case 29: error29 = (state == HIGH) ? 1 : 0; break;
      case 30: error30 = (state == HIGH) ? 1 : 0; break;
      case 31: error31 = (state == HIGH) ? 1 : 0; break;
      case 32: error32 = (state == HIGH) ? 1 : 0; break;
      case 33: error33 = (state == HIGH) ? 1 : 0; break;
      case 34: error34 = (state == HIGH) ? 1 : 0; break;
      case 35: error35 = (state == HIGH) ? 1 : 0; break;
      case 36: error36 = (state == HIGH) ? 1 : 0; break;
      case 37: error37 = (state == HIGH) ? 1 : 0; break;
    }
  }
}
