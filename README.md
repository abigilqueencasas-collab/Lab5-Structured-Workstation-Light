# Laboratory Activity 5 – Structured Workstation Light


## 1. Circuit Sketch (Inputs and Outputs)

```
                         ESP32 DevKit
                      ┌───────────────────┐
   3V3 ──────┬────────┤ 3V3               │
              │        │                   │
         ┌────┴────┐   │                   │
         │   POT    │  │                   │<img width="3072" height="4096" alt="circuit" src="https://github.com/user-attachments/assets/e6189a03-7bd4-4bfb-8de5-973eea4a4e40" />


         │ (10k)    ├──┼─ GPIO34 (ADC IN) ─┼── INPUT  (potentiometer wiper,
         │ wiper    │  │                   │           brightness setting)
         └────┬────┘   │                   │
   GND ───────┴────────┤ GND               │
                        │                   │
   3V3 ──[10kΩ]──┬──────┤ GPIO23 (INPUT) ──┼── INPUT  (button, Active-LOW,
                  │      │  (INPUT_PULLUP)  │           INPUT_PULLUP enabled)
              [BUTTON]   │                   │
                  │      │                   │
   GND ───────────┴──────┤                   │
                        │                   │
                        │ GPIO18 (OUTPUT) ──┼── OUTPUT (Status LED, through
                        │                   │           220Ω resistor to GND)
                        │                   │
                        │ GPIO19 (PWM OUT) ─┼── OUTPUT (Brightness LED, through
                        │                   │           220Ω resistor to GND)
                        └───────────────────┘
```

<img width="3072" height="4096" alt="circuit" src="https://github.com/user-attachments/assets/254437a0-ce95-4c8f-b1d2-d700b51a2bc7" />



**Inputs:**
- **GPIO23** – momentary push button (Active-LOW, `INPUT_PULLUP`) → enables/disables brightness control
- **GPIO34** – potentiometer wiper (analog input, 0–4095) → sets requested brightness

**Outputs:**
- **GPIO18** – Status LED → ON only while the button is held
- **GPIO19** – Brightness/PWM LED → follows the potentiometer, but only while the button is held

---

## 2. Labeled Wiring Table

| From | To | Purpose |
|---|---|---|
| ESP32 3V3 | Potentiometer pin 1 | Pot supply (high side) |
| ESP32 GND | Potentiometer pin 3 | Pot supply (low side) |
| Potentiometer wiper (pin 2) | ESP32 GPIO34 | Analog brightness input (ADC) |
| ESP32 3V3 | 10kΩ resistor → Button leg 1 | Button pull-up path |
| Button leg 1 | ESP32 GPIO23 | Button signal (reads LOW when pressed) |
| Button leg 2 | ESP32 GND | Completes circuit when button is pressed |
| ESP32 GPIO18 | 220Ω resistor → Status LED anode | Status LED signal |
| Status LED cathode | ESP32 GND | Status LED return path |
| ESP32 GPIO19 | 220Ω resistor → Brightness LED anode | PWM brightness signal |
| Brightness LED cathode | ESP32 GND | Brightness LED return path |

> Note: `pinMode(BUTTON_PIN, INPUT_PULLUP)` enables the ESP32's internal
> pull-up, so the external 10kΩ resistor is optional/redundant but does no
> harm if already wired that way.

---

## 3. Code Structure

| Function | Role | Notes |
|---|---|---|
| `readInputs()` | **INPUT** stage | Reads `digitalRead(BUTTON_PIN)` and `analogRead(POT_PIN)`; stores into `isEnabled` and `rawPotValue` |
| `processInputs()` | **PROCESS** stage | Decides whether to apply brightness or force zero, based on `isEnabled`; calls `scaleToDuty()` |
| `scaleToDuty(int raw)` | **Scaling helper** | Clamps the raw 0–4095 ADC reading and maps it to an 8-bit (0–255) PWM duty |
| `updateOutputs()` | **OUTPUT** stage | Writes `statusLedOn` to GPIO18 and `appliedDuty` to GPIO19 via `ledcWrite()` |

### How constants, Boolean state, and numeric readings are used

- **Constants** (`BUTTON_PIN`, `POT_PIN`, `STATUS_LED_PIN`, `PWM_LED_PIN`,
  `PWM_FREQ`, `PWM_RESOLUTION`) are declared once at the top with `const int`
  so every function references the same pin/setting by name instead of a
  "magic number," making the wiring easy to change in one place.
- **Boolean state** (`isEnabled`, `statusLedOn`) stores a simple true/false
  decision carried between stages: `readInputs()` sets `isEnabled` from the
  button level, and `processInputs()` uses it to decide whether the system
  is "live" this loop — this is what lets the status LED and brightness LED
  turn off together the instant the button is released.
- **Numeric readings** (`rawPotValue`, `appliedDuty`) hold the continuously
  changing analog data: `rawPotValue` is the raw 12-bit ADC code from the
  potentiometer, and `appliedDuty` is that value scaled down to the 8-bit
  range the PWM peripheral expects — scaling is isolated inside
  `scaleToDuty()` so it is testable and reusable.

---

## 4. Table of Expected vs Observed Behavior

| Test | Knob Position | Expected | Observed | Result |
|---|---|---|---|---|
| Reset, button released | — | Both LEDs OFF | Both LEDs OFF | ✅ Pass |
| Rotate knob while released | Low / Mid / High | No change — both LEDs stay OFF | Both LEDs stayed OFF at all positions | ✅ Pass |
| Hold button, knob at low | Low | Status LED ON, Brightness LED very dim/off | Status LED ON, Brightness LED dim | ✅ Pass |
| Hold button, knob at middle | Mid | Status LED ON, Brightness LED medium brightness | Status LED ON, Brightness LED at ~50% brightness | ✅ Pass |
| Hold button, knob at high | High | Status LED ON, Brightness LED at full brightness | Status LED ON, Brightness LED at full brightness | ✅ Pass |
| Release button (any knob position) | Any | Both LEDs OFF immediately | Both LEDs turned OFF immediately | ✅ Pass |

**Summary:** All required tests passed. The status LED and brightness LED
only respond while the button is actively held, confirming the enable/
disable gating works correctly. Rotating the knob has zero effect on the
output while the button is released, satisfying requirement #4. Brightness scales smoothly and proportionally from low → middle → high knob positions
while the button is held, confirming the `scaleToDuty()` mapping works
correctly across the full range.


## Demonstration
https://drive.google.com/file/d/1t7YXiqEMqoisXlZYdlQwO7_aaZ7JhoQ3/view?usp=sharing
