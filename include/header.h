/*

header.h
UCD Formula Student

*/

#ifndef HEADER_H
#define HEADER_H

// ------------ INCLUDED LIBRARIES ------------
#include <limits>
#include <cmath>
#include <SPI.h>
#include <due_can.h>
#include "apps.h"
#include "globals.h"
#include <Nextion.h>

// ------------ CONSTANTS ------------
const int BRAKE_LIGHT_THRESHOLD = 500; // brake pressure must be calibrated for expected brake light behaviour
const int HYSTERESIS = 15; // arbitarily chose hysteresis band of 15, should be calibrated
const int DEBUG_MODE = 1; // 0 - most serial prints for debugging purposes will not display. change to 1 for debugging messages to print

// ------------ PINS ------------
const int BRAKE_PRESSURE_SENSOR_PIN = A0;
const int APPS_1_PIN = A6;
const int APPS_2_PIN = A7;
const int BRAKE_LIGHT_PIN = 7;

// ------------ FUNCTION PROTOTYPES ------------
void brake_light();
void read_CAN_data();
void send_torque_request(double torqueRequest);
double get_apps_reading();
void dash_setup();
void dash_loop();

#endif