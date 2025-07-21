#include "header.h" // Assumes this includes Nextion.h and other necessary headers

// --- Nextion Configuration ---
// IMPORTANT: The component names ("dbg_bms_v", etc.) MUST MATCH the "objname" 
// property you set for each text box in your Nextion Editor on the dev page.

// -- Page 1: Main Driving Display --
NexText p1_speed = NexText(1, 4, "t1");
NexText p1_soc   = NexText(1, 6, "t3");

// -- Page 3: Dev Page Components --
// [STATUS] Section
NexText dbg_bms_v      = NexText(3, 4, "dbg_bms_v");     // BMS Voltage
NexText dbg_bms_soc    = NexText(3, 5, "dbg_bms_soc");    // BMS State of Charge
NexText dbg_bms_flt    = NexText(3, 6, "dbg_bms_flt");    // BMS Fault Status
NexText dbg_brk_r      = NexText(3, 7, "dbg_brk_r");      // Rear Brake Pressure
NexText dbg_brk_f      = NexText(3, 8, "dbg_brk_f");      // Front Brake Pressure
NexText dbg_brk_c      = NexText(3, 9, "dbg_brk_c");      // Combined Brake Pressure
NexText dbg_brk_lt     = NexText(3, 10, "dbg_brk_lt");     // Brake Light Status
NexText dbg_apps       = NexText(3, 11, "dbg_apps");       // APPS Pedal Position
NexText dbg_torq_req   = NexText(3, 12, "dbg_torq_req");   // Torque Request
NexText dbg_mpu        = NexText(3, 13, "dbg_mpu");       // MPU Angle (X,Y,Z)

// ----- BAMOCAR TEST STATUS ----- Section
NexText dbg_rpm        = NexText(3, 14, "dbg_rpm");       // Motor RPM
NexText dbg_torq_act   = NexText(3, 15, "dbg_torq_act");  // Actual Motor Torque
NexText dbg_mtemp      = NexText(3, 16, "dbg_mtemp");     // Motor Temperature
NexText dbg_bamo_stat  = NexText(3, 17, "dbg_bamo_stat"); // Bamocar Controller Status
NexText dbg_bamo_rdy   = NexText(3, 18, "dbg_bamo_rdy");  // Bamocar Ready State

// Buffer for formatting text before sending
char text_buffer[64];

void nextion_setup() {
  Serial3.begin(9600); 
  delay(500);
  Serial.print("Initializing Nextion display...");
  nexInit();
  Serial.println("Nextion display initialized (Dev Page Mode).");
}

/**
 * @brief Updates the main driving display (Page 1)
 */
void nextion_update_driver_page() {
  sprintf(text_buffer, "%d", vehicleSpeed);
  p1_speed.setText(text_buffer);

  sprintf(text_buffer, "%.0f%%", can.getPackSOC()); // Fix: use %.0f for double
  p1_soc.setText(text_buffer);
}

/**
 * @brief Updates all the individual fields on the dev/debug page based on the provided Serial block.
 */
void nextion_update_dev_page() {
  // This function assumes global variables like 'brakePressureRear', 'mpuInitialized', etc. are accessible.
  double pedal_position = get_apps_reading(); // Get current pedal reading

  // --- [STATUS] Section ---
  sprintf(text_buffer, "%.1f V", can.getPackVoltage());
  dbg_bms_v.setText(text_buffer);

  sprintf(text_buffer, "%.0f %%", can.getPackSOC()); // Fix: use %.0f for double
  dbg_bms_soc.setText(text_buffer);

  dbg_bms_flt.setText(can.getSystemError() ? "FAULT" : "OK");

  sprintf(text_buffer, "%d", brakePressureRear);
  dbg_brk_r.setText(text_buffer);

  sprintf(text_buffer, "%d", brakePressureFront);
  dbg_brk_f.setText(text_buffer);

  sprintf(text_buffer, "%d", brakePressureCombined);
  dbg_brk_c.setText(text_buffer);

  dbg_brk_lt.setText(digitalRead(BRAKE_LIGHT_PIN) ? "ON" : "OFF");

  sprintf(text_buffer, "%.1f", pedal_position);
  dbg_apps.setText(text_buffer);

  int torqueInt = (int)((pedal_position / 100.0) * 32767);
  sprintf(text_buffer, "%d", torqueInt);
  dbg_torq_req.setText(text_buffer);

  if (mpuInitialized) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sprintf(text_buffer, "%.1f,%.1f,%.1f", a.acceleration.x, a.acceleration.y, a.acceleration.z);
    dbg_mpu.setText(text_buffer);
  } else {
    dbg_mpu.setText("Not Init");
  }

  // --- BAMOCAR TEST STATUS Section ---
  sprintf(text_buffer, "%.0f RPM", can.getMotorRPM()); 
  dbg_rpm.setText(text_buffer);

  sprintf(text_buffer, "%.1f Nm", can.getMotorTorque());
  dbg_torq_act.setText(text_buffer);

  sprintf(text_buffer, "%.1f C", can.getMotorTemp());
  dbg_mtemp.setText(text_buffer);

  dbg_bamo_stat.setText(can.getSystemError() ? "ERROR" : "OK");
  dbg_bamo_rdy.setText(can.getSystemError() ? "NOT READY" : "READY");
}
