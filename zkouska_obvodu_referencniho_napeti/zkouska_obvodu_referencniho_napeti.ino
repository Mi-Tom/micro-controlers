const int pinBat = 32;      // Pin pro dělič baterie
const int pinRef = 35;      // Pin pro LM285 (2.5V)
const float R1 = 100000.0;  //100K
const float R2 = 100000.0;
const float ratio = (R1 + R2) / R2;
int led_pins[] = { 4, 16, 2, 0, 15 };
int led_count = 5;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < led_count; i++) {
    pinMode(led_pins[i], OUTPUT);
  }
}

void loop() {
  // Průměrování pro stabilitu
  int ref = 0;
  int bat = 0;

  ref = analogRead(pinRef);
  bat = analogRead(pinBat);

  float rawBat = (bat * 2.5) / ref;
  float vBattery = rawBat * ratio;

  float percent = (vBattery - 3.2) / (4.2 - 3.2) * 100.0;
  percent = constrain(percent, 0, 100);

  int led_on = map(vBattery * 100, 320, 400, 1, led_count);
  led_on = constrain(led_on, 1, led_count);

  for (int i = 0; i < led_on; i++) {
    digitalWrite(led_pins[i], HIGH);
  }

  Serial.print("Napeti: ");
  Serial.print(vBattery);
  Serial.println(" V");
  Serial.print("Nabiti: ");
  Serial.print(percent);
  Serial.println(" %");
  Serial.print("Ledky: ");
  Serial.print(led_on);
  Serial.println(" LED");

  delay(100);

  for (int i = 0; i < led_count; i++) {
    digitalWrite(led_pins[i], LOW);
  }
}