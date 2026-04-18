#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#define RADTODEG 57.2958
#define MAXTILT 35.0
#define MAXYAWANGLE 60.0

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long lastTime;

float angleX = 0;
float angleY = 0;
float yaw_input_prev = 0.0;
static float yaw_angle = 0;

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

    if(dt > 0.05)
    {
        dt = 0.01;
    }

    lastTime = currentTime;

    float accAngleX = atan2(ay, az) * RADTODEG;
    float accAngleY = atan2(-ax, sqrt(ay*ay + az*az)) * RADTODEG;

    float gx_dps = gx / 131.0;
    float gy_dps = gy / 131.0;

    float alpha = 0.98;

    angleX = alpha * (angleX + gx_dps * dt) + (1 - alpha) * accAngleX;
    angleY = alpha * (angleY + gy_dps * dt) + (1 - alpha) * accAngleY;

    float roll_input = angleX / MAXTILT;
    float pitch_input = angleY / MAXTILT;  
    float yaw_rate = gz / 131.0;

    if(abs(yaw_rate) < 1.0)
    {
        yaw_rate = 0;
    }

    yaw_angle += yaw_rate * dt;
    yaw_angle *= 0.995;

    float yaw_input = yaw_angle / MAXYAWANGLE;

    yaw_input = 0.9 * yaw_input_prev + 0.1 * yaw_input;
    yaw_input_prev = yaw_input;

    if(abs(roll_input) < 0.05)
    {
        roll_input = 0;
    }

    if(abs(pitch_input) < 0.05)
    {
        pitch_input = 0;
    }

    roll_input = constrain(roll_input, -1.0, 1.0);
    pitch_input = constrain(pitch_input, -1.0, 1.0);
    yaw_input = constrain(yaw_input, -1.0, 1.0);

    Serial.print("Roll:\t");
    Serial.println(roll_input);
    Serial.print("Pitch:\t"); 
    Serial.println(pitch_input);
    Serial.print("Yaw:\t");
    Serial.println(yaw_input);
    Serial.println("---------------");

    delay(5);
}