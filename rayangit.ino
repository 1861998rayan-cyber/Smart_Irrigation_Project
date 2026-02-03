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

