# MCU Systems and Functions 

## Throttle

- [ ] Read both potentiometers of Acceleration-Pedal Position Sensor (APPS)

- [ ] Decode voltage readings into software-readable values

- [ ] Perform APPS plausibility check as per T11.8.8

- [ ] In realtime construct and send CAN-bus frame with torque request that can be understood by the Bamocar motor controller 

## Brake / Brake Light

- [ ] Send signal to activate brake light based on brake pressure sensor reading 

## APPS / Brake Plausibility Check

- [ ] Read and decode voltage readings of APPS and Brake Pressure Sensor into software-readable values

- [ ] Implement function as per rule EV2.3 

## Shutdown Circuit

- [ ] Monitor shutdown circuit breakpoints between components (see schematic). Then send 0-amp current request / 0 Nm torque request to motor controller and then open shutdown circuit relay associated with MCU (strictly in that order)

- [ ] Indicate on the dashboard that there’s a fault if circuit is broken and the cause of fault (BMS/IMD/etc)

- [ ] Send signal to shutdown circuit to close the associated MCU relay when MCU is active with no faults

- [ ] Send 0-amp current request / 0Nm torque request to motor controller and then open shutdown circuit relay if CAN bus fault is detected (strictly in that order) 

## BMS

- [ ] Communicate with Orion BMS 2 using CAN-bus to receive battery state-of-charge, voltage, current and temperature readings, and to detect BMS faults.

- [ ] Show the useful info and faults on the dashboard screen. 

## Motor Controller

- [ ] Implement a power (current) limiter for use in the endurance events. Power profiles for full power (acceleration/skidpad/sprint events) and reduced power (endurance event).

- [ ] Make this power profile selectable by the driver on the dashboard. 

## Dashboard

- [ ] Show useful data from sensors on the screen (if any)

- [ ] Receive signal from BMS and IMD for detecting faults and send signal to cockpit LEDs. 

## Precharge/TS activation procedure

- [ ] Monitor for push of precharge button in cockpit and send signal to precharge circuit to activate precharge procedure if pre-requisite conditions are met.

- [ ] Monitor precharge circuit for signal that indicates when precharge has completed.

- [ ] Display current status of precharge on dashboard screen.

- [ ] Prohibit TS Activate button from closing AIRs until precharge procedure is complete 

## Discharge Circuit

- [ ] Monitor signal indicating discharge status in discharge circuit.

- [ ] Raise fault if discharging when AIRs are closed and open MCU shutdown circuit relay.

- [ ] Display fault on dashboard screen. 

## Ready-to-drive Procedure

- [ ] Implement RTD procedure, including reading necessary sensors (brake and push button), ensuring actions are performed in the required order.

- [ ] If all actions are performed in the correct order, send signal to RTD speaker to activate RTD sound.