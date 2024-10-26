#define BRAKE_LIGHT_PIN 9                // Pin 9 for brake light
#define BRAKE_PRESSURE_SENSOR_PIN A0     // Analog pin A0 for potentiometer (brake pressure sensor)
#define THRESHOLD 500                    // Threshold for when brake light should activate (needs calibration)
#define DEBUG_MODE 1                     // set debug mode to true in order to read serial outputs

void setup() {
  // Initialize brake light pin as output
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);

  // Initialize serial communication for debugging over USB
  Serial.begin(9600);
}

void loop() {
  // function call to read brake pressure and handle brake light logic
  brake_light();

  if (DEBUG_MODE);
    delay(200);  // small delay to stabilize readings and not overwhelm the serial output, probs remove this in final code
}

void brake_light(){
    // read analog input from brake pressure sensor
  int brakePressure = analogRead(BRAKE_PRESSURE_SENSOR_PIN);
  
  if (DEBUG_MODE){
    // Print the brake pressure value to the serial monitor for debugging
    // Serial.print("Brake Pressure (A0): ");
    Serial.println(brakePressure);
  }

  // if brake pressure is above the threshold, turn on the brake light
  if (brakePressure > THRESHOLD) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
    if (DEBUG_MODE)
      Serial.println("Brake Light On\n");      // print brake light status to serial monitor 
  } else {
    digitalWrite(BRAKE_LIGHT_PIN, LOW);
    if (DEBUG_MODE)
      Serial.println("Brake Light Off\n");     // print brake light status to serial monitor
  }
}