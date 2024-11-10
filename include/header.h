/*

header.h
UCD Formula Student

*/

#ifndef BRAKE_LIGHT_H
#define BRAKE_LIGHT_H

// ------------ INCLUDED LIBRARIES ------------
#include <SPI.h>
#include <due_can.h>

// ------------ CONSTANTS ------------
const int BRAKE_LIGHT_PIN = 9;
const int BRAKE_PRESSURE_SENSOR_PIN = A0;
const int THRESHOLD = 500;
const int DEBUG_MODE = 1;

// ------------ FUNCTION PROTOTYPES ------------
void brake_light();
void readCANData();

#endif