#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("OK");
  } else {
    Serial.println("FAIL");
  }
}

void loop() {}