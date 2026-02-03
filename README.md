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
#include <LiquidCrystal_I2C.h>

// LCD
LiquidCrystal_I2C lcd(0x27,16,2); 

// Sensor Pins
const int soilPin = A0;       // Soil Moisture
const int potPin = A1;        // Potentiometer لمحاكاة pH
const int tempPin = A2;       // Temp Sensor لمحاكاة حرارة
const int gasPin = A3;        // Gas Sensor

// L293D Pins Mixer 1
const int EN1 = 5; // PWM
const int IN1 = 6;
const int IN2 = 7;

// L293D Pins Mixer 2
const int EN2 = 3; // PWM
const int IN3 = 2;
const int IN4 = 4;

// Relay Pins
const int relayFert1 = 9;
const int relayWater = 8;
const int relayFert2 = 10;

// Thresholds
int dryMoisture = 40;
int wetMoisture = 60;
float pH_low = 6.5;
float pH_high = 7.5;
int gasThreshold = 500;

// Variables
int soilVal, gasVal, potVal, tempVal;
float pHVal;

// System State
enum SystemState {IDLE, IRRIGATING, FERT1, FERT2};
SystemState state = IDLE;

// Timer for LCD
unsigned long previousLCD = 0;
const long intervalLCD = 500; // تحديث LCD كل نص ثانية

// Timer for Mixing Fertilizer
unsigned long mixStart = 0;
const unsigned long mixDuration = 10000; // 10 ثواني لكل مرحلة
int mixStep = 0; // 0 = متوقف، 1 = forward، 2 = reverse

void setup() {
  lcd.init();
  lcd.backlight();
  
  // Output Pins
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(relayFert1, OUTPUT);
  pinMode(relayFert2, OUTPUT);
  pinMode(relayWater, OUTPUT);

  // Turn off everything initially
  stopAll();
}

void loop() {
  readSensors();
  updateLCD();
  controlIrrigation();
  controlFertilization();
  handleMixing(); // التحكم بالموتورات بدون توقف
}

// قراءة الحساسات
void readSensors(){
  soilVal = map(analogRead(soilPin),0,1023,0,100);
  tempVal = analogRead(tempPin); // 0-1023
  gasVal = analogRead(gasPin);   // Gas Sensor
  potVal = analogRead(potPin);   // Pot لمحاكاة pH

  // معادلة نرنست لمحاكاة pH باستخدام Pot و Temp
  float voltage = potVal * 5.0 / 1023.0;
  float tempC = map(tempVal,0,1023,0,50);
  float slope = (0.0591 * tempC / 25.0);
  float E_ref = 2.5;
  pHVal = 7 - (voltage - E_ref)/slope;
}

// تحديث الشاشة بدون وميض
void updateLCD(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousLCD >= intervalLCD){
    previousLCD = currentMillis;
    lcd.setCursor(0,0);
    lcd.print("Moist:");
    lcd.print(soilVal);
    lcd.print("% pH:");
    lcd.print(pHVal,1);

    lcd.setCursor(0,1);
    lcd.print("Gas:");
    lcd.print(gasVal);
    lcd.print(" State:");
    lcd.print(state);
  }
}

// التحكم بالري
void controlIrrigation(){
  if(soilVal < dryMoisture){
    digitalWrite(relayWater,HIGH);
    state = IRRIGATING;
  } else if(soilVal >= wetMoisture){
    digitalWrite(relayWater,LOW);
    if(state == IRRIGATING) state = IDLE;
  }
}

// التحكم بالتسميد
void controlFertilization(){
  if(state != IRRIGATING && mixStep == 0){
    if(pHVal < pH_low || gasVal < gasThreshold){
      digitalWrite(relayFert1,HIGH);
      state = FERT1;
      mixStep = 1; // ابدأ المرحلة الأمامية لموتور 1
      mixStart = millis();
    } 
    else if(pHVal > pH_high){
      digitalWrite(relayFert2,HIGH);
      state = FERT2;
      mixStep = 1; // ابدأ المرحلة الأمامية لموتور 2
      mixStart = millis();
    } 
    else {
      stopAll();
      state = IDLE;
    }
  }
}

// التعامل مع الموتورات بدون delay
void handleMixing(){
  unsigned long currentMillis = millis();

  if(state == FERT1){
    if(mixStep == 1 && currentMillis - mixStart < mixDuration){
      // المرحلة الأمامية
      analogWrite(EN1, 204);
      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
    }
    else if(mixStep == 1 && currentMillis - mixStart >= mixDuration){
      // الانتقال للمرحلة العكسية
      mixStart = currentMillis;
      mixStep = 2;
      digitalWrite(IN1,LOW);
      digitalWrite(IN2,HIGH);
    }
    else if(mixStep == 2 && currentMillis - mixStart >= mixDuration){
      // إيقاف الموتور
      analogWrite(EN1,0);
      digitalWrite(IN1,LOW);
      digitalWrite(IN2,LOW);
      digitalWrite(relayFert1,LOW);
      mixStep = 0;
      state = IDLE;
    }
  }

  else if(state == FERT2){
    if(mixStep == 1 && currentMillis - mixStart < mixDuration){
      analogWrite(EN2, 128);
      digitalWrite(IN3,HIGH);
      digitalWrite(IN4,LOW);
    }
    else if(mixStep == 1 && currentMillis - mixStart >= mixDuration){
      mixStart = currentMillis;
      mixStep = 2;
      digitalWrite(IN3,LOW);
      digitalWrite(IN4,HIGH);
    }
    else if(mixStep == 2 && currentMillis - mixStart >= mixDuration){
      analogWrite(EN2,0);
      digitalWrite(IN3,LOW);
      digitalWrite(IN4,LOW);
      digitalWrite(relayFert2,LOW);
      mixStep = 0;
      state = IDLE;
    }
  }
}

// ايقاف كل الأجهزة
void stopAll(){
  digitalWrite(relayFert1,LOW);
  digitalWrite(relayFert2,LOW);
  digitalWrite(relayWater,LOW);
  analogWrite(EN1,0);
  analogWrite(EN2,0);
  mixStep = 0;
}

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
