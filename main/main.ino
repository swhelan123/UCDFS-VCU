#define BRAKE_LIGHT_PIN 9     // Pin 9 for LED (simulating brake light)
#define BRAKE_PRESSURE_SENSOR_PIN A0  // Analog pin A0 for potentiometer (brake pressure sensor)
#define THRESHOLD 500         // Threshold for when brake light should activate

void setup() {
  // Initialize brake light pin as output
  pinMode(BRAKE_LIGHT_PIN, OUTPUT);

  // Initialize serial communication for debugging over USB
  Serial.begin(9600);

  
}

int cycle = 0;

void loop() {
  // read analog input from brake pressure sensor
  int brakePressure = analogRead(BRAKE_PRESSURE_SENSOR_PIN);

  // Print the brake pressure value to the serial monitor for debugging
  Serial.print("Brake Pressure (A0): ");
  Serial.println(brakePressure);

  // if brake pressure is above the threshold, turn on the brake light
  if (brakePressure > THRESHOLD) {
    digitalWrite(BRAKE_LIGHT_PIN, HIGH);
  } else {
    digitalWrite(BRAKE_LIGHT_PIN, LOW);
  }

  delay(50);  // small delay to stabilize readings and not overwhelm the serial output, probs remove this in final code
}
