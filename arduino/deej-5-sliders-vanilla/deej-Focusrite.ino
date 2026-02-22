#include <math.h>

// ===== ANTIPARALLELE BICOLOR LED PINS =====
const int led1A = 2;
const int led1B = 3;
const int led2A = 4;
const int led2B = 5;
const int led3A = 6;
const int led3B = 7;
const int led4A = 8;
const int led4B = 9;

// ===== WEISSE LED =====
const int whiteLED = 10;

// ===== GLOBALER MUTE =====
const int muteButtonPin = 11;

// ===== ANZAHL KANÄLE =====
const int NUM_CHANNELS = 4;

// ===== KANAL-TASTER =====
const int channelButtonPins[NUM_CHANNELS] = {A0, A1, A2, A3};

// ===== POTIS =====
const int potPins[NUM_CHANNELS] = {A4, A5, A6, A7};

// ===== VARIABLEN =====
int sendValues[NUM_CHANNELS];
bool channelEnabled[NUM_CHANNELS] = {true, true, true, true};
bool lastButtonState[NUM_CHANNELS];

bool muteActive = false;
bool lastMuteButtonState = HIGH;

bool blinkState = false;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 400;

unsigned long lastSend = 0;
const unsigned long sendInterval = 40; // ~25Hz

// ===== SETUP =====
void setup() {

  Serial.begin(9600);

  // CH340 + Win11 Stabilisierung
  delay(4000);

  // Start-Frames senden
  for (int i = 0; i < 50; i++) {
    Serial.println("1|2|3|4");
    delay(30);
  }

  pinMode(whiteLED, OUTPUT);
  analogWrite(whiteLED, 8);

  pinMode(muteButtonPin, INPUT_PULLUP);

  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(channelButtonPins[i], INPUT_PULLUP);
    pinMode(potPins[i], INPUT);

    pinMode(getLEDA(i), OUTPUT);
    pinMode(getLEDB(i), OUTPUT);

    digitalWrite(getLEDA(i), LOW);
    digitalWrite(getLEDB(i), LOW);

    lastButtonState[i] = digitalRead(channelButtonPins[i]);
  }
}

// ===== LOOP =====
void loop() {

  readMuteButton();
  readChannelButtons();
  updateSendValues();
  updateBlink();
  updateLEDs();

  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();
    sendValuesSerial();
  }
}

// ===== FUNKTIONEN =====

void readMuteButton() {
  int reading = digitalRead(muteButtonPin);
  if (lastMuteButtonState == HIGH && reading == LOW) {
    muteActive = !muteActive;
    blinkState = true;
    lastBlinkTime = millis();
  }
  lastMuteButtonState = reading;
}

void readChannelButtons() {
  for (int i = 0; i < NUM_CHANNELS; i++) {
    int reading = digitalRead(channelButtonPins[i]);
    if (lastButtonState[i] == HIGH && reading == LOW) {
      channelEnabled[i] = !channelEnabled[i];
    }
    lastButtonState[i] = reading;
  }
}

// ===== LINEARISIERUNG FÜR LOG-POTIS =====
void updateSendValues() {

  static float smoothValues[NUM_CHANNELS] = {0,0,0,0};
  const float smoothing = 0.15;   // 0.05 = sehr weich | 0.3 = direkter

  for (int i = 0; i < NUM_CHANNELS; i++) {

    if (!channelEnabled[i] || muteActive) {
      sendValues[i] = 1023;
      smoothValues[i] = 1023;   // wichtig: kein Nachziehen nach Mute
    } 
    else {

      int raw = analogRead(potPins[i]);
      float norm = raw / 1023.0;

      // sanftere Anti-Log-Korrektur
      norm = pow(norm, 1.6);

      float target = norm * 1023.0;

      // Exponential smoothing
      smoothValues[i] = smoothValues[i] + smoothing * (target - smoothValues[i]);

      sendValues[i] = (int)(smoothValues[i] + 0.5);
    }
  }
}

void updateBlink() {
  if (!muteActive) {
    blinkState = false;
    return;
  }

  if (millis() - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = millis();
    blinkState = !blinkState;
  }
}

void updateLEDs() {
  for (int i = 0; i < NUM_CHANNELS; i++) {

    if (!channelEnabled[i]) {
      setLEDRed(i, true);
    }
    else if (muteActive) {
      setLEDRed(i, blinkState);
    }
    else {
      setLEDGreen(i, true);
    }
  }
}

void setLEDRed(int index, bool on) {
  digitalWrite(getLEDA(index), on ? HIGH : LOW);
  digitalWrite(getLEDB(index), LOW);
}

void setLEDGreen(int index, bool on) {
  digitalWrite(getLEDA(index), LOW);
  digitalWrite(getLEDB(index), on ? HIGH : LOW);
}

int getLEDA(int i) {
  const int pins[4] = {led1A, led2A, led3A, led4A};
  return pins[i];
}

int getLEDB(int i) {
  const int pins[4] = {led1B, led2B, led3B, led4B};
  return pins[i];
}

// ===== STABILES SERIAL SENDEN =====
void sendValuesSerial() {

  static char line[32];

  snprintf(line, sizeof(line), "%d|%d|%d|%d",
           sendValues[0],
           sendValues[1],
           sendValues[2],
           sendValues[3]);

  Serial.println(line);
}
