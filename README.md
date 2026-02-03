#include <LiquidCrystal_I2C.h>

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27,16,2); // عنوان LCD

// ---------------- Sensor Pins ----------------
const int soilPin = A0; // Soil Moisture
const int potPin  = A1; // Potentiometer لمحاكاة pH
const int tempPin = A2; // Temp Sensor لمحاكاة حرارة
const int gasPin  = A3; // Gas Sensor

// ---------------- DC Motors via L293D ----------------
// Mixer 1
const int EN1 = 5; // PWM
const int IN1 = 6;
const int IN2 = 7;

// Mixer 2
const int EN2 = 3; // PWM
const int IN3 = 2;
const int IN4 = 4;

// ---------------- Relays ----------------
const int relayFert1 = 9;
const int relayWater = 8;
const int relayFert2 = 10;

// ---------------- Thresholds ----------------
int dryMoisture = 40;
int wetMoisture = 60;
float pH_low = 6.5;
float pH_high = 7.5;
int gasThreshold = 500;

// ---------------- Variables ----------------
int soilVal, gasVal, potVal, tempVal;
float pHVal;

// ---------------- System State ----------------
enum SystemState {IDLE, IRRIGATING, FERT1, FERT2};
SystemState state = IDLE;

// ---------------- Timer for LCD ----------------
unsigned long previousLCD = 0;
const long intervalLCD = 500; // تحديث LCD كل نصف ثانية

// ---------------- Timer for Mixing ----------------
unsigned long mixStart = 0;
const unsigned long mixDuration = 10000; // 10 ثواني لكل مرحلة
int mixStep = 0; // 0 = متوقف، 1 = forward، 2 = reverse

// ---------------- Setup ----------------
void setup() {
  lcd.init();
  lcd.backlight();
  
  // Motor pins
  pinMode(EN1, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Relay pins
  pinMode(relayFert1, OUTPUT);
  pinMode(relayFert2, OUTPUT);
  pinMode(relayWater, OUTPUT);

  stopAll(); // تأكد من إيقاف كل الأجهزة عند التشغيل
}

// ---------------- Loop ----------------
void loop() {
  readSensors();
  updateLCD();
  controlIrrigation();
  controlFertilization();
  handleMixing(); // التحكم بالموتورات بدون توقف
}

// ---------------- Sensor Reading ----------------
void readSensors(){
  soilVal = map(analogRead(soilPin),0,1023,0,100);
  tempVal = analogRead(tempPin);
  gasVal = analogRead(gasPin);
  potVal = analogRead(potPin);

  // حساب pH باستخدام Potentiometer و Temp
  float voltage = potVal * 5.0 / 1023.0;
  float tempC = map(tempVal,0,1023,0,50);
  float slope = (0.0591 * tempC / 25.0);
  float E_ref = 2.5;
  pHVal = 7 - (voltage - E_ref)/slope;
}

// ---------------- LCD Update ----------------
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
    switch(state){
      case IDLE: lcd.print("IDLE  "); break;
      case IRRIGATING: lcd.print("IRRIG "); break;
      case FERT1: lcd.print("FERT1 "); break;
      case FERT2: lcd.print("FERT2 "); break;
    }
  }
}

// ---------------- Irrigation Control ----------------
void controlIrrigation(){
  if(soilVal < dryMoisture){
    digitalWrite(relayWater,HIGH); // تشغيل الريليه
    state = IRRIGATING;
  } else if(soilVal >= wetMoisture){
    digitalWrite(relayWater,LOW); // إيقاف الريليه
    if(state == IRRIGATING) state = IDLE;
  }
}

// ---------------- Fertilization Control ----------------
void controlFertilization(){
  if(state != IRRIGATING && mixStep == 0){
    if(pHVal < pH_low || gasVal < gasThreshold){
      digitalWrite(relayFert1,HIGH);
      state = FERT1;
      mixStep = 1; // المرحلة الأمامية لموتور 1
      mixStart = millis();
    } 
    else if(pHVal > pH_high){
      digitalWrite(relayFert2,HIGH);
      state = FERT2;
      mixStep = 1; // المرحلة الأمامية لموتور 2
      mixStart = millis();
    } 
    else {
      stopAll();
      state = IDLE;
    }
  }
}

// ---------------- Mixing Handler ----------------
void handleMixing(){
  unsigned long currentMillis = millis();

  if(state == FERT1){
    if(mixStep == 1 && currentMillis - mixStart < mixDuration){
      analogWrite(EN1, 204); 
      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
    }
    else if(mixStep == 1 && currentMillis - mixStart >= mixDuration){
      mixStart = currentMillis;
      mixStep = 2;
      digitalWrite(IN1,LOW);
      digitalWrite(IN2,HIGH);
    }
    else if(mixStep == 2 && currentMillis - mixStart >= mixDuration){
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

// ---------------- Stop All Devices ----------------
void stopAll(){
  digitalWrite(relayFert1,LOW);
  digitalWrite(relayFert2,LOW);
  digitalWrite(relayWater,LOW);
  analogWrite(EN1,0);
  analogWrite(EN2,0);
  mixStep = 0;
}
