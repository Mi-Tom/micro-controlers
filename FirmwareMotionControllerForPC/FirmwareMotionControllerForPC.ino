#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <BleGamepad.h>

#define RADTODEG 57.2958
#define MAXTILT 35.0
#define MAXYAWANGLE 60.0
#define BUTTON_DELAY 50

#define POTPIN 33   // Pin pro potenciomentr
#define PIN_BAT 32  // Pin pro dělič baterie
#define PIN_REF 35  //Pin pro ref napeti

// ---- Bluetooth Gamepad Setup ----
BleGamepad bleGamepad("Drone Motion Controller", "DIY-Dev", 100);

//---- Kalibrace ---
float rollOffset = 0;
float pitchOffset = 0;
float yawOffset = 0;

//---- Gyroskop ----
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long lastTime;

float angleX = 0;
float angleY = 0;
float yaw_input_prev = 0.0;
static float yaw_angle = 0;

//---- Baterka ----
const float R1 = 22000.0;  //100K
const float R2 = 22000.0;
const float VOLT_RATIO = (R1 + R2) / R2;

// ---- JOYSTICK THROTTLE KALIBRACE ----
const float JOY_CENTER = 0.32;    // střed (změřená klidová hodnota)
const float JOY_DEADZONE = 0.02;  // mrtvá zóna kolem středu
float joy = 0;
float joyCenter = 0;

int ledPins[] = { 13, 12, 15, 2, 4 };  // Jde od cervene po zelenou
int ledCount = 5;

int batteryLedValue = 0;

//---- Tlacitka ----
struct Button {
  int pin;
  unsigned long lastPush;
  bool lastState;
  unsigned long offTime;
};

Button clbBtn = { 23, 0, HIGH, 0 };  //kalibracni tlacitko

Button batBtn = { 14, 0, HIGH, 0 };
Button emeBtn = { 27, 0, HIGH, 0 };

Button AUX1Btn = { 19, 0, HIGH, 0 };  // - Žlutý kabel, AUX1 - ARM
Button AUX2Btn = { 18, 0, HIGH, 0 };  // - Zelený kabel, AUX2 - ALTHOLD/ANGLE

//---- Emergency mod ----
bool isEmergency = false;

// ---- Emergency blink ----
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const int blinkInterval = 300;

// ---- AUX LED animace ----
bool AUX1Animating = false;
bool AUX2Animating = false;

//---- Emergency mod ----
bool isAUX1 = false;
bool isAUX2 = true;

bool AUX1LongPressDone = false;
bool AUX2LongPressDone = false;

//---- ESP NOW komunikace ----
uint8_t broadcastAddress[] = { 0x08, 0xb6, 0x1f, 0xb8, 0x4c, 0x50 };  //MAC adresa

typedef struct struct_message {
  float roll;
  float pitch;
  float yaw;
  float throttle;
  bool aux1;
  bool aux2;
} struct_message;

struct_message message;

// ---- Pomocne pro vypis ----
int p = 0;

// =====================TESTOVANI_NAPETI_BATERIE=====================

int getBatteryLedCount() {
  float refRaw = analogRead(PIN_REF);

  int samples = 10;
  uint32_t sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(PIN_BAT);
  }

  float batRaw = sum / (float)samples;

  float voltage = (batRaw / refRaw) * 2.5 * VOLT_RATIO;

  int percent = constrain((voltage - 3.2) * 100, 0, 100);

  int ledsToLight = 0;

  // HYSTEREZE + "plné držení"
  if (percent >= 80) {
    ledsToLight = 5;
  } else if (percent >= 60) {
    ledsToLight = 4;
  } else if (percent >= 40) {
    ledsToLight = 3;
  } else if (percent >= 20) {
    ledsToLight = 2;
  } else if (percent >= 0) {
    ledsToLight = 1;
  } else {
    ledsToLight = 0;
  }

  return constrain(ledsToLight, 0, ledCount);
}


void showBatteryLeds(int ledsToLight) {

  for (int i = 0; i < ledCount; i++) {

    if (i < ledsToLight) {
      digitalWrite(ledPins[i], LOW);
    } else {
      digitalWrite(ledPins[i], HIGH);
    }
  }
}

