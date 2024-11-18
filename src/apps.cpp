/*

appst.cpp
Written by Hari Mohan
UCD Formula Student

*/

#include "header.h"

// maximum and minimum voltage values from the potentiometer
const double PEDAL_MIN = 1.4;
const double PEDAL_MAX = 3.2;

// returns a value between 0 and 100
// 0 standing for fully released pedal
// 100 standing for fully actuated pedal
// will return -1 if APPS implausibility detected
double get_apps_reading() {
  double apps_1_voltage;
  double apps_2_voltage;

  int apps_1_reading = analogRead(APPS_1_PIN);
  int apps_2_reading = analogRead(APPS_2_PIN);

  // map values from 0-1023 to 0.0-3.3
  apps_1_voltage = apps_1_reading * 3.3 / 1023.0;
  apps_2_voltage = apps_2_reading * 3.3 / 1023.0;

  // map values to a 0 to 100 scale to check for implausibility if more than
  // 10 point difference
  apps_1_voltage = (100.0 / (PEDAL_MAX - PEDAL_MIN)) * (apps_1_voltage - PEDAL_MIN);
  apps_2_voltage = (100.0 / (PEDAL_MAX - PEDAL_MIN)) * (apps_2_voltage - PEDAL_MIN);

  if (abs(apps_1_reading - apps_2_reading) - 10.0 > std::numeric_limits<double>::epsilon()) {
    Serial.println("Implausibility detected!!!");
    return -1;
  }

  return (apps_1_voltage + apps_2_voltage) / 2;
}