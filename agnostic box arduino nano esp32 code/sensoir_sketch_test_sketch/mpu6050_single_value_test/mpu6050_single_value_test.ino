/*
  Arduino Nano ESP32 - MPU6050 Single Value Test

  This prints only the final value from the full MPU test sketch: compY.
  Keep the sensor still and horizontal during startup calibration.
*/

#include <Wire.h>

const int MPU = 0x68;

float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float gyroOffsetX, gyroOffsetY, gyroOffsetZ;
float angleYZero;
float compAngleY;
unsigned long previousTime;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(100);

  calibrateMpu();
  previousTime = millis();
}

void loop() {
  readMpu();

  unsigned long currentTime = millis();
  float elapsedTime = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  float angleY = atan2(-accX, accZ) * 180.0 / PI;
  float frontY = angleY - angleYZero;

  compAngleY = 0.96 * (compAngleY + gyroY * elapsedTime) + 0.04 * frontY;

  Serial.println(compAngleY, 1);

  delay(100);
}

void readMpu() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();

  Wire.read();
  Wire.read();

  Wire.read();
  Wire.read();
  int16_t rawGyroY = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read();

  accX = rawAccX / 16384.0;
  accZ = rawAccZ / 16384.0;

  gyroY = rawGyroY / 131.0 - gyroOffsetY;
}

void calibrateMpu() {
  const int samples = 300;
  float gyroSumY = 0;
  float angleSumY = 0;

  for (int i = 0; i < samples; i++) {
    readMpu();

    gyroSumY += gyroY;
    angleSumY += atan2(-accX, accZ) * 180.0 / PI;

    delay(5);
  }

  gyroOffsetY = gyroSumY / samples;
  angleYZero = angleSumY / samples;
  compAngleY = 0;
}
