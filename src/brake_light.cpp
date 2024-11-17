/*

brake_light.cpp
Written by Shane Whelan
UCD Formula Student

*/

#include "header.h"

void brake_light() {
  int brakePressure = analogRead(BRAKE_PRESSURE_SENSOR_PIN);

  if (DEBUG_MODE) {
    Serial.print("Brake Pressure: ");
    Serial.println(brakePressure);
  }

  if (brakePressure > BRAKE_LIGHT_THRESHOLD) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
    if (DEBUG_MODE) Serial.println("Brake Light On");
  } else {
    digitalWrite(BRAKE_LIGHT_PIN, LOW);
    if (DEBUG_MODE) Serial.println("Brake Light Off");
  }
}