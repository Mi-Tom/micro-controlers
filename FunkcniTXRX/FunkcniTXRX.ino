#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// --- ESP-NOW ---
typedef struct struct_message {
  float roll;
  float pitch;
  float yaw;
  float throttle;
} struct_message;

struct_message message;

// --- ELRS TX ---
HardwareSerial ELRSSerial(2);
uint16_t kanaly[16];

void posliCrsfKanaly(uint16_t *channels) {
  uint8_t packet[26];
  packet[0] = 0xC8; // SYNC - puvodne bylo na 0xEE
  packet[1] = 24;   // Délka
  packet[2] = 0x16; // Typ: RC Channels

  // Mapa pro převod 1000-2000 na CRSF rozsah (cca 172-1811)
  uint16_t out[16];
  for (int i = 0; i < 16; i++) {
    out[i] = (channels[i] - 1000) * 1024 / 625 + 191;
  }

  // Bitové balení
  packet[3]  = (uint8_t)(out[0] & 0x07FF);
  packet[4]  = (uint8_t)((out[0] >> 8) | ((out[1] & 0x07FF) << 3));
  packet[5]  = (uint8_t)((out[1] >> 5) | ((out[2] & 0x07FF) << 6));
  packet[6]  = (uint8_t)(out[2] >> 2);
  packet[7]  = (uint8_t)((out[2] >> 10) | ((out[3] & 0x07FF) << 1));
  packet[8]  = (uint8_t)((out[3] >> 7) | ((out[4] & 0x07FF) << 4));
  packet[9]  = (uint8_t)((out[4] >> 4) | ((out[5] & 0x07FF) << 7));
  packet[10] = (uint8_t)(out[5] >> 1);
  packet[11] = (uint8_t)((out[5] >> 9) | ((out[6] & 0x07FF) << 2));
  packet[12] = (uint8_t)((out[6] >> 6) | ((out[7] & 0x07FF) << 5));
  packet[13] = (uint8_t)(out[7] >> 3);
  packet[14] = (uint8_t)(out[8] & 0x07FF);
  packet[15] = (uint8_t)((out[8] >> 8) | ((out[9] & 0x07FF) << 3));
  packet[16] = (uint8_t)((out[9] >> 5) | ((out[10] & 0x07FF) << 6));
  packet[17] = (uint8_t)(out[10] >> 2);
  packet[18] = (uint8_t)((out[10] >> 10) | ((out[11] & 0x07FF) << 1));
  packet[19] = (uint8_t)((out[11] >> 7) | ((out[12] & 0x07FF) << 4));
  packet[20] = (uint8_t)((out[12] >> 4) | ((out[13] & 0x07FF) << 7));
  packet[21] = (uint8_t)(out[13] >> 1);
  packet[22] = (uint8_t)((out[13] >> 9) | ((out[14] & 0x07FF) << 2));
  packet[23] = (uint8_t)((out[14] >> 6) | ((out[15] & 0x07FF) << 5));
  packet[24] = (uint8_t)(out[15] >> 3);

  packet[25] = crsf_crc8(&packet[2], 23);
  ELRSSerial.write(packet, 26);
}

// --- FUNKCE PRO VÝPOČET CRC A BALENÍ ---
uint8_t crsf_crc8(uint8_t *data, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) crc = (crc << 1) ^ 0xD5;
      else crc <<= 1;
    }
  }
  return crc;
}

void data_receive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&message, data, sizeof(message));
}

void setup() {
  Serial.begin(115200);

  // --- ELRS TX ---
  ELRSSerial.begin(420000, SERIAL_8N1, 16, 17); 
  for(int i=0; i<16; i++) kanaly[i] = 1500;
  kanaly[4] = 1000;

  // --- ESP-NOW ---
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(data_receive);
  
  Serial.println("Vysílač ESP-NOW -> ELRS připraven!");
}

void loop() {
  float t = millis() / 1000.0;

  // --- TADY SI NASTAVUJ HODNOTY (Jednoduché a přehledné) ---

  Serial.println("Data received:");
  Serial.print("Roll: ");
  Serial.println(message.roll);

  Serial.print("Pitch: ");
  Serial.println(message.pitch);

  Serial.print("Yaw: ");
  Serial.println(message.yaw);

  Serial.print("Throttle: ");
  Serial.println(message.throttle);
  Serial.println("----------------");

  kanaly[0] = 1500 + message.roll * 400;        // Roll
  kanaly[1] = 1500 + message.pitch * 400;       // Pitch
  kanaly[2] = 1000 + message.throttle * 900;    // Throttle
  kanaly[3] = 1500 + message.yaw * 400;         // Yaw
  kanaly[4] = 2000;                             // AUX1 (Arm)
  kanaly[5] = 2000;                             // AUX2 (Angle mode)
  // -------------------------------------------------------

  // Zavoláme "černou skříňku", která data zabalí a pošle
  posliCrsfKanaly(kanaly);

  delay(10); // 100 Hz frekvence odesílání
}
