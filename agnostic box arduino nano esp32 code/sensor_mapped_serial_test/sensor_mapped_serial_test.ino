/*
  Arduino Nano ESP32
  MPU6050 + 3 Pots + 3 Buttons + 3 LEDs

  Serial array:
  [pot0Mapped, pot1Mapped, pot2Mapped, button0, button1, button2, gyroMapped]

  Pot values are mapped from 12-bit ADC range 0-4095 to 1-127.
  Gyro angle value maps 0-90 degrees to 0-127.
  Gyro values below 0 clamp to 0.
  Gyro values above 127 clamp to 120.

  Connections:
  A1 -> pot 0
  A2 -> pot 1
  A3 -> pot 2

  D10 -> button 0
  D11 -> button 1
  D12 -> button 2

  D6 -> LED 0
  D5 -> LED 1
  D4 -> LED 2

  A4 -> SDA
  A5 -> SCL
*/

#include <Wire.h>

const int MPU = 0x68;

// MPU values
float AccX, AccZ;
float GyroY;
float angleYZero;
float compAngleY = 0;
float GyroErrorY;
float elapsedTime, currentTime, previousTime;

// LEDs
const int ledPin0 = D6;
const int ledPin1 = D5;
const int ledPin2 = D4;

// Pots
const int potPin0 = A1;
const int potPin1 = A2;
const int potPin2 = A3;

// Buttons
const int buttonPin0 = D10;
const int buttonPin1 = D11;
const int buttonPin2 = D12;
const int buttonPressedState = HIGH;

// Data array
int sensorData[7];

// Smoothing
const float alpha = 0.05;
float pot0Smooth = 0;
float pot1Smooth = 0;
float pot2Smooth = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12); // Nano ESP32: 0-4095

  Wire.begin();
  // If needed, use this instead:
  // Wire.begin(A4, A5);

  // Wake MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(100);

  calculate_IMU_error();

  pinMode(ledPin0, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  pinMode(buttonPin0, INPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  Serial.println("Nano ESP32 mapped serial sensor test ready");
}

void loop() {
  accgyro();

  int raw0 = analogRead(potPin0);
  int raw1 = analogRead(potPin1);
  int raw2 = analogRead(potPin2);

  pot0Smooth = alpha * raw0 + (1.0 - alpha) * pot0Smooth;
  pot1Smooth = alpha * raw1 + (1.0 - alpha) * pot1Smooth;
  pot2Smooth = alpha * raw2 + (1.0 - alpha) * pot2Smooth;

  // Active-high buttons: pressed = 1, released = 0
  int btn0 = digitalRead(buttonPin0) == buttonPressedState ? 1 : 0;
  int btn1 = digitalRead(buttonPin1) == buttonPressedState ? 1 : 0;
  int btn2 = digitalRead(buttonPin2) == buttonPressedState ? 1 : 0;

  sensorData[0] = mapPotToMidi((int)pot0Smooth);
  sensorData[1] = mapPotToMidi((int)pot1Smooth);
  sensorData[2] = mapPotToMidi((int)pot2Smooth);
  sensorData[3] = btn0;
  sensorData[4] = btn1;
  sensorData[5] = btn2;
  sensorData[6] = mapGyroToMidi(compAngleY);

  int brightness0 = map(sensorData[0], 1, 127, 0, 255);
  int brightness1 = map(sensorData[1], 1, 127, 0, 255);
  int brightness2 = map(sensorData[2], 1, 127, 0, 255);

  analogWrite(ledPin0, btn0 ? brightness0 : 0);
  analogWrite(ledPin1, btn1 ? brightness1 : 0);
  analogWrite(ledPin2, btn2 ? brightness2 : 0);

  printArray(sensorData, 7);

  delay(10);
}

int mapPotToMidi(int value) {
  value = constrain(value, 0, 4095);
  return map(value, 0, 4095, 1, 127);
}

int mapGyroToMidi(float angle) {
  int mappedValue = map((int)angle, 0, 90, 0, 127);

  if (mappedValue < 0) {
    return 0;
  }

  if (mappedValue > 127) {
    return 120;
  }

  return mappedValue;
}

void printArray(int arr[], int length) {
  for (int i = 0; i < length; i++) {
    Serial.print(arr[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void accgyro() {
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

  AccX = rawAccX / 16384.0;
  AccZ = rawAccZ / 16384.0;
  GyroY = rawGyroY / 131.0 - GyroErrorY;

  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000.0;

  float angleY = atan2(-AccX, AccZ) * 180.0 / PI;
  float frontY = angleY - angleYZero;
  compAngleY = 0.96 * (compAngleY + GyroY * elapsedTime) + 0.04 * frontY;
}

void calculate_IMU_error() {
  Serial.println("Calibrating MPU6050... keep it still.");

  const int samples = 300;
  float gyroSumY = 0;
  float angleSumY = 0;

  for (int i = 0; i < samples; i++) {
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

    AccX = rawAccX / 16384.0;
    AccZ = rawAccZ / 16384.0;
    GyroY = rawGyroY / 131.0;

    gyroSumY += GyroY;
    angleSumY += atan2(-AccX, AccZ) * 180.0 / PI;

    delay(5);
  }

  GyroErrorY = gyroSumY / samples;
  angleYZero = angleSumY / samples;
  compAngleY = 0;

  Serial.println("IMU Calibration Complete");
  currentTime = millis();
  previousTime = currentTime;
}
