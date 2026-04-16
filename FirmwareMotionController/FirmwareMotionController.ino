#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu(0x68);

void setup() {
  Serial.begin(115200);
  Serial.println("Baf, zatim funguju!");
  Wire.begin(21, 22); // SDA=21, SCL=22
  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
    while (1) delay(1000);
  }
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);

  // Convert raw to g and deg/s if you want:
  float accelX = ax / 16384.0; // for ±2g
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;

  float gyroX = gx / 131.0; // for ±250 deg/s
  float gyroY = gy / 131.0;
  float gyroZ = gz / 131.0;

  Serial.print("A: "); Serial.print(accelX); Serial.print(", ");
  Serial.print(accelY); Serial.print(", "); Serial.print(accelZ);
  Serial.print(" | G: "); Serial.print(gyroX); Serial.print(", ");
  Serial.print(gyroY); Serial.print(", "); Serial.println(gyroZ);

  delay(200);
}
