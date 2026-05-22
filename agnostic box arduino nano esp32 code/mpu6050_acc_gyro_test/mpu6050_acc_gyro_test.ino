/*
  Arduino Nano ESP32 - MPU6050 Accelerometer/Gyroscope Test

  Upload this sketch, open Serial Monitor at 115200 baud, and move the sensor.
  Keep the sensor still and horizontal during startup calibration.
*/

#include <Wire.h>

const int MPU = 0x68;

float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float gyroOffsetX, gyroOffsetY, gyroOffsetZ;
float angleX, angleY;
float angleXZero, angleYZero;
float compAngleX, compAngleY;
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

  Serial.println("MPU6050 test ready");
  Serial.println("rawAccX rawAccY rawAccZ rawGyroX rawGyroY rawGyroZ angleX angleY frontX frontY compX compY");
}

void loop() {
  readMpu();

  unsigned long currentTime = millis();
  float elapsedTime = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  angleX = atan2(accY, accZ) * 180.0 / PI;
  angleY = atan2(-accX, accZ) * 180.0 / PI;

  float frontX = angleX - angleXZero;
  float frontY = angleY - angleYZero;

  compAngleX = 0.96 * (compAngleX + gyroX * elapsedTime) + 0.04 * frontX;
  compAngleY = 0.96 * (compAngleY + gyroY * elapsedTime) + 0.04 * frontY;

  Serial.print(accX, 3);
  Serial.print(" ");
  Serial.print(accY, 3);
  Serial.print(" ");
  Serial.print(accZ, 3);
  Serial.print(" ");
  Serial.print(gyroX, 2);
  Serial.print(" ");
  Serial.print(gyroY, 2);
  Serial.print(" ");
  Serial.print(gyroZ, 2);
  Serial.print(" ");
  Serial.print(angleX, 1);
  Serial.print(" ");
  Serial.print(angleY, 1);
  Serial.print(" ");
  Serial.print(frontX, 1);
  Serial.print(" ");
  Serial.print(frontY, 1);
  Serial.print(" ");
  Serial.print(compAngleX, 1);
  Serial.print(" ");
  Serial.println(compAngleY, 1);

  delay(100);
}

void readMpu() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);

  int16_t rawAccX = Wire.read() << 8 | Wire.read();
  int16_t rawAccY = Wire.read() << 8 | Wire.read();
  int16_t rawAccZ = Wire.read() << 8 | Wire.read();

  Wire.read();
  Wire.read();

  int16_t rawGyroX = Wire.read() << 8 | Wire.read();
  int16_t rawGyroY = Wire.read() << 8 | Wire.read();
  int16_t rawGyroZ = Wire.read() << 8 | Wire.read();

  accX = rawAccX / 16384.0;
  accY = rawAccY / 16384.0;
  accZ = rawAccZ / 16384.0;

  gyroX = rawGyroX / 131.0 - gyroOffsetX;
  gyroY = rawGyroY / 131.0 - gyroOffsetY;
  gyroZ = rawGyroZ / 131.0 - gyroOffsetZ;
}

void calibrateMpu() {
  Serial.println("Calibrating MPU6050. Keep it still and horizontal.");

  const int samples = 300;
  float gyroSumX = 0;
  float gyroSumY = 0;
  float gyroSumZ = 0;
  float angleSumX = 0;
  float angleSumY = 0;

  for (int i = 0; i < samples; i++) {
    readMpu();

    gyroSumX += gyroX;
    gyroSumY += gyroY;
    gyroSumZ += gyroZ;
    angleSumX += atan2(accY, accZ) * 180.0 / PI;
    angleSumY += atan2(-accX, accZ) * 180.0 / PI;

    delay(5);
  }

  gyroOffsetX = gyroSumX / samples;
  gyroOffsetY = gyroSumY / samples;
  gyroOffsetZ = gyroSumZ / samples;
  angleXZero = angleSumX / samples;
  angleYZero = angleSumY / samples;
  compAngleX = 0;
  compAngleY = 0;

  Serial.println("Calibration complete");
}
