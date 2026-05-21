/*
  Arduino Nano ESP32
  MPU6050 + 3 Pots + 3 Buttons + 3 LEDs

  Array:
  [pot0, pot1, pot2, button0, button1, button2, pitch]

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
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY;
float gyroAngleX, gyroAngleY;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;

// LEDs
const int ledPin0 = D6;
const int ledPin1 = D5;
const int ledPin2 = D4;

// Pots
const int potPin0 = A1;
const int potPin1 = A2;
const int potPin2 = A3;

// Buttons
const int buttonPin0 = D12;
const int buttonPin1 = D11;
const int buttonPin2 = D10;

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

  analogReadResolution(12); // Nano ESP32: 0–4095

  // I2C on Nano ESP32
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

  // Use INPUT_PULLUP for stable buttons
  // This means: pressed = LOW, released = HIGH
  pinMode(buttonPin0, INPUT_PULLUP);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  Serial.println("Nano ESP32 sensor test ready");
}

void loop() {
  accgyro();

  int raw0 = analogRead(potPin0);
  int raw1 = analogRead(potPin1);
  int raw2 = analogRead(potPin2);

  pot0Smooth = alpha * raw0 + (1.0 - alpha) * pot0Smooth;
  pot1Smooth = alpha * raw1 + (1.0 - alpha) * pot1Smooth;
  pot2Smooth = alpha * raw2 + (1.0 - alpha) * pot2Smooth;

  // INPUT_PULLUP: pressed = LOW
  int btn0 = digitalRead(buttonPin0) == LOW ? 1 : 0;
  int btn1 = digitalRead(buttonPin1) == LOW ? 1 : 0;
  int btn2 = digitalRead(buttonPin2) == LOW ? 1 : 0;

  sensorData[0] = (int)pot0Smooth;
  sensorData[1] = (int)pot1Smooth;
  sensorData[2] = (int)pot2Smooth;
  sensorData[3] = btn0;
  sensorData[4] = btn1;
  sensorData[5] = btn2;
  sensorData[6] = (int)pitch;

  // LED brightness: 12-bit ADC to 8-bit PWM
  int brightness0 = sensorData[0] / 16;
  int brightness1 = sensorData[1] / 16;
  int brightness2 = sensorData[2] / 16;

  analogWrite(ledPin0, btn0 ? brightness0 : 0);
  analogWrite(ledPin1, btn1 ? brightness1 : 0);
  analogWrite(ledPin2, btn2 ? brightness2 : 0);

  printArray(sensorData, 7);

  delay(10);
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
  Wire.requestFrom(MPU, 6, true);

  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;

  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000.0;

  Wire.beginTransmission(MPU);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);

  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  GyroX -= GyroErrorX;
  GyroY -= GyroErrorY;
  GyroZ -= GyroErrorZ;

  gyroAngleX += GyroX * elapsedTime;
  gyroAngleY += GyroY * elapsedTime;
  yaw += GyroZ * elapsedTime;

  roll  = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;
}

void calculate_IMU_error() {
  Serial.println("Calibrating MPU6050... keep it still.");

  c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

    AccErrorX += atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI;
    AccErrorY += atan(-AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI;

    c++;
  }

  AccErrorX /= 200;
  AccErrorY /= 200;

  c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

    GyroErrorX += GyroX;
    GyroErrorY += GyroY;
    GyroErrorZ += GyroZ;

    c++;
  }

  GyroErrorX /= 200;
  GyroErrorY /= 200;
  GyroErrorZ /= 200;

  Serial.println("IMU Calibration Complete");
}