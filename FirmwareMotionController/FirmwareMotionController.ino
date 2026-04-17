#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#define RADTODEG 57.2958

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

void setup() {
    Wire.begin(21, 22); // Inicializace I2C na pinech ESP32
    Serial.begin(115200);

    Serial.println("Inicializace I2C zarizeni...");
    accelgyro.initialize();

    // Overeni spojeni
    Serial.println("Testovani pripojeni...");
    Serial.println(accelgyro.testConnection() ? "MPU6500 pripojen uspesne" : "MPU6500 pripojeni selhalo");
}

void loop() {
    // Cteni syrovych dat
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Vypis do monitoru
    /*Serial.print("A:");
    Serial.print(ax); Serial.print("\t");
    Serial.print(ay); Serial.print("\t");
    Serial.print(az); Serial.print("\t | ");
    Serial.print("G:");
    Serial.print(gx); Serial.print("\t");
    Serial.print(gy); Serial.print("\t");
    Serial.println(gz);*/

    float accAngleX = atan2(ay, az) * RADTODEG;
    float accAngleY = atan2(-ax, sqrt(ay*ay + az*az)) * RADTODEG;
    Serial.print("Osa X:\t");
    Serial.println(accAngleX);
    Serial.print("Osa Y:\t"); 
    Serial.println(accAngleY);
    Serial.println("---------------");

    delay(100);
}