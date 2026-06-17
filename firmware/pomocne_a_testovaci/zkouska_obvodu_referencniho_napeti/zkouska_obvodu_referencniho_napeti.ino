#define PIN_BAT 32      // Pin pro dělič baterie
#define PIN_REF 35      // Pin pro LM285 (2.5V)
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

#define BUTTON_DELAY 50

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


// ==============================SETUP===============================
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(batBtn.pin, INPUT_PULLUP);
}

// ===============================LOOP===============================
void loop() {
  unsigned long now = millis();
  bool currentState = digitalRead(batBtn.pin);

  if (batBtn.lastState == HIGH && currentState == LOW) {
    if (now - batBtn.lastPush > BUTTON_DELAY) {
      batBtn.lastPush = now;
      checkBattery();
      batBtn.offTime = now + 2000; // Po jak dlouhou dobu bude indikace baterie svítit
    }
  }
  batBtn.lastState = currentState;

  // Automatické vypnutí LED vázané na toto tlačítko
  if (batBtn.offTime > 0 && now >= batBtn.offTime) {
    turnOffLeds();
    batBtn.offTime = 0;
  }
}


