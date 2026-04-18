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
  Serial.print("Data received: ");
  
  for (int i = 0; i < len; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println();

  // Pokud chceš MAC adresu odesílatele:
  if (info != NULL) {
    const uint8_t *mac = info->src_addr;
    Serial.printf("From: ec:62:60:94:da:74\n",
                  mac[0], mac[1], mac[2],
                  mac[3], mac[4], mac[5]);
  }
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