void showAux1Progress(bool turningOn, float progress) {

  progress = constrain(progress, 0.0, 1.0);

  int leds = progress * ledCount + 0.999;

  if (turningOn) {

    // ZAPÍNÁNÍ
    // [1....]
    // [11...]
    // [111..]

    for (int i = 0; i < ledCount; i++) {

      if (i < leds) {
        digitalWrite(ledPins[i], LOW);
      } else {
        digitalWrite(ledPins[i], HIGH);
      }
    }

  } else {

    // VYPÍNÁNÍ
    // [11111]
    // [1111.]
    // [111..]

    int ledsOn = ledCount - leds;

    for (int i = 0; i < ledCount; i++) {

      if (i < ledsOn) {
        digitalWrite(ledPins[i], LOW);
      } else {
        digitalWrite(ledPins[i], HIGH);
      }
    }
  }
}

void showAux2Progress(bool turningOn, float progress) {

  progress = constrain(progress, 0.0, 1.0);

  int step = constrain(progress * 4.0, 0, 3);

  // nejdřív všechno zhasnout
  turnOffLeds();

  if (turningOn) {

    // ZAPÍNÁNÍ
    // [.....]
    // [..1..]
    // [.111.]
    // [11111]

    if (step >= 1) {
      digitalWrite(ledPins[2], LOW);
    }

    if (step >= 2) {
      digitalWrite(ledPins[1], LOW);
      digitalWrite(ledPins[3], LOW);
    }

    if (step >= 3) {
      digitalWrite(ledPins[0], LOW);
      digitalWrite(ledPins[4], LOW);
    }

  } else {

    // VYPÍNÁNÍ
    // [11111]
    // [.111.]
    // [..1..]
    // [.....]

    // začátek = vše zapnuto
    for (int i = 0; i < ledCount; i++) {
      digitalWrite(ledPins[i], LOW);
    }

    if (step >= 1) {
      digitalWrite(ledPins[0], HIGH);
      digitalWrite(ledPins[4], HIGH);
    }

    if (step >= 2) {
      digitalWrite(ledPins[1], HIGH);
      digitalWrite(ledPins[3], HIGH);
    }

    if (step >= 3) {
      digitalWrite(ledPins[2], HIGH);
    }
  }
}

void turnOffLeds() {
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
}

// ==============================ESP NOW=============================

/*void ESPNOW_send(float roll_input, float pitch_input, float yaw_input, float throttle_input, bool a1, bool a2) {
  message.roll = roll_input;
  message.pitch = pitch_input;
  message.yaw = yaw_input;
  message.throttle = throttle_input;
  message.aux1 = a1;
  message.aux2 = a2;

  esp_err_t outcome = esp_now_send(broadcastAddress, (uint8_t *)&message, sizeof(message));

  // OTRAVNY VYPIS K ESP-NOW
  /*if (outcome == ESP_OK) {
    Serial.println("Message sent successfully!");
  } else {
    Serial.println("Error sending the message");
  }/
}*/

/*void data_sent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  /*Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");/
}*/

// ========================POMOCNY_VYPIS_UDAJU=======================
void printout_data(float roll_input, float pitch_input, float yaw_input, float throttle_input, bool a1, bool a2) {
  p++;
  if (p == 10) {
    Serial.print("Roll:\t");
    Serial.println(roll_input);
    Serial.print("Pitch:\t");
    Serial.println(pitch_input);
    Serial.print("Yaw:\t");
    Serial.println(yaw_input);
    //Serial.print("Raw Pontentiometer:\t"); //když tak přidat do formalnich parametru fce
    //Serial.println(pot_value);
    Serial.print("Throttle:\t");
    Serial.println(throttle_input);
    if (a1) {
      Serial.print("AUX1 = TRUE\t|\t");
    } else {
      Serial.print("AUX1 = FALSE\t|\t");
    }
    if (a2) {
      Serial.println("AUX2 = TRUE");
    } else {
      Serial.println("AUX2 = FALSE");
    }
    Serial.println("---------------");
    p = 0;
  }
}

