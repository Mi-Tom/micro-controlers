#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <BleGamepad.h>

#define RADTODEG 57.2958
#define MAXTILT 35.0      // Náklon pro 100% výchylku Roll/Pitch v simulátoru
#define MAXYAWANGLE 60.0  // Úhel pro 100% výchylku zatáčení (Yaw)
#define BUTTON_DELAY 50

#define POTPIN 33   // Pin pro potenciometr (Plyn)
#define PIN_BAT 32  // Piny pro baterii (ponechány pro zachování kódu)
#define PIN_REF 35

// ---- Kalibrace offsetů ----
float rollOffset = 0;
float pitchOffset = 0;
float yawOffset = 0;

// ---- Gyroskop a AkcelerometR ----
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long lastTime;

float angleX = 0;
float angleY = 0;
float yaw_input_prev = 0.0;
float yaw_angle = 0;

// ---- Potenciometr Plyn (Throttle) ----
float joy = 0;
float joyCenter = 0.32;  // Střed změřený v setupu

// ---- LED Diody ----
int ledPins[] = { 13, 12, 15, 2, 4 };
int ledCount = 5;

// ---- Tlačítka ----
struct Button {
  int pin;
  unsigned long lastPush;
  bool lastState;
  uint8_t gamepadButton;  // ID tlačítka posílané do PC
  bool lastSentState;     // NOVÉ: Paměť pro odeslaný stav (kvůli AutoReportu)
};

Button clbBtn = { 23, 0, HIGH, 0, HIGH };          // Kalibrace (jen interní funkce)
Button batBtn = { 14, 0, HIGH, BUTTON_3, HIGH };   // Tlačítko baterie -> Button 3 v PC
Button emeBtn = { 27, 0, HIGH, 0, HIGH };          // Emergency (jen interní funkce)
Button AUX1Btn = { 19, 0, HIGH, BUTTON_1, HIGH };  // AUX1 (např. ARM) -> Button 1 v PC
Button AUX2Btn = { 18, 0, HIGH, BUTTON_2, HIGH };  // AUX2 (režim) -> Button 2 v PC

// ---- Stavy ----
bool isEmergency = false;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const int blinkInterval = 300;

int p = 0;  // Divider pro sériový výpis

// Konfigurace BLE Gamepadu
BleGamepadConfiguration bleGamepadConfig;
BleGamepad bleGamepad("Drone Sim Controller", "DIY-Dev", 100);

void turnOffLeds() {
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
}

void calibrateController() {
  rollOffset = angleX;
  pitchOffset = angleY;
  yawOffset = yaw_angle;
  Serial.println("KONTROLER ZKALIBROVAN (Nove offsety nastaveny)");
}

void setup() {
  Serial.begin(115200);

  // Potenciometr nastavení
  analogReadResolution(12);        // 0–4095
  analogSetAttenuation(ADC_11db);  // 0–3.3V

  // Načtení klidové polohy středu páčky plynu
  long sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += analogRead(POTPIN);
    delay(5);
  }
  joyCenter = (sum / 50.0) / 4095.0;

  // Inicializace I2C sběrnice pro MPU6050
  Wire.begin(21, 22);
  Wire.setClock(400000);  // Rychlá I2C komunikace
  accelgyro.initialize();

  // Nastavení pinů pro LED a tlačítka
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  turnOffLeds();

  pinMode(clbBtn.pin, INPUT_PULLUP);
  pinMode(batBtn.pin, INPUT_PULLUP);
  pinMode(emeBtn.pin, INPUT_PULLUP);
  pinMode(AUX1Btn.pin, INPUT_PULLUP);
  pinMode(AUX2Btn.pin, INPUT_PULLUP);

  // Nastavení parametrů Bluetooth herního ovladače
  bleGamepadConfig.setAutoReport(true);  // ZAPNUTO: Knihovna posílá změny automaticky
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepadConfig.setButtonCount(3);  // AUX1, AUX2, BAT

  // Povolíme pouze osy X, Y, Z, Rz (Roll, Pitch, Throttle, Yaw)
  bleGamepadConfig.setWhichAxes(true, true, true, true, false, false, false, false);
  bleGamepad.begin(&bleGamepadConfig);

  lastTime = micros();
  Serial.println("Simulátorový Bluetooth ovladač spuštěn s AutoReportem!");
}

