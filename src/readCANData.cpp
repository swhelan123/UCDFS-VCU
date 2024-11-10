/*

readCANData.cpp
Written by Shane Whelan
UCD Formula Student

*/

#include "header.h"
#include "globals.h"

float cellVoltage = 0.0;
int stateOfCharge = 0;

void readCANData() {
  CAN_FRAME incoming;

  if (Can0.available()) {
    Can0.read(incoming);

    switch (incoming.id) {
      case 0x100:
        cellVoltage = ((incoming.data.byte[0] << 8) | incoming.data.byte[1]) * 0.001;
        if (DEBUG_MODE) {
          Serial.print("Cell Voltage: ");
          Serial.println(cellVoltage);
        }
        break;

      case 0x101:
        stateOfCharge = incoming.data.byte[0];
        if (DEBUG_MODE) {
          Serial.print("State of Charge: ");
          Serial.println(stateOfCharge);
        }
        break;

      default:
        if (DEBUG_MODE) {
          Serial.print("Unknown CAN ID received: 0x");
          Serial.println(incoming.id, HEX);
        }
        break;
    }
  }
}