//========================KALIBRACE===================================
void calibrateController() {

  rollOffset = angleX;
  pitchOffset = angleY;
  yawOffset = yaw_angle;

  Serial.println("================================");
  Serial.println("KONTROLER ZKALIBROVAN");
  Serial.println("Nova neutralni pozice nastavena");
  Serial.println("================================");
}
// ==============================SETUP===============================

void setup() {
  // Potenciometr
  analogReadResolution(12);        // 0–4095
  analogSetAttenuation(ADC_11db);  // rozsah cca 0–3.3V

  // kalibrace středu joysticku
  long sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += analogRead(POTPIN);
    delay(5);
  }

  joyCenter = (sum / 50.0) / 4095.0;

  // Kalibracni tlacitko
  pinMode(clbBtn.pin, INPUT_PULLUP);

  // Akcelerometr a Gyroskop
  Wire.begin(21, 22);  // Inicializace I2C na pinech ESP32
  Serial.begin(115200);

  Serial.println("Inicializace I2C zarizeni...");
  accelgyro.initialize();

  bleGamepad.begin();

  Serial.println("Testovani pripojeni...");
  Serial.println(accelgyro.testConnection() ? "MPU6500 pripojen uspesne" : "MPU6500 pripojeni selhalo");

  lastTime = micros();



  // Baterka
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(batBtn.pin, INPUT_PULLUP);

  // emergency button
  pinMode(emeBtn.pin, INPUT_PULLUP);

  // AUX1 tlacitko
  pinMode(AUX1Btn.pin, INPUT_PULLUP);

  // AUX2 tlacitko
  pinMode(AUX2Btn.pin, INPUT_PULLUP);

  lastTime = micros();
  Serial.println("Bluetooth Gamepad aktivní! Spáruj mě s PC.");
}

// ===============================LOOP===============================

