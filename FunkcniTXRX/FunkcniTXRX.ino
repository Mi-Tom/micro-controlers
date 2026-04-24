#include <Arduino.h>

HardwareSerial ELRSSerial(2); // UART2 (TX na pinu 17)

void setup() {
  Serial.begin(115200);
  // ELRS standardní rychlost pro ESP32 -> Modul
  ELRSSerial.begin(400000, SERIAL_8N1, 16, 17); 
  Serial.println("ESP32 ELRS Transmitter Emulator Started");
}

// Funkce pro výpočet CRSF CRC8 (poly 0xD5)
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

void loop() {
  float t = millis() / 1000.0;

  // 1. Definuj kanály (1000 - 2000)
  uint16_t channels[16];
  for(int i=0; i<16; i++) channels[i] = 1500; // default střed

  channels[0] = 1500 + sin(t) * 500;      // Roll
  channels[2] = 1000 + (sin(t*0.5)+1)*500; // Throttle
  
  // 2. Sestavení paketu (26 bajtů celkem)
  uint8_t packet[26];
  packet[0] = 0xEE; // Adresa: SYNC_BYTE (Radio Transmitter -> Module)
  packet[1] = 24;   // Délka (zbytek paketu)
  packet[2] = 0x16; // Typ: RC_CHANNELS_PACKED

  // 3. Bitové balení 11-bitových kanálů (CRSF formát)
  packet[3]  = (uint8_t)(channels[0] & 0x07FF);
  packet[4]  = (uint8_t)((channels[0] >> 8) | ((channels[1] & 0x07FF) << 3));
  packet[5]  = (uint8_t)((channels[1] >> 5) | ((channels[2] & 0x07FF) << 6));
  packet[6]  = (uint8_t)(channels[2] >> 2);
  packet[7]  = (uint8_t)((channels[2] >> 10) | ((channels[3] & 0x07FF) << 1));
  packet[8]  = (uint8_t)((channels[3] >> 7) | ((channels[4] & 0x07FF) << 4));
  packet[9]  = (uint8_t)((channels[4] >> 4) | ((channels[5] & 0x07FF) << 7));
  packet[10] = (uint8_t)(channels[5] >> 1);
  packet[11] = (uint8_t)((channels[5] >> 9) | ((channels[6] & 0x07FF) << 2));
  packet[12] = (uint8_t)((channels[6] >> 6) | ((channels[7] & 0x07FF) << 5));
  packet[13] = (uint8_t)(channels[7] >> 3);
  packet[14] = (uint8_t)(channels[8] & 0x07FF);
  packet[15] = (uint8_t)((channels[8] >> 8) | ((channels[9] & 0x07FF) << 3));
  packet[16] = (uint8_t)((channels[9] >> 5) | ((channels[10] & 0x07FF) << 6));
  packet[17] = (uint8_t)(channels[10] >> 2);
  packet[18] = (uint8_t)((channels[10] >> 10) | ((channels[11] & 0x07FF) << 1));
  packet[19] = (uint8_t)((channels[11] >> 7) | ((channels[12] & 0x07FF) << 4));
  packet[20] = (uint8_t)((channels[12] >> 4) | ((channels[13] & 0x07FF) << 7));
  packet[21] = (uint8_t)(channels[13] >> 1);
  packet[22] = (uint8_t)((channels[13] >> 9) | ((channels[14] & 0x07FF) << 2));
  packet[23] = (uint8_t)((channels[14] >> 6) | ((channels[15] & 0x07FF) << 5));
  packet[24] = (uint8_t)(channels[15] >> 3);

  // 4. Výpočet CRC (počítá se z Typu a Dat, tedy od indexu 2 do 24)
  packet[25] = crsf_crc8(&packet[2], 23);

  // 5. Odeslání do ELRS modulu
  ELRSSerial.write(packet, 26);

  delay(20); // 50 Hz je pro testování víc než dost
}