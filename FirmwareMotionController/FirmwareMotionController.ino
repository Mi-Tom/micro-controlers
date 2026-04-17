#include <Wire.h>

#define MPU_ADDR 0x68

int16_t ax, ay, az, gx, gy, gz;

void setup() {
  Serial.begin(115200);
  Serial.println("Start...");

  Wire.begin(21, 22); // ESP32

  // Probuzení MPU6500 (registr PWR_MGMT_1)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    Serial.println("Chyba komunikace!");
    while (1);
  }

  Serial.println("MPU6500 pripraven");
}

void loop() {
  // Začít čtení od ACCEL_XOUT_H (0x3B)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  if (Wire.available() == 14) {
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();

    Wire.read(); Wire.read(); // teplota (ignorujeme)

    gx = Wire.read() << 8 | Wire.read();
    gy = Wire.read() << 8 | Wire.read();
    gz = Wire.read() << 8 | Wire.read();

    float accelX = ax / 16384.0;
    float accelY = ay / 16384.0;
    float accelZ = az / 16384.0;

    float gyroX = gx / 131.0;
    float gyroY = gy / 131.0;
    float gyroZ = gz / 131.0;

    Serial.print("ACC: ");
    Serial.print(accelX); Serial.print(", ");
    Serial.print(accelY); Serial.print(", ");
    Serial.print(accelZ);

    Serial.print(" | GYRO: ");
    Serial.print(gyroX); Serial.print(", ");
    Serial.print(gyroY); Serial.print(", ");
    Serial.print(gyroZ);

    Serial.println();
  } else {
    Serial.println("Chyba cteni dat");
  }

  delay(200);
}