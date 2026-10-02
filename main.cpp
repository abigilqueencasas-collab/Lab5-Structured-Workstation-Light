#include <Arduino.h>

// -------------------------------------------------------------
// Laboratory Activity 5: Structured Workstation Light
// Board: DOIT ESP32 DEVKIT V1 (PlatformIO)
// -------------------------------------------------------------

// Pin Definitions
const int BUTTON_PIN = 4; // Momentary enable button (active-LOW)
const int POT_PIN = 34; // Potentiometer wiper (ADC1_CH6)
const int LED_PIN = 18; // Workstation light output

// PWM Configuration
const int PWM_FREQ = 5000; // 5 kHz
const int PWM_RESOLUTION = 8; // 8-bit resolution (values 0 - 255)

// Global State Variables
bool isEnabled = false;
int rawPotValue = 0;
int outputDuty = 0;

// -------------------------------------------------------------
// Required Scaling Function: maps 12-bit ADC to 8-bit PWM duty
// -------------------------------------------------------------
int scaleToDuty(int raw) {
  int clamped = constrain(raw, 0, 4095);
  return map(clamped, 0, 4095, 0, 255);
}

// -------------------------------------------------------------
// 1. INPUT FUNCTION: Sample physical hardware
// -------------------------------------------------------------
void readInputs() {
  // Active-LOW logic: button reads LOW when held down
  isEnabled = (digitalRead(BUTTON_PIN) == LOW);
  rawPotValue = analogRead(POT_PIN);
}

// -------------------------------------------------------------
// 2. PROCESSING FUNCTION: Decision logic and math
// -------------------------------------------------------------
void processLogic() {
  if (isEnabled) {
    outputDuty = scaleToDuty(rawPotValue);
  } else {
    // When button is released, light control is disabled and output is forced off
    outputDuty = 0;
  }
}

// -------------------------------------------------------------
// 3. OUTPUT FUNCTION: Drive hardware using universal LEDC API
// -------------------------------------------------------------
void writeOutputs() {
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(LED_PIN, outputDuty);
  #else
    ledcWrite(0, outputDuty); // channel 0
  #endif
}

void setup() {
  Serial.begin(115200);

  // Configure digital input with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Setup PWM pin across both older (v2.x) and newer (v3.x+) ESP32 cores
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  #else
    ledcSetup(0, PWM_FREQ, PWM_RESOLUTION); // channel 0
    ledcAttachPin(LED_PIN, 0);
  #endif

  // Ensure output starts completely off upon boot/reset
  writeOutputs();
}

void loop() {
  readInputs();
  processLogic();
  writeOutputs();

  // Serial debug logging
  Serial.print("Enabled: ");
  Serial.print(isEnabled ? "YES" : "NO");
  Serial.print(" | Raw ADC: ");
  Serial.print(rawPotValue);
  Serial.print(" | Output Duty: ");
  Serial.println(outputDuty);

  delay(20);
}
