#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
    char character[100];
    int integer;
    float floating_value;
    bool bool_value;
} struct_message;


struct_message message;

void data_receive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&message, data, sizeof(message));

  Serial.println("Data received:");
  Serial.print("Text: ");
  Serial.println(message.character);

  Serial.print("Int: ");
  Serial.println(message.integer);

  Serial.print("Float: ");
  Serial.println(message.floating_value);

  Serial.print("Bool: ");
  Serial.println(message.bool_value);
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(data_receive);
}
 
void loop() {

}
