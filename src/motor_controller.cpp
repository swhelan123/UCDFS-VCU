/*

motor_controller.cpp
Written by ShengXin Chen (Jerry), Shane Whelan
UCD Formula Student

*/

#include "header.h"
#include "globals.h"

void send_torque_request(double apps_voltage) {
    // Scale voltage directly to Bamocar's torque range (0–32767)
    uint16_t scaled_torque = (uint16_t)(((apps_voltage - PEDAL_MIN) / (PEDAL_MAX - PEDAL_MIN)) * 32767);

    

    // Construct the CAN frame
    CAN_FRAME outgoing;
    outgoing.id = 0x201;           // Torque command ID
    outgoing.extended = 0;         // Standard CAN frame
    outgoing.length = 8;           // Fixed 8-byte frame

    // Fill the CAN frame
    outgoing.data.byte[0] = scaled_torque & 0xFF;          // Low byte of torque
    outgoing.data.byte[1] = (scaled_torque >> 8) & 0xFF;   // High byte of torque
    outgoing.data.byte[2] = 0x00;                          // Reserved
    outgoing.data.byte[3] = 0x00;                          // Reserved
    outgoing.data.byte[4] = 0x00;                          // Reserved
    outgoing.data.byte[5] = 0x00;                          // Reserved
    outgoing.data.byte[6] = 0x00;                          // Reserved
    outgoing.data.byte[7] = 0x00;                          // Reserved

    // Send the CAN frame
    Can0.sendFrame(outgoing);

    // Debugging output
    if (DEBUG_MODE) {
        Serial.print("APPS Voltage: ");
        Serial.println(apps_voltage);
        Serial.print("Scaled Torque Value: ");
        Serial.println(scaled_torque);
    }
}