void loop() {
  // ---- emergency tlačítko ----
  unsigned long now = millis();

  bool emeCurrentState = digitalRead(emeBtn.pin);

  if (emeBtn.lastState == HIGH && emeCurrentState == LOW) {
    if (now - emeBtn.lastPush > BUTTON_DELAY) {
      emeBtn.lastPush = now;
      isEmergency = !isEmergency;
      p = 0;

      if (isEmergency) {
        Serial.print("Spinkám, jsem vystresovaný!!!!");
      } else {
        Serial.println("Jsem odstresovaný!!!!");
      }
    }
  }
  emeBtn.lastState = emeCurrentState;

  // Kalibrace
  bool clbCurrentState = digitalRead(clbBtn.pin);

  if (clbBtn.lastState == HIGH && clbCurrentState == LOW) {
    if (now - clbBtn.lastPush > BUTTON_DELAY) {
      clbBtn.lastPush = now;
      calibrateController();
    }
  }
  clbBtn.lastState = clbCurrentState;

  // AUX1 tlacitko
  bool AUX1CurrentState = digitalRead(AUX1Btn.pin);

  // začátek stisku
  if (AUX1CurrentState == LOW && AUX1Btn.lastState == HIGH) {

    AUX1Btn.lastPush = now;
    AUX1LongPressDone = false;
    AUX1Animating = true;
  }

  // držení tlačítka
  if (AUX1CurrentState == LOW) {

    float holdProgress = (now - AUX1Btn.lastPush) / 1000.0;

    // animace jen dokud neni hotovy long press
    if (!AUX1LongPressDone) {
      showAux1Progress(!isAUX1, holdProgress);
    }

    // dokončení long press
    if (!AUX1LongPressDone && (now - AUX1Btn.lastPush >= 1000)) {

      isAUX1 = !isAUX1;

      AUX1LongPressDone = true;
      AUX1Animating = false;

      turnOffLeds();
    }
  }

  // puštění tlačítka
  if (AUX1CurrentState == HIGH && AUX1Btn.lastState == LOW) {

    AUX1LongPressDone = false;
    AUX1Animating = false;

    turnOffLeds();
  }

  AUX1Btn.lastState = AUX1CurrentState;

  // AUX2 tlacitko
  bool AUX2CurrentState = digitalRead(AUX2Btn.pin);

  // začátek stisku
  if (AUX2CurrentState == LOW && AUX2Btn.lastState == HIGH) {

    AUX2Btn.lastPush = now;
    AUX2LongPressDone = false;
    AUX2Animating = true;
  }

  // držení tlačítka
  if (AUX2CurrentState == LOW) {

    float holdProgress = (now - AUX2Btn.lastPush) / 1000.0;

    // animace jen dokud neni hotovy long press
    if (!AUX2LongPressDone) {
      showAux2Progress(!isAUX2, holdProgress);
    }

    // dokončení long press
    if (!AUX2LongPressDone && (now - AUX2Btn.lastPush >= 1000)) {

      isAUX2 = !isAUX2;

      AUX2LongPressDone = true;
      AUX2Animating = false;

      turnOffLeds();
    }
  }

  // puštění tlačítka
  if (AUX2CurrentState == HIGH && AUX2Btn.lastState == LOW) {

    AUX2LongPressDone = false;
    AUX2Animating = false;

    turnOffLeds();
  }
  AUX2Btn.lastState = AUX2CurrentState;

  // ---- Akcelerometr a Gyroskop ----
  // casovy vypocet pro akcelerometr a gyroskop
  unsigned long currentTime = micros();
  float dt = (currentTime - lastTime) / 1000000.0;
  lastTime = currentTime;
  if (dt > 0.05) {
    dt = 0.01;
  }

  if (!isEmergency) {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    if (bleGamepad.isConnected()) {

      // 1. Roll, Pitch (mapování tvých vypočítaných angleX/Y)
      float roll_input = constrain((angleX - rollOffset) / MAXTILT, -1.0, 1.0);
      float pitch_input = constrain((angleY - pitchOffset) / MAXTILT, -1.0, 1.0);

      int bleRoll = (roll_input + 1.0) * 16383.5;
      int blePitch = (pitch_input + 1.0) * 16383.5;

      // Yaw
      float yaw_rate = gz / 131.0;
      if (abs(yaw_rate) < 1.0) yaw_rate = 0;
      yaw_angle += yaw_rate * dt;
      int bleYaw = (constrain((yaw_angle - yawOffset) / MAXYAWANGLE, -1.0, 1.0) + 1.0) * 16383.5;

      // 2. Throttle (Plyn)
      int raw = analogRead(POTPIN);
      float joyRaw = raw / 4095.0;
      // Použijeme tvůj joyCenter pro lineární plyn 0-100%
      float mapped_throttle;
      if (joyRaw >= joyCenter) {
        mapped_throttle = (joyRaw - joyCenter) / (1.0 - joyCenter);
      } else {
        mapped_throttle = 0;  // Pokud je páčka pod středem/v klidu, dej nulu
      }
      int bleThrottle = constrain(mapped_throttle * 32737, 0, 32737);

      // 3. Odeslání do PC (Bluetooth)
      bleGamepad.setAxes(bleRoll, blePitch, bleYaw, 0, 0, bleThrottle, DPAD_CENTERED);

      // 4. AUX tlačítka
      if (isAUX1) bleGamepad.press(BUTTON_1);
      else bleGamepad.release(BUTTON_1);
      if (isAUX2) bleGamepad.press(BUTTON_2);
      else bleGamepad.release(BUTTON_2);
    }

    // Původní výpisy a ESP-NOW můžeš smazat nebo zakomentovat:
    // printout_data(...);
    // ESPNOW_send(...);

  } else {
    // V nouzovém režimu pošli nuly
    if (bleGamepad.isConnected()) {
      bleGamepad.setAxes(16384, 16384, 16384, 0, 0, 0, DPAD_CENTERED);
    }

    /*float accAngleX = atan2(ay, az) * RADTODEG;
    float accAngleY = atan2(-ax, sqrt(ay * ay + az * az)) * RADTODEG;

    float gx_dps = gx / 131.0;
    float gy_dps = gy / 131.0;

    float alpha = 0.98;

    angleX = alpha * (angleX + gx_dps * dt) + (1 - alpha) * accAngleX;
    angleY = alpha * (angleY + gy_dps * dt) + (1 - alpha) * accAngleY;

    float roll_input = (angleX - rollOffset) / MAXTILT;
    float pitch_input = (angleY - pitchOffset) / MAXTILT;
    float yaw_rate = gz / 131.0;

    if (abs(yaw_rate) < 1.0) {
      yaw_rate = 0;
    }

    yaw_angle += yaw_rate * dt;
    yaw_angle *= 0.995;

    float yaw_input = (yaw_angle - yawOffset) / MAXYAWANGLE;

    yaw_input = 0.9 * yaw_input_prev + 0.1 * yaw_input;
    yaw_input_prev = yaw_input;

    if (abs(roll_input) < 0.05) {
      roll_input = 0;
    }

    if (abs(pitch_input) < 0.05) {
      pitch_input = 0;
    }

    roll_input = constrain(roll_input, -1.0, 1.0);
    pitch_input = constrain(pitch_input, -1.0, 1.0);
    yaw_input = constrain(yaw_input, -1.0, 1.0);

    // ---- JOYSTICK THROTTLE (MAPOVÁNÍ -1/1 -> 0/1) ----
    int raw = analogRead(POTPIN);

    // 1. Normalizace surového čtení na rozsah 0.0 až 1.0
    float joyRaw = raw / 4095.0;

    // 2. Lehký filtr pro vyhlazení šumu potenciometru
    joy = 0.85 * joy + 0.15 * joyRaw;

    // 3. Převod na tvůj rozsah -1.0 až 1.0 (vycházíme z tvé kalibrace středu)
    // joyCenter je hodnota, kterou jsi změřil v setupu (cca 0.32)
    float mapped_input;
    if (joy >= joyCenter) {
      mapped_input = (joy - joyCenter) / (1.0 - joyCenter);  // Horní polovina (0 až 1)
    } else {
      mapped_input = (joy - joyCenter) / joyCenter;  // Dolní polovina (0 až -1)
    }

    // 4. MAPOVÁNÍ PRO INAV ALTHOLD (Cíl: 0.0 až 1.0, střed 0.5)
    const float deadzone = 0.005;  // Mrtvá zóna kolem fyzického středu
    float final_throttle;

    if (abs(mapped_input) < deadzone) {
      final_throttle = 0.1;  // PŘESNÝ STŘED = 1500uS v INAV (Držení výšky)
    } else {
      // Matematický převod: (-1.0 až 1.0) -> (0.0 až 1.0)
      final_throttle = ((mapped_input + 1.0) / 2.0) * 0.2;
    }

    // Pojistka rozsahu
    final_throttle = constrain(final_throttle, 0.0, 0.2);


    // Vypis
    printout_data(roll_input, pitch_input, yaw_input, final_throttle, isAUX1, isAUX2);

    // ESP-NOW
    //ESPNOW_send(roll_input, pitch_input, yaw_input, final_throttle, isAUX1, isAUX2);

  } else {
    // Vypis
    printout_data(0, 0, 0, 0.5, isAUX1, isAUX2);

    // ESP-NOW
    //ESPNOW_send(0, 0, 0, 0.5, isAUX1, isAUX2);
  }*/

    // Baterka
    now = millis();
    bool batCurrentState = digitalRead(batBtn.pin);

    if (batBtn.lastState == HIGH && batCurrentState == LOW) {
      if (now - batBtn.lastPush > BUTTON_DELAY) {

        batBtn.lastPush = now;

        // načti baterku jen jednou
        batteryLedValue = getBatteryLedCount();

        // zobrazuj 2 sekundy
        batBtn.offTime = now + 2000;
      }
    }
    batBtn.lastState = batCurrentState;
    // ================= LED MANAGEMENT =================

    if (!AUX1Animating && !AUX2Animating) {

      // ---- Emergency blikání ----
      if (isEmergency) {

        if (now - lastBlinkTime >= blinkInterval) {

          lastBlinkTime = now;
          blinkState = !blinkState;

          for (int i = 0; i < ledCount; i++) {
            digitalWrite(ledPins[i], blinkState);
          }
        }
      }

      // ---- Indikace baterky ----
      else if (batBtn.offTime > 0 && now < batBtn.offTime) {
        showBatteryLeds(batteryLedValue);
      }

      // ---- Nic nesvítí ----
      else {

        turnOffLeds();
        batBtn.offTime = 0;
      }
    }
  }
    delay(5);
  }