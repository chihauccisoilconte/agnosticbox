#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_PWM
#define MOZZI_AUDIO_PIN_1 D9

#include <Wire.h>
#include <Mozzi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>

#define CONTROL_RATE 128

const int MPU = 0x68;

const int POT_PITCH  = A1;
const int POT_VOLUME = A2;
const int POT_ECHO   = A3;

const int BTN_SINE   = D10;
const int BTN_SAW    = D11;
const int BTN_SQUARE = D12;

const int LED_SINE   = D4;
const int LED_SAW    = D5;
const int LED_SQUARE = D6;

Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> sineOsc(SIN2048_DATA);
Oscil<SAW2048_NUM_CELLS, AUDIO_RATE> sawOsc(SAW2048_DATA);
Oscil<SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> squareOsc(SQUARE_NO_ALIAS_2048_DATA);
Oscil<SIN2048_NUM_CELLS, CONTROL_RATE> lfo(SIN2048_DATA);

bool sineHeld = false;
bool sawHeld = false;
bool squareHeld = false;

float accX = 0;
float accZ = 0;
float gyroY = 0;
float gyroOffsetY = 0;
float angleYZero = 0;
float compAngleY = 0;
unsigned long previousMpuTime = 0;

#define ECHO_SIZE 2048
int8_t echoBuffer[ECHO_SIZE];
int echoIndex = 0;
int echoMix = 0;
int echoFeedback = 0;
int volumeLevel = 127;

unsigned long lastPrint = 0;

float mapFloat(float value, float inMin, float inMax, float outMin, float outMax) {
  return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);

  Wire.begin();

  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(100);
  calibrateMpu();
  previousMpuTime = millis();

  pinMode(BTN_SINE, INPUT_PULLUP);
  pinMode(BTN_SAW, INPUT_PULLUP);
  pinMode(BTN_SQUARE, INPUT_PULLUP);

  pinMode(LED_SINE, OUTPUT);
  pinMode(LED_SAW, OUTPUT);
  pinMode(LED_SQUARE, OUTPUT);

  lfo.setFreq(5.0f);

  Serial.println("3 oscillator Mozzi synth with MPU6050 vibrato and volume knob");

  startMozzi(CONTROL_RATE);
}

void updateControl() {
  updateMpuAngle();

  int pPitch = analogRead(POT_PITCH);
  int pVolume = analogRead(POT_VOLUME);
  int pEcho = analogRead(POT_ECHO);

  sineHeld   = digitalRead(BTN_SINE) == HIGH;
  sawHeld    = digitalRead(BTN_SAW) == HIGH;
  squareHeld = digitalRead(BTN_SQUARE) == HIGH;

  digitalWrite(LED_SINE, sineHeld);
  digitalWrite(LED_SAW, sawHeld);
  digitalWrite(LED_SQUARE, squareHeld);

  float baseFreq = map(pPitch, 0, 4095, 80, 900);

  float tiltAmount = constrain(abs(compAngleY), 0.0f, 45.0f);
  float modDepth = mapFloat(tiltAmount, 0.0f, 45.0f, 0.0f, 80.0f);
  float lfoRate = mapFloat(tiltAmount, 0.0f, 45.0f, 2.0f, 9.0f);

  lfo.setFreq(lfoRate);

  int lfoValue = lfo.next();
  float vibrato = ((float)lfoValue / 127.0f) * modDepth;

  float freq = baseFreq + vibrato;
  if (freq < 40.0f) freq = 40.0f;

  sineOsc.setFreq(freq);
  sawOsc.setFreq(freq * 0.995f);
  squareOsc.setFreq(freq * 1.005f);

  volumeLevel = map(pVolume, 0, 4095, 0, 127);
  echoMix = map(pEcho, 0, 4095, 0, 90);
  echoFeedback = map(pEcho, 0, 4095, 0, 95);

  if (millis() - lastPrint > 250) {
    lastPrint = millis();

    Serial.print("Pitch:");
    Serial.print(pPitch);

    Serial.print(" Volume:");
    Serial.print(pVolume);

    Serial.print(" Echo:");
    Serial.print(pEcho);

    Serial.print(" | GyroY angle:");
    Serial.print(compAngleY, 1);

    Serial.print(" | ModDepth:");
    Serial.print(modDepth, 1);

    Serial.print(" | LfoRate:");
    Serial.print(lfoRate, 1);

    Serial.print(" | Freq:");
    Serial.print(freq);

    Serial.print(" | Sine:");
    Serial.print(sineHeld);

    Serial.print(" Saw:");
    Serial.print(sawHeld);

    Serial.print(" Square:");
    Serial.println(squareHeld);
  }
}

AudioOutput updateAudio() {
  int mix = 0;

  if (sineHeld) {
    mix += sineOsc.next();
  } else {
    sineOsc.next();
  }

  if (sawHeld) {
    mix += sawOsc.next();
  } else {
    sawOsc.next();
  }

  if (squareHeld) {
    mix += squareOsc.next();
  } else {
    squareOsc.next();
  }

  if (!sineHeld && !sawHeld && !squareHeld) {
    mix = 0;
  }

  int heldCount = 0;
  if (sineHeld) heldCount++;
  if (sawHeld) heldCount++;
  if (squareHeld) heldCount++;

  if (heldCount >= 2) {
    mix = mix / heldCount;
  }

  mix = (mix * volumeLevel) >> 7;

  int delayed = echoBuffer[echoIndex];

  int withEcho = mix + ((delayed * echoMix) >> 7);
  int feedbackSample = mix + ((delayed * echoFeedback) >> 7);

  if (feedbackSample > 127) feedbackSample = 127;
  if (feedbackSample < -128) feedbackSample = -128;

  echoBuffer[echoIndex] = feedbackSample;

  echoIndex++;
  if (echoIndex >= ECHO_SIZE) echoIndex = 0;

  if (withEcho > 127) withEcho = 127;
  if (withEcho < -128) withEcho = -128;

  return MonoOutput::from8Bit(withEcho);
}

void loop() {
  audioHook();
}

void updateMpuAngle() {
  readMpu();

  unsigned long currentTime = millis();
  float elapsedTime = (currentTime - previousMpuTime) / 1000.0f;
  previousMpuTime = currentTime;

  float angleY = atan2(-accX, accZ) * 180.0f / PI;
  float frontY = angleY - angleYZero;

  compAngleY = 0.96f * (compAngleY + gyroY * elapsedTime) + 0.04f * frontY;
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

  accX = rawAccX / 16384.0f;
  accZ = rawAccZ / 16384.0f;
  gyroY = rawGyroY / 131.0f - gyroOffsetY;
}

void calibrateMpu() {
  Serial.println("Calibrating MPU6050... keep it still.");

  const int samples = 300;
  float gyroSumY = 0;
  float angleSumY = 0;

  for (int i = 0; i < samples; i++) {
    readMpu();

    gyroSumY += gyroY;
    angleSumY += atan2(-accX, accZ) * 180.0f / PI;

    delay(5);
  }

  gyroOffsetY = gyroSumY / samples;
  angleYZero = angleSumY / samples;
  compAngleY = 0;

  Serial.println("IMU Calibration Complete");
}
