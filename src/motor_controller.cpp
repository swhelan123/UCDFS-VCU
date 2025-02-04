/* 
 * motor_controller.cpp
 * Written by ShengXin Chen (Jerry), Shane Whelan
 * UCD Formula Student
 */

#include "header.h"
#include "globals.h"

// Include the Bamocar library headers
#include "bamocar-due.h"

// Create a Bamocar object on mailbox 0
static Bamocar bamocar(0);

// OPTIONAL: If you need to override the default CAN IDs (0x201 rx, 0x181 tx),
// uncomment and adjust as needed:
// static void initMotorControllerIDs() {
//     bamocar.setRxID(0x201);
//     bamocar.setTxID(0x181);
// }

void send_torque_request(double torque_request)
{
    // Set 0% torque request for value outside valid range
    if (torque_request < 0 || torque_request > 100) torque_request = 0;

    // Bamocar expects a fraction (0.0 to 1.0), so convert
    float torqueFraction = torque_request / 100.0;

    // Use the library call to send torque
    bamocar.setTorque(torqueFraction);

    if (DEBUG_MODE) {
        Serial.print("Torque Request Sent: ");
        Serial.println(torque_request);
    }
}

