# BCA188 - IoT Firmware Programming and Device I/O
## Laboratory Activity 5: Structured Workstation Light


## Task
Use Example 6 as a starting point. Keep separate input, processing, and output functions. Hold the button to enable brightness control and release it to disable.


## Materials
| Qty | Item |
|---|---|
| 1 | ESP32 DevKit (DOIT ESP32 DEVKIT V1) |
| 1 | USB cable |
| 1 | Breadboard |
| 1 | LED |
| 1 | Series resistor for the LED (value: ___ Ω) |
| 1 | Pushbutton (normally open) |
| 1 | 10 kΩ potentiometer |
| several | Jumper wires |

## 1. Circuit: inputs and outputs

| Role | Name in code | GPIO | Type | Connection |
|---|---|---|---|---|
| Enable button | `BUTTON_PIN` | 4 | Digital input (INPUT_PULLUP, active-LOW) | GPIO4 → button → GND |
| Brightness knob | `POT_PIN` | 34 | Analog input (ADC1_CH6) | Outer terminals → 3V3 and GND; wiper → GPIO34 |
| Workstation light | `LED_PIN` | 18 | PWM output | GPIO18 → resistor → LED anode; cathode → GND |

**Inputs:** the button (GPIO4) and the potentiometer (GPIO34).
**Output:** the LED (GPIO18), driven by PWM.

<img width="1360" height="840" alt="lab5_wiring_diagram" src="https://github.com/user-attachments/assets/ced1a5b4-cebe-4130-9656-8a6b0b92bf00" />


## Source code
```cpp
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
```

## 2. How constants, Boolean state, and numeric readings are used

- **Constants** (`BUTTON_PIN`, `POT_PIN`, `LED_PIN`, `PWM_FREQ`, `PWM_RESOLUTION`) name the wiring and PWM settings that should not change while the program runs. If a pin moves, only one line needs to be edited.
- **Boolean state** (`isEnabled`) stores a yes/no fact: is the button being held? `readInputs()` sets it from the button (`LOW` means held), and `processLogic()` uses it to decide whether the knob is allowed to control the light.
- **Numeric readings** (`rawPotValue`, `outputDuty`) hold numbers. `rawPotValue` is the 12-bit ADC code from the potentiometer (0 to 4095), and `outputDuty` is the 8-bit PWM value (0 to 255) that is finally written to the LED. They are kept separate so the knob can be read at all times while the output stays off when the button is released.

## 3. Scaling function

The scaling calculation is placed in its own function, as required:

```cpp
int scaleToDuty(int raw) {
  int clamped = constrain(raw, 0, 4095);
  return map(clamped, 0, 4095, 0, 255);
}
```

It limits the input to the valid ADC range, then converts the 12-bit reading (0 to 4095) into an 8-bit duty (0 to 255).

Program structure: `readInputs()` samples the hardware, `processLogic()` decides the duty, and `writeOutputs()` drives the LED. `loop()` just calls them in that order.

## 4. Knob while released

When the button is released, `isEnabled` is `false`, so `processLogic()` forces `outputDuty = 0`. Rotating the knob changes `rawPotValue`, but the LED stays off.

Confirmed: _(yes / no)_

## 5. Results for low, middle, and high knob positions (button held)

| Knob position | Raw ADC | Predicted duty | Observed duty | LED brightness |
|---|---|---|---|---|
| Low | 0 | 0 | 0 | LED off / lowest brightness |
| Middle | 2048 | 127 | 127 | Medium brightness |
| High | 4095 | 255 | 255 | Full brightness |

Predicted duty uses `raw × 255 / 4095` with integer arithmetic (about 0 at raw 0, 127 at raw 2048, and 255 at raw 4095). Real readings will differ slightly.

## Required tests: expected vs. observed

| Test | Expected | Observed |
|---|---|---|
| Reset with button released | LED off | LED off |
| Hold button | Brightness control enabled (`Enabled: YES`) | Enabled: YES, brightness control active |
| Rotate knob while held | Brightness changes with the knob | Brightness changed smoothly with the knob |
| Rotate knob while released | LED stays off | LED stayed off |
| Release button | LED off | LED turned off |

## Demonstration
https://drive.google.com/file/d/1wOJQEWqh0Q2W5F3OBx44Ik6_h0Xm3yAT/view?usp=sharing
