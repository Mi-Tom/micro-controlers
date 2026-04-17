#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#define RADTODEG 57.2958

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long lastTime;

float angleX = 0;
float angleY = 0;

void setup() {
    Wire.begin(21, 22); // Inicializace I2C na pinech ESP32
    Serial.begin(115200);

    Serial.println("Inicializace I2C zarizeni...");
    accelgyro.initialize();

    // Overeni spojeni
    Serial.println("Testovani pripojeni...");
    Serial.println(accelgyro.testConnection() ? "MPU6500 pripojen uspesne" : "MPU6500 pripojeni selhalo");

    lastTime = micros();
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
    unsigned long currentTime = micros();
    float dt = (currentTime - lastTime) / 1000000.0;
    lastTime = currentTime;

    float accAngleX = atan2(ay, az) * RADTODEG;
    float accAngleY = atan2(-ax, sqrt(ay*ay + az*az)) * RADTODEG;

    float gx_dps = gx / 131.0;
    float gy_dps = gy / 131.0;

    float alpha = 0.98;

    angleX = alpha * (angleX + gx_dps * dt) + (1 - alpha) * accAngleX;
    angleY = alpha * (angleY + gy_dps * dt) + (1 - alpha) * accAngleY;

    Serial.print("Osa X:\t");
    Serial.println(angleX);
    Serial.print("Osa Y:\t"); 
    Serial.println(angleY);
    Serial.println("---------------");

    delay(5);
}