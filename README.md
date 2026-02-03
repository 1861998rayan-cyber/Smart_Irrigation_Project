# Smart_Irrigation_Project
Arduino-based automated irrigation and fertilization system using sensors to optimize water and fertilizer usage.

## Overview
This is an **Arduino-based smart irrigation system** that monitors soil moisture, pH (simulated), temperature, and gas levels to control water and fertilizer pumps automatically.  

The system uses **DC motors with H-Bridge (L293D)** for fertilizer mixers and relays for water and fertilizer pumps. It provides **real-time feedback on an LCD** and ensures efficient resource use.

---

## Features
- Automatic irrigation based on soil moisture levels.
- Fertilizer mixing with two separate DC motors.
- Real-time LCD display of:
  - Soil moisture
  - pH value
  - Gas sensor readings
  - System state (IDLE, IRRIGATING, FERT1, FERT2)
- Adjustable moisture and pH thresholds.
- Non-blocking code using `millis()` for smooth operation.
- Expandable for IoT or remote monitoring.

---

## Components
- Arduino Uno (or compatible board)
- Soil Moisture Sensor
- Potentiometer (simulated pH)
- Temperature sensor (simulated)
- Gas sensor (simulated)
- 2 x DC Motors (for mixers)
- 2 x L293D H-Bridge modules
- 3 x Relays (Water and Fertilizer pumps)
- LiquidCrystal I2C (16x2) Display
- Jumper wires & breadboard
- 12V Power supply (for motors)
- LEDs (optional for indicators)

---

## Circuit Connections

**Water Pump Relay**  
- `relayWater` → Controls irrigation pump  

**Fertilizer Pumps**  
- `relayFert1` → Fertilizer Pump 1  
- `relayFert2` → Fertilizer Pump 2  

**DC Motors (Mixers)**  
- Mixer 1: `EN1`, `IN1`, `IN2`  
- Mixer 2: `EN2`, `IN3`, `IN4`  

**Sensors**  
- A0 → Soil Moisture  
- A1 → Potentiometer (pH simulation)  
- A2 → Temperature  
- A3 → Gas  

---

## How It Works
1. **Sensor Reading:** Continuously reads soil moisture, pH, temperature, and gas levels.  
2. **Irrigation Control:**  
   - If soil moisture < `dryMoisture` → Water pump turns ON.  
   - If soil moisture ≥ `wetMoisture` → Water pump turns OFF.  
3. **Fertilization Control:**  
   - If pH < `pH_low` or gas < `gasThreshold` → Fertilizer 1 starts.  
   - If pH > `pH_high` → Fertilizer 2 starts.  
   - Mixers operate forward for 10 seconds, then reverse to complete mixing.  
4. **Display:** LCD shows soil moisture, pH, gas, and system state.  
5. **Non-blocking Operation:** Uses `millis()` timers to allow simultaneous sensor reading, motor operation, and display updates.

---

## Thresholds
- Dry Moisture: 40%  
- Wet Moisture: 60%  
- pH Low: 6.5  
- pH High: 7.5  
- Gas Threshold: 500  

> These values can be adjusted in the Arduino code for different plants or soil conditions.

---

## Project Structure
Smart_Irrigation_Project/
  # Arduino code
├── README.md # Project description and instructions
├── LICENSE # MIT License
├── .gitignore # Ignore unnecessary files
└── Circuit_Diagram.jpg<img width="1487" height="757" alt="image" src="https://github.com/user-attachments/assets/0e3424e6-56ce-43ca-8224-c18f68d38971" />
 # Optional schematic image


---

## Notes
- Ensure the 12V power supply matches motor specifications.
- Relays are active HIGH (ON when pin HIGH). Adjust if using different relay modules.
- PWM values (analogWrite) can be adjusted to control motor speed.
- Expandable for IoT monitoring or mobile notifications.

---

## License
This project is licensed under the MIT License – see LICENSE file for details.
