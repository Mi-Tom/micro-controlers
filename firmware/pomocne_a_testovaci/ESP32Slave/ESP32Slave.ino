#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  float roll;
  float pitch;
  float yaw;
  float throttle;
} struct_message;


struct_message message;

void data_receive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memcpy(&message, data, sizeof(message));

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
