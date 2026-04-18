#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <esp_now.h>
#include <WiFi.h>

#define RADTODEG 57.2958
#define MAXTILT 35.0
#define MAXYAWANGLE 60.0
#define BUTTON_DELAY 50

#define POTPIN 33   // Pin pro potenciomentr
#define PIN_BAT 32  // Pin pro dělič baterie
#define PIN_REF 35  // Pin pro LM285 (2.5V)

//---- Gyroskop ----
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long lastTime;

float angleX = 0;
float angleY = 0;
float yaw_input_prev = 0.0;
static float yaw_angle = 0;
int p = 0;

//---- Button Baterka ----
const float R1 = 100000.0;  //100K
const float R2 = 100000.0;
const float VOLT_RATIO = (R1 + R2) / R2;

int ledPins[] = { 4, 16, 2, 0, 15 };
int ledCount = 5;

struct Button {
  int pin;
  unsigned long lastPush;
  bool lastState;
  unsigned long offTime;
};

Button batBtn = { 27, 0, HIGH, 0 };
Button emeBtn = { 26, 0, HIGH, 0 };

//---- ESP NOW komunikace ----
uint8_t broadcastAddress[] = { 0x08, 0xb6, 0x1f, 0xb8, 0x4c, 0x50 };  //MAC adresa

typedef struct struct_message {
  float roll;
  float pitch;
  float yaw;
  float throttle;
} struct_message;

struct_message message;

// =====================TESTOVANI_NAPETI_BATERIE=====================

void checkBattery() {
  int refRaw = analogRead(PIN_REF);
  int batRaw = analogRead(PIN_BAT);

  float voltage = (float(batRaw) / refRaw) * 2.5 * VOLT_RATIO;
  int percent = constrain((voltage - 3.2) * 100, 0, 100);

  int ledsToLight = map(percent, 0, 100, 0, ledCount);
  ledsToLight = constrain(ledsToLight, 1, ledCount);

  for (int i = 0; i < ledsToLight; i++) {
    digitalWrite(ledPins[i], HIGH);
  }

  Serial.print("Napeti: ");
  Serial.print(voltage);
  Serial.println(" V");
  Serial.print("Nabiti: ");
  Serial.print(percent);
  Serial.println(" %");
  Serial.print("Ledky: ");
  Serial.print(ledsToLight);
  Serial.println(" LED");
}


void turnOffLeds() {
  for (int i = 0; i < ledCount; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

// ==============================ESP NOW=============================

void ESPNOW_send(float roll_input, float pitch_input, float yaw_input, float throttle_input) {
  message.roll = roll_input;
  message.pitch = pitch_input;
  message.yaw = yaw_input;
  message.throttle = throttle_input;

  esp_err_t outcome = esp_now_send(broadcastAddress, (uint8_t *)&message, sizeof(message));

  if (outcome == ESP_OK) {
    Serial.println("Message sent successfully!");
  } else {
    Serial.println("Error sending the message");
  }
}

void data_sent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// ==============================SETUP===============================

void setup() {
  // Akcelerometr a Gyroskop
  Wire.begin(21, 22);  // Inicializace I2C na pinech ESP32
  Serial.begin(115200);

  Serial.println("Inicializace I2C zarizeni...");
  accelgyro.initialize();

  Serial.println("Testovani pripojeni...");
  Serial.println(accelgyro.testConnection() ? "MPU6500 pripojen uspesne" : "MPU6500 pripojeni selhalo");

  lastTime = micros();

  // Potenciometr
  analogReadResolution(12);        // 0–4095
  analogSetAttenuation(ADC_11db);  // rozsah cca 0–3.3V

  // Baterka
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(batBtn.pin, INPUT_PULLUP);

  // emergency button
  pinMode(emeBtn.pin, INPUT_PULLUP);

  // ESP NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(data_sent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

// ===============================LOOP===============================

void loop() {
  // Akcelerometr a Gyroskop
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  unsigned long currentTime = micros();
  float dt = (currentTime - lastTime) / 1000000.0;

  if (dt > 0.05) {
    dt = 0.01;
  }

  lastTime = currentTime;

  float accAngleX = atan2(ay, az) * RADTODEG;
  float accAngleY = atan2(-ax, sqrt(ay * ay + az * az)) * RADTODEG;

  float gx_dps = gx / 131.0;
  float gy_dps = gy / 131.0;

  float alpha = 0.98;

  angleX = alpha * (angleX + gx_dps * dt) + (1 - alpha) * accAngleX;
  angleY = alpha * (angleY + gy_dps * dt) + (1 - alpha) * accAngleY;

  float roll_input = angleX / MAXTILT;
  float pitch_input = angleY / MAXTILT;
  float yaw_rate = gz / 131.0;

  if (abs(yaw_rate) < 1.0) {
    yaw_rate = 0;
  }

  yaw_angle += yaw_rate * dt;
  yaw_angle *= 0.995;

  float yaw_input = yaw_angle / MAXYAWANGLE;

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

  // Pontenciometr
  int pot_value = analogRead(POTPIN);
  float throttle_input = pot_value / 4095.0;

  // Vypis
  p++;
  if (p == 10) {
    Serial.print("Roll:\t");
    Serial.println(roll_input);
    Serial.print("Pitch:\t");
    Serial.println(pitch_input);
    Serial.print("Yaw:\t");
    Serial.println(yaw_input);
    Serial.print("Raw Pontentiometer:\t");
    Serial.println(pot_value);
    Serial.print("Throttle:\t");
    Serial.println(throttle_input);
    Serial.println("---------------");
    p = 0;
  }

  // Baterka
  unsigned long now = millis();
  bool currentState = digitalRead(batBtn.pin);

  if (batBtn.lastState == HIGH && currentState == LOW) {
    if (now - batBtn.lastPush > BUTTON_DELAY) {
      batBtn.lastPush = now;
      checkBattery();
      batBtn.offTime = now + 2000;  // Po jak dlouhou dobu bude indikace baterie svítit
    }
  }
  batBtn.lastState = currentState;

  // Automatické vypnutí LED vázané na tlačítko LED
  if (batBtn.offTime > 0 && now >= batBtn.offTime) {
    turnOffLeds();
    batBtn.offTime = 0;
  }

  // emergency tlačítko
  now = millis();
  currentState = digitalRead(emeBtn.pin);


  if (emeBtn.lastState == HIGH && currentState == LOW) {
    if (now - emeBtn.lastPush > BUTTON_DELAY) {
      emeBtn.lastPush = now;
      Serial.print("Spinkám, jsem vystresovaný!!!!");
      int emergency_time = 10000; // Doba nouzového čekání po zmáčknutí emergency tlačítka
      unsigned long enter_time = millis();
      bool leave = false;
      while(leave)
      {
        ESPNOW_send(0, 0, 0, 0);
        now = millis();
        if(enter_time - now >= emergency_time)
        {
          leave = true;
        }
      } 
    }
  }

  // ESP NOW
  ESPNOW_send(roll_input, pitch_input, yaw_input, throttle_input);

  delay(5);
}