void loop() {
  unsigned long now = millis();

  // ---- Časová delta pro integraci gyroskopu ----
  unsigned long currentTime = micros();
  float dt = (currentTime - lastTime) / 1000000.0;
  lastTime = currentTime;
  if (dt > 0.05 || dt <= 0) dt = 0.01;

  // ---- Obsluha Emergency tlačítka ----
  bool emeCurrentState = digitalRead(emeBtn.pin);
  if (emeBtn.lastState == HIGH && emeCurrentState == LOW) {
    if (now - emeBtn.lastPush > BUTTON_DELAY) {
      emeBtn.lastPush = now;
      isEmergency = !isEmergency;
      if (isEmergency) Serial.println("EMERGENCY: Aktivováno!");
      else Serial.println("EMERGENCY: Deaktivováno.");
    }
  }
  emeBtn.lastState = emeCurrentState;

  // ---- Obsluha Kalibračního tlačítka ----
  bool clbCurrentState = digitalRead(clbBtn.pin);
  if (clbBtn.lastState == HIGH && clbCurrentState == LOW) {
    if (now - clbBtn.lastPush > BUTTON_DELAY) {
      clbBtn.lastPush = now;
      calibrateController();
    }
  }
  clbBtn.lastState = clbCurrentState;

  // ---- Výpočty polohy (MPU6050) ----
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float accAngleX = atan2(ay, az) * RADTODEG;
  float accAngleY = atan2(-ax, sqrt((long)ay * ay + (long)az * az)) * RADTODEG;

  float gx_dps = gx / 131.0;
  float gy_dps = gy / 131.0;
  float alpha = 0.98;

  angleX = alpha * (angleX + gx_dps * dt) + (1 - alpha) * accAngleX;
  angleY = alpha * (angleY + gy_dps * dt) + (1 - alpha) * accAngleY;

  float roll_input = (angleX - rollOffset) / MAXTILT;
  float pitch_input = (angleY - pitchOffset) / MAXTILT;

  float yaw_rate = gz / 131.0;
  if (abs(yaw_rate) < 1.0) yaw_rate = 0;

  yaw_angle += yaw_rate * dt;
  yaw_angle *= 0.995;

  float yaw_input = (yaw_angle - yawOffset) / MAXYAWANGLE;
  yaw_input = 0.9 * yaw_input_prev + 0.1 * yaw_input;
  yaw_input_prev = yaw_input;

  if (abs(roll_input) < 0.05) roll_input = 0;
  if (abs(pitch_input) < 0.05) pitch_input = 0;

  roll_input = constrain(roll_input, -1.0, 1.0);
  pitch_input = constrain(pitch_input, -1.0, 1.0);
  yaw_input = constrain(yaw_input, -1.0, 1.0);

  // ---- Čtení a převod páčky Plynu (Throttle) ----
  int raw = analogRead(POTPIN);
  float joyRaw = raw / 4095.0;
  joy = 0.85 * joy + 0.15 * joyRaw;

  float mapped_input;
  if (joy >= joyCenter) {
    mapped_input = (joy - joyCenter) / (1.0 - joyCenter);
  } else {
    mapped_input = (joy - joyCenter) / joyCenter;
  }

  float final_throttle = (mapped_input + 1.0) / 2.0;
  final_throttle = constrain(final_throttle, 0.0, 1.0);

  // ---- Odesílání dat do Bluetooth ----
  if (bleGamepad.isConnected()) {
    
    if (!isEmergency) {
      turnOffLeds();

      // Přepočet rozsahů pro osy (0 až 32767)
      uint16_t pcRoll  = (roll_input + 1.0) * 16383.5;
      uint16_t pcPitch = (pitch_input + 1.0) * 16383.5;
      uint16_t pcYaw   = (yaw_input + 1.0) * 16383.5;
      uint16_t pcThr   = final_throttle * 32767;

      // S AutoReportem stačí poslat osy – knihovna je rovnou odešle do PC
      bleGamepad.setAxes(pcRoll, pcPitch, pcThr, pcYaw, 0, 0, 0, 0);

      // ---- TLAČÍTKA: Posílají se JEN při reálné změně stavu ----
      
      // AUX1 Tlačítko
      bool aux1State = digitalRead(AUX1Btn.pin);
      if (aux1State != AUX1Btn.lastSentState) {
        if (aux1State == LOW) bleGamepad.press(AUX1Btn.gamepadButton);
        else bleGamepad.release(AUX1Btn.gamepadButton);
        AUX1Btn.lastSentState = aux1State;
      }

      // AUX2 Tlačítko
      bool aux2State = digitalRead(AUX2Btn.pin);
      if (aux2State != AUX2Btn.lastSentState) {
        if (aux2State == LOW) bleGamepad.press(AUX2Btn.gamepadButton);
        else bleGamepad.release(AUX2Btn.gamepadButton);
        AUX2Btn.lastSentState = aux2State;
      }

      // Battery Tlačítko
      bool batState = digitalRead(batBtn.pin);
      if (batState != batBtn.lastSentState) {
        if (batState == LOW) bleGamepad.press(batBtn.gamepadButton);
        else bleGamepad.release(batBtn.gamepadButton);
        batBtn.lastSentState = batState;
      }

    } else {
      // Emergency aktivní: Vycentrovat osy a uvolnit držená tlačítka
      bleGamepad.setAxes(16383, 16383, 0, 16383, 0, 0, 0, 0);
      
      if (AUX1Btn.lastSentState == LOW) { bleGamepad.release(BUTTON_1); AUX1Btn.lastSentState = HIGH; }
      if (AUX2Btn.lastSentState == LOW) { bleGamepad.release(BUTTON_2); AUX2Btn.lastSentState = HIGH; }
      if (batBtn.lastSentState == LOW)  { bleGamepad.release(BUTTON_3); batBtn.lastSentState = HIGH; }

      // Nouzové blikání LED
      if (now - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = now;
        blinkState = !blinkState;
        for (int i = 0; i < ledCount; i++) {
          digitalWrite(ledPins[i], blinkState ? LOW : HIGH);
        }
      }
    }
    
    // Poznámka: bleGamepad.sendReport() zde s AutoReportem už vůbec nevoláme!
  }

  // Debug výpis do Serial monitoru
  p++;
  if (p >= 40) {
    Serial.printf("Roll: %.2f | Pitch: %.2f | Yaw: %.2f | Throttle: %.2f | Connected: %s\n",
                  roll_input, pitch_input, yaw_input, final_throttle, bleGamepad.isConnected() ? "YES" : "NO");
    p = 0;
  }

  delay(5);
}