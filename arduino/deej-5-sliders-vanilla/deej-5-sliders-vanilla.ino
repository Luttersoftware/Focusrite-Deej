

// LED 1
const int led1Gruen = 2;
const int led1Rot   = 3;

// LED 2
const int led2Gruen = 4;
const int led2Rot   = 5;

// LED 3
const int led3Gruen = 6;
const int led3Rot   = 7;

// LED 4
const int led4Gruen = 8;
const int led4Rot   = 9;

// ===== WEISSE LED =====
const int whiteLED = 10; // immer HIGH

// ===== ANALOG INPUTS =====
const int NUM_CHANNELS = 4;

// Potis
const int potPins[NUM_CHANNELS] = {A0, A1, A2, A3};

// Schalter (Pull-Up)
const int switchPins[NUM_CHANNELS] = {A4, A5, A6, A7};

// Speicher für Werte
int potValues[NUM_CHANNELS];
int sendValues[NUM_CHANNELS];


// ===== SETUP =====
void setup() {
  // LED Pins
  pinMode(led1Gruen, OUTPUT); pinMode(led1Rot, OUTPUT);
  pinMode(led2Gruen, OUTPUT); pinMode(led2Rot, OUTPUT);
  pinMode(led3Gruen, OUTPUT); pinMode(led3Rot, OUTPUT);
  pinMode(led4Gruen, OUTPUT); pinMode(led4Rot, OUTPUT);

  // Weiße LED
  pinMode(whiteLED, OUTPUT);
  digitalWrite(whiteLED, HIGH);

  // Potis
  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(potPins[i], INPUT);
  }

  // Schalter mit Pull-Up
  for (int i = 0; i < NUM_CHANNELS; i++) {
    pinMode(switchPins[i], INPUT_PULLUP);
  }

  Serial.begin(9600);
}


// ===== LOOP =====
void loop() {
  readInputs();
  updateLEDs();
  sendValuesSerial();
  delay(10);
}


// ===== FUNKTIONEN =====
void readInputs() {
  for (int i = 0; i < NUM_CHANNELS; i++) {
    int raw = analogRead(potPins[i]);

    // Log-Poti linearisieren (quadratische Näherung)
    float norm = pow(raw / 1023.0, 2);  // x^2 approximiert Linearität
    potValues[i] = int(norm * 1023);    // Linearer Wert 0–1023

    // Pull-Up: LOW = Schalter AN
    int switchState = digitalRead(switchPins[i]);

    if (switchState == LOW) { // Schalter AN
      sendValues[i] = potValues[i];
    } else {                 // Schalter AUS
      sendValues[i] = 0;
    }
  }
}

void updateLEDs() {
  setLED(0, sendValues[0] > 0);
  setLED(1, sendValues[1] > 0);
  setLED(2, sendValues[2] > 0);
  setLED(3, sendValues[3] > 0);
}

void setLED(int index, bool state) {
  const int gruenPins[4] = {led1Gruen, led2Gruen, led3Gruen, led4Gruen};
  const int rotPins[4]   = {led1Rot,   led2Rot,   led3Rot,   led4Rot};

  if (state) {
    digitalWrite(gruenPins[index], HIGH);
    digitalWrite(rotPins[index], LOW);
  } else {
    digitalWrite(gruenPins[index], LOW);
    digitalWrite(rotPins[index], HIGH);
  }
}

void sendValuesSerial() {
  String builtString = "";

  for (int i = 0; i < NUM_CHANNELS; i++) {
    builtString += String(sendValues[i]);
    if (i < NUM_CHANNELS - 1) builtString += "|";
  }

  Serial.println(builtString);
}

// ===== BICOLOR LED PINS =====
/* 
  ===== Pin-Belegung Arduino Nano =====
  
  Bicolor-LEDs:
    LED1 Grün   -> D2
    LED1 Rot    -> D3
    LED2 Grün   -> D4
    LED2 Rot    -> D5
    LED3 Grün   -> D6
    LED3 Rot    -> D7
    LED4 Grün   -> D8
    LED4 Rot    -> D9

  Weiße LED (immer an):
    -> D10

  Potentiometer (logarithmisch, linearisiert):
    Poti1 -> A0 (zu Schalter1 / LED1)
    Poti2 -> A1 (zu Schalter2 / LED2)
    Poti3 -> A2 (zu Schalter3 / LED3)
    Poti4 -> A3 (zu Schalter4 / LED4)

  Schalter (Pull-Up aktiviert):
    Schalter1 -> A4 (LOW = AN, HIGH = AUS)
    Schalter2 -> A5 (LOW = AN, HIGH = AUS)
    Schalter3 -> A6 (LOW = AN, HIGH = AUS)
    Schalter4 -> A7 (LOW = AN, HIGH = AUS)
*/

/*
  ===== Arduino Nano Pin-Belegung – Verdrahtung =====
  
            +5V
             |
             |
           [Potis]
    A0 ---|\/|--- GND  <- Poti1
    A1 ---|\/|--- GND  <- Poti2
    A2 ---|\/|--- GND  <- Poti3
    A3 ---|\/|--- GND  <- Poti4

           [Schalter]
    A4 ---/ --- GND  <- Schalter1 (LOW = AN, HIGH = AUS)
    A5 ---/ --- GND  <- Schalter2
    A6 ---/ --- GND  <- Schalter3
    A7 ---/ --- GND  <- Schalter4

           [Bicolor-LEDs]
       D2 ──[R]─> Grün ┌─┐
       D3 ──[R]─> Rot  └─┘  <- LED1
       D4 ──[R]─> Grün ┌─┐
       D5 ──[R]─> Rot  └─┘  <- LED2
       D6 ──[R]─> Grün ┌─┐
       D7 ──[R]─> Rot  └─┘  <- LED3
       D8 ──[R]─> Grün ┌─┐
       D9 ──[R]─> Rot  └─┘  <- LED4

          [Weiße LED immer an]
       D10 ──[R]─> LED ── GND

  Hinweise:
  - [R] = 220–330 Ω Widerstand
  - Schalter: zwischen Pin und GND
  - Bicolor-LEDs: jeder Pin + eigener Widerstand
  - Weiße LED: Pin D10 über Widerstand an Anode, Kathode an GND
  - Poti linearisiert im Sketch (logarithmisch -> linear)
*/
