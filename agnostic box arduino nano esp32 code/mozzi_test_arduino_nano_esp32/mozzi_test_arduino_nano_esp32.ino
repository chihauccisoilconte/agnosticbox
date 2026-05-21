#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_PWM
#define MOZZI_AUDIO_PIN_1 D9

#include <Mozzi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>

#define CONTROL_RATE 128

const int POT_PITCH = A1;
const int POT_MOD   = A2;
const int POT_ECHO  = A3;

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

#define ECHO_SIZE 2048
int8_t echoBuffer[ECHO_SIZE];
int echoIndex = 0;
int echoMix = 0;
int echoFeedback = 0;

unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);

  pinMode(BTN_SINE, INPUT_PULLUP);
  pinMode(BTN_SAW, INPUT_PULLUP);
  pinMode(BTN_SQUARE, INPUT_PULLUP);

  pinMode(LED_SINE, OUTPUT);
  pinMode(LED_SAW, OUTPUT);
  pinMode(LED_SQUARE, OUTPUT);

  lfo.setFreq(5.0f);

  Serial.println("3 oscillator Mozzi synth with echo tails");

  startMozzi(CONTROL_RATE);
}

void updateControl() {
  int pPitch = analogRead(POT_PITCH);
  int pMod   = analogRead(POT_MOD);
  int pEcho  = analogRead(POT_ECHO);

  sineHeld   = digitalRead(BTN_SINE) == HIGH;
  sawHeld    = digitalRead(BTN_SAW) == HIGH;
  squareHeld = digitalRead(BTN_SQUARE) == HIGH;

  digitalWrite(LED_SINE, sineHeld);
  digitalWrite(LED_SAW, sawHeld);
  digitalWrite(LED_SQUARE, squareHeld);

  float baseFreq = map(pPitch, 0, 4095, 80, 900);

  float modDepth = map(pMod, 0, 4095, 0, 80);
  float lfoRate  = map(pMod, 0, 4095, 20, 900) / 100.0f;

  lfo.setFreq(lfoRate);

  int lfoValue = lfo.next();
  float vibrato = ((float)lfoValue / 127.0f) * modDepth;

  float freq = baseFreq + vibrato;
  if (freq < 40.0f) freq = 40.0f;

  sineOsc.setFreq(freq);
  sawOsc.setFreq(freq * 0.995f);
  squareOsc.setFreq(freq * 1.005f);

  echoMix = map(pEcho, 0, 4095, 0, 90);
  echoFeedback = map(pEcho, 0, 4095, 0, 95);

  if (millis() - lastPrint > 250) {
    lastPrint = millis();

    Serial.print("Pitch:");
    Serial.print(pPitch);

    Serial.print(" Mod:");
    Serial.print(pMod);

    Serial.print(" Echo:");
    Serial.print(pEcho);

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