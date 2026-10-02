#include <Arduino.h>


// -------------------------------------------------------------
// Pin and Configuration Constants
// -------------------------------------------------------------
const int BUTTON_PIN     = 4;   // Momentary enable button (Active-LOW)
const int POT_PIN        = 34;  // Potentiometer input
const int STATUS_LED_PIN = 2;   // Status indicator LED
const int PWM_LED_PIN    = 18;  // Dimming LED output


// PWM Peripheral Configuration (Core v2.x requires a PWM channel)
const int PWM_CHANNEL    = 0;
const int PWM_FREQ       = 5000; // 5 kHz
const int PWM_RESOLUTION = 8;    // 8-bit resolution (0 - 255)


// -------------------------------------------------------------
// Global Variables for System State
// -------------------------------------------------------------
bool isEnabled    = false; // Button state: true when held, false when released
int rawPotValue   = 0;     // Raw ADC value from potentiometer (0 - 4095)
int appliedDuty   = 0;     // Computed PWM duty cycle (0 - 255)
bool statusLedOn  = false; // State of the status LED


// -------------------------------------------------------------
// Helper / Scaling Function
// -------------------------------------------------------------
int scaleToDuty(int raw) {
  int clamped = constrain(raw, 0, 4095);
  return map(clamped, 0, 4095, 0, 255);
}


// -------------------------------------------------------------
// Structured Architecture Functions
// -------------------------------------------------------------


// 1. INPUT: Reads button state and potentiometer value
void readInputs() {
  // Active-LOW button: LOW means pressed / held
  isEnabled = (digitalRead(BUTTON_PIN) == LOW);
  rawPotValue = analogRead(POT_PIN);
}


// 2. PROCESS: Scales input and decides whether to apply duty or zero
void processInputs() {
  if (isEnabled) {
    statusLedOn = true;
    appliedDuty = scaleToDuty(rawPotValue);
  } else {
    statusLedOn = false;
    appliedDuty = 0;
  }
}


// 3. WRITE: Sends the decision to both outputs
void updateOutputs() {
  // Update status LED
  digitalWrite(STATUS_LED_PIN, statusLedOn ? HIGH : LOW);


  // Update PWM brightness LED via PWM channel
  ledcWrite(PWM_CHANNEL, appliedDuty);
}


// -------------------------------------------------------------
// Setup & Main Loop
// -------------------------------------------------------------
void setup() {
  Serial.begin(115200);


  // Configure Inputs
  pinMode(BUTTON_PIN, INPUT_PULLUP);


  // Configure Status Output
  pinMode(STATUS_LED_PIN, OUTPUT);


  // Configure PWM Output (ESP32 Core v2.x syntax)
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PWM_LED_PIN, PWM_CHANNEL);


  // Initial state check: ensure outputs start off
  updateOutputs();
}


void loop() {
  readInputs();
  processInputs();
  updateOutputs();


  // Serial debug monitor output
  Serial.print("Enabled: ");
  Serial.print(isEnabled ? "YES" : "NO");
  Serial.print(" | ADC: ");
  Serial.print(rawPotValue);
  Serial.print(" | Applied Duty: ");
  Serial.println(appliedDuty);


  // 20 ms loop pacing
  delay(20);
}
