#include <mcp_can.h>
#include <SPI.h>

// CAN Bus setup
#define CAN_Module_INT 2  // CAN INT pin
MCP_CAN CAN_Module(10);   // Set CS to pin 10 for MCP2515

#define BRAKE_LIGHT_PIN 9             // Pin 9 for brake light
#define BRAKE_PRESSURE_SENSOR_PIN A0  // Analog pin A0 for potentiometer (brake pressure sensor)
#define THRESHOLD 500                 // Threshold for when brake light should activate (needs calibration)
#define DEBUG_MODE 1                  // set debug mode to true in order to read serial outputs

// GLOBAL VARIABLES
// *** BMS ***
float cellVoltage = 0.0;
int stateOfCharge = 0;

void setup() {
  // Initialize brake light pin as output
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);

  // Initialize serial communication for debugging over USB
  Serial.begin(9600);

  // Initialize CAN Module
  if (CAN_Module.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("CAN Module Initialized!");
  } else {
    Serial.println("Error Initializing CAN Module!");
    while (1)
      ;
  }

  CAN_Module.setMode(MCP_NORMAL);  // Set normal operating mode for CAN
  pinMode(CAN_Module_INT, INPUT);
}

void loop() {
  // function call to read brake pressure and handle brake light logic
  brake_light();

  // function to read CAN messages and update variables
  readCANData();

  if (DEBUG_MODE)
    ;
  delay(200);  // small delay to stabilize readings and not overwhelm the serial output, probs remove this in final code
}

void brake_light() {
  // read analog input from brake pressure sensor
  int brakePressure = analogRead(BRAKE_PRESSURE_SENSOR_PIN);

  if (DEBUG_MODE) {
    // Print the brake pressure value to the serial monitor for debugging
    // Serial.print("Brake Pressure (A0): ");
    Serial.println(brakePressure);
  }

  // if brake pressure is above the threshold, turn on the brake light
  if (brakePressure > THRESHOLD) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
    if (DEBUG_MODE)
      Serial.println("Brake Light On\n");  // print brake light status to serial monitor
  } else {
    digitalWrite(BRAKE_LIGHT_PIN, LOW);
    if (DEBUG_MODE)
      Serial.println("Brake Light Off\n");  // print brake light status to serial monitor
  }
}

void readCANData() {
  if (!digitalRead(CAN_Module_INT)) {  // If CAN message is received
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    CAN_Module.readMsgBuf(&rxId, &len, rxBuf);

    // Check message ID and parse data
    switch (rxId) {
      case 0x100:                                            // Example ID for cell voltage
        cellVoltage = ((rxBuf[0] << 8) | rxBuf[1]) * 0.001;  // Convert bytes to voltage (e.g., in mV)
        if (DEBUG_MODE) {
          Serial.print("Cell Voltage: ");
          Serial.println(cellVoltage);
        }
        break;

      case 0x101:                  // Example ID for state of charge
        stateOfCharge = rxBuf[0];  // SOC might be directly in a single byte
        if (DEBUG_MODE) {
          Serial.print("State of Charge: ");
          Serial.println(stateOfCharge);
        }
        break;

        // Add additional cases for other CAN IDs as needed

      default:
        if (DEBUG_MODE) Serial.println("Unknown CAN ID received");
        break;
    }
  }
}