# RC Bionic Butterfly: Complete Scratch-Build DIY Manual & Engineering Guide

---

## Table of Contents
1. [Introduction & Aerodynamic Principles](#1-introduction--aerodynamic-principles)
2. [Microcontroller Upgrade Research & Flight Response Analysis](#2-microcontroller-upgrade-research--flight-response-analysis)
3. [Key Innovations from Max Imagination's ESP-FLUTTER (Video Breakdown)](#3-key-innovations-from-max-imaginations-esp-flutter-video-breakdown)
4. [Complete Bill of Materials (Procurement Inventory)](#4-complete-bill-of-materials-procurement-inventory)
5. [Phase 1: 3D Printing & Component Preparation](#5-phase-1-3d-printing--component-preparation)
6. [Phase 2: Wing Skeleton, Glowing LEDs & Aerodynamic Skinning](#6-phase-2-wing-skeleton-glowing-leds--aerodynamic-skinning)
7. [Phase 3: Servo Mechanics & Chassis Assembly](#7-phase-3-servo-mechanics--chassis-assembly)
8. [Phase 4: Electrical Wiring & Power Conditioning](#8-phase-4-electrical-wiring--power-conditioning)
9. [Phase 5: Firmware Configuration & Flashing (Arduino Pro Micro & ESP32-C3)](#9-phase-5-firmware-configuration--flashing-arduino-pro-micro--esp32-c3)
10. [Phase 6: Flying via Smartphone Web App or RC Transmitter](#10-phase-6-flying-via-smartphone-web-app-or-rc-transmitter)
11. [Phase 7: Pre-Flight Checks, CG Balancing & Maiden Flight](#11-phase-7-pre-flight-checks-cg-balancing--maiden-flight)
12. [Troubleshooting & Flight Tuning](#12-troubleshooting--flight-tuning)

---

## 1. Introduction & Aerodynamic Principles

Unlike conventional propeller aircraft or multirotors, an **ornithopter** generates both **lift** and **forward thrust** through the synchronized flapping of flexible membrane wings.

```
       Flapping Axis                     Flapping Axis
           \                                 /
            \                               /
      [ Left Wing ]                   [ Right Wing ]
             \                             /
              +---[ Left Servo ]-[ Right Servo ]---+
                           [ Central Chassis ]
```

### How the Two-Servo Bionic Butterfly Flies
1. **Thrust via Aeroelastic Flex:** The leading edge is rigid (1.5mm carbon fiber rod), while the trailing edge and ribs are flexible (0.8mm–1.0mm carbon fiber rods). When the servo pushes the wing down on the downstroke, the leading edge descends first and the flexible trailing edge deflects upward, directing air backward to produce forward thrust. On the upstroke, the trailing edge bends downward, continuing to direct air backward.
2. **Steering (Differential Amplitude):** Rather than merely tilting the wings, high-performance ornithopters modulate the *stroke amplitude* between wings. To turn right, the left wing flaps through a larger arc (generating higher thrust/lift) while the right wing flaps through a smaller arc. This induces a clean, natural bank-and-turn yaw moment without stalling the inside wing!
3. **Turn Lift Boost:** When banked in a turn, vertical lift decreases. An automatic +15% throttle boost during hard steering maintains altitude.
4. **Pitch (Climb / Dive):** Achieved through symmetric wing bias. Shifting the neutral stroke angle of both wings upward shifts the center of aerodynamic pressure forward relative to the Center of Gravity (CG), pitching the nose up to climb. Shifting them downward pitches the nose down.

---

## 2. Microcontroller Upgrade Research & Flight Response Analysis

The baseline build uses an **Arduino Pro Micro (ATmega32U4, 8-bit AVR @ 16MHz)** with an external PPM receiver. Replacing the microcontroller substantially improves flight responsiveness, weight, and stability:

| Feature / Metric | Stock: Arduino Pro Micro (ATmega32U4) | Upgrade 1: Seeed Studio XIAO ESP32-C3 | Upgrade 2: FPV Micro AIO (STM32F411 + Gyro + ELRS) | Upgrade 3: Waveshare RP2040-Zero |
| :--- | :--- | :--- | :--- | :--- |
| **CPU Architecture** | 8-bit AVR @ 16 MHz | 32-bit RISC-V @ 160 MHz | 32-bit ARM Cortex-M4 @ 100 MHz | Dual-Core ARM Cortex-M0+ @ 133 MHz |
| **Weight (Board only)** | ~5.0 g | **1.2 g** | ~3.0 g (All-in-One) | ~1.5 g |
| **Built-in Wireless** | None (Needs ~1.5g RX) | **ESP-NOW & Wi-Fi Web App** | **ExpressLRS (ELRS 2.4GHz)** | None (Needs ~0.5g RX) |
| **Control Latency** | 20 ms (PPM frame) | **2 – 4 ms (ESP-NOW / Web)** | **2 ms (ELRS 250Hz)** | 2 – 3 ms (CRSF UART) |
| **Servo Refresh Rate** | 50 Hz (standard Servo.h) | **250 Hz – 333 Hz (LEDC)** | **333 Hz (Timer PWM)** | **250 Hz – 333 Hz (PIO)** |
| **Active Stabilization** | None (Software open-loop) | Optional via I2C MPU6050 | **Integrated 6-Axis Gyro (BMI270)** | Optional via I2C IMU |
| **Brownout Sensitivity** | High (Weak LDO regulator) | Medium (Low drop-out) | Ultra-low (High-efficiency switching BEC) | Low |
| **Total System Weight** | ~6.5 g (with RX + cables) | **1.2 g (No RX needed!)** | **3.0 g (All integrated)** | ~2.0 g (with micro RX) |

---

## 3. Key Innovations from Max Imagination's ESP-FLUTTER (Video Breakdown)

The YouTube video **"Make an ESP32 RC Bionic Butterfly That Flies | ESP-FLUTTER" by Max Imagination** ([https://youtu.be/5Le4I-1Pf9I](https://youtu.be/5Le4I-1Pf9I)) presents several revolutionary breakthroughs that elevate the bionic butterfly from a simple hobby toy to a high-performance biomimetic robot.

### 1. Differential-Amplitude Steering
* **Previous Technique (S-DiY / TruongVanSu91):** Shifted the mean servo neutral angle (`center ± offset`). Both wings flapped with identical amplitude, which often caused wing stall, altitude drop, or violent roll snaps.
* **Max Imagination's Innovation:** The stroke depth is dynamically varied between wings:
  $$\text{Wing Delta} = \text{SteerNorm} \times \text{BaseAmplitude} \times 0.45$$
  $$\text{Left Amplitude} = \text{BaseAmplitude} + \text{Wing Delta}$$
  $$\text{Right Amplitude} = \text{BaseAmplitude} - \text{Wing Delta}$$
  When steering right, the left wing flaps through a larger arc while the right wing flaps through a shallower arc. This delivers immediate, banked turning authority without wing flutter or stalling!

### 2. Automatic Turn Lift Boost
* When an ornithopter banks, aerodynamic lift tilts sideways, causing the aircraft to drop.
* Max added an automatic mathematical boost:
  $$\text{TurnLiftBoost} = 1.0 + (|\text{SteerNorm}| \times 0.15)$$
  At full stick deflection, flap amplitude automatically surges by **+15%**, providing the extra vertical thrust needed to hold altitude in tight turns.

### 3. Smartphone Web App Controller (Zero Added RX Weight!)
* The ESP32-C3 hosts its own Wi-Fi Access Point (`Butterfly_RC`).
* By opening `http://192.168.4.1` on any iPhone, Android, or laptop, pilots get:
  * Full virtual on-screen touch joystick
  * Vertical throttle slider
  * Live battery voltage telemetry
  * LED lighting mode switcher (Off / Breath / Strobe)
  * Eliminates the external RC receiver completely, shaving **5.3 grams** (~10% of total aircraft weight)!

### 4. Night-Flight Flexible LED Filaments
* Integrates ultra-thin **flexible 3V LED filaments** (the flexible glowing wire filaments from decorative Edison bulbs, ~130mm or 300mm long, weighing under 0.3g).
* The filament is bonded directly along the carbon fiber leading edge or wing ribs, producing a magical bioluminescent glow in dusk or dark indoor environments!

---

## 4. Complete Bill of Materials (Procurement Inventory)

### A. Flight Electronics (Choose Track 1 or Track 2)

#### Track 1: Arduino Pro Micro (Classic RC Transmitter)
* [ ] **1× Arduino Pro Micro (ATmega32U4):** 5V / 16MHz, USB-C.
* [ ] **1× Micro RC Receiver:** **FlySky FS-A8S** (PPM/i-Bus, 1.2g) or **Radiomaster RP1 / BetaFPV ELRS** (0.5g).

#### Track 2: Seeed Studio XIAO ESP32-C3 (Smartphone Web App + ESP-NOW + ELRS) - *Recommended*
* [ ] **1× Seeed Studio XIAO ESP32-C3:** 32-bit RISC-V @ 160MHz, built-in Wi-Fi & Bluetooth, 1.2g.
* [ ] **Resistors for Battery Divider:** 1× 49kΩ (or 47kΩ) and 1× 18kΩ (0805 SMD or 1/8W through-hole) connected to D2.

### B. Common Actuation & Power Components
* [ ] **2× Micro Flapping Servos:**
  * *Option A (High-Voltage Direct 2S - Best):* **C-Torque 9g HV Bionic Butterfly Servos** or **PTK 7465 MG-D HV** (Digital Metal Gear, 0.06s/60°, 2.5 kg·cm torque).
  * *Option B (Standard 5V-6V):* **EMAX ES08MD II** or **TowerPro MG90S** (requires 5V/6V 3A Micro BEC).
* [ ] **1× Power Decoupling Capacitor:** **470µF – 1000µF Low-ESR Electrolytic Capacitor (10V or 16V)**.
* [ ] **2–3× Flight Batteries:** **2S 7.4V 250mAh – 350mAh LiPo (30C–75C)** (e.g. Tattu 300mAh 2S 75C or GNB 2S 350mAh) with XT30 or JST-PH 2.0 connector.
* [ ] **1× Micro Power Switch:** Ultra-light SPDT slide switch.
* [ ] **Flexible LED Filaments (Night Flight):** 2× 3V 130mm or 300mm ultra-flexible LED filaments (Ice Blue, Warm White, or Pink) + 33Ω current limiting resistor.

### C. Airframe & Wing Materials
* [ ] **1× 3D Printed Central Chassis:** `FC Mount V5.STL` (PETG).
* [ ] **2× 3D Printed Wing Drive Arms:** `ARMV5.STL` (1× normal, 1× mirrored; 100% solid infill).
* [ ] **Solid Carbon Fiber Rods:**
  * **1.5 mm OD × 500 mm:** 1 rod (cut into 2× 200 mm leading edge spars).
  * **1.0 mm OD × 1000 mm:** 1–2 rods (cut into 4× 150 mm forewing ribs).
  * **0.8 mm OD × 1000 mm:** 1 rod (cut into 4× 140 mm hindwing and flexible struts).
* [ ] **Wing Membrane:** **0.5 oz (20–30 g/m²) Ripstop Nylon** or **15–20 µm Mylar / BoPET Film** (~0.5 m²).
* [ ] **Lashing Thread:** **Braided Kevlar thread (30–50 lb)** or **0.15 mm Dyneema / Spectra fishing line**.
* [ ] **Adhesives:** Thin CA, Medium CA, CA Kicker/Activator spray, and 3M 77 spray adhesive or 3mm double-sided tissue tape.

---

## 5. Phase 1: 3D Printing & Component Preparation

```
   [ FC Mount V5 ]           [ ARMV5 - Right ]          [ ARMV5 - Left ]
   (Central Chassis)         (Print as-is)              (Mirror in slicer!)
   Material: PETG (20%)      Material: PETG (100%)      Material: PETG (100%)
```

1. **Chassis (`FC Mount V5.STL`):** Print 1× in PETG at 0.16mm or 0.20mm layer height, 20% gyroid infill. Weight: ~5.5g.
2. **Wing Hub Arms (`ARMV5.STL`):**
   * **CRITICAL:** The wing hub arm is chiral.
     * **Right Arm:** Load `ARMV5.STL` and slice as-is.
     * **Left Arm:** Load `ARMV5.STL`, right-click -> **Mirror along X or Y axis**, then slice.
   * **Infill:** **100% Solid** (or 5+ perimeters). Weight: ~3.0g each.

---

## 6. Phase 2: Wing Skeleton, Glowing LEDs & Aerodynamic Skinning

```
                 Leading Edge Spar (1.5mm Carbon Rod) + Flexible LED Filament
             ===================================================================\
            /  Forewing Rib 1 (1.0mm)                                            \
           / ------------------------------------------\                          \
   [HUB] =<    Forewing Rib 2 (1.0mm)                   \                          ) Forewing
     ||    \ -------------------------------------\      )                        /
     ||     \  Forewing Rib 3 (1.0mm)              )    /                        /
     ||      \ ------------------------------\    /    /                        /
     ||       \                               \  /    /                        /
     ||        \ Hindwing Spar (0.8mm)         \/    /                        /
     ||         \ --------------------\             /                        /
     ||          \ Diagonal (0.8mm)    ) Hindwing  /                        /
     ||           \ ------------------/           /                        /
                   \                             /                        /
                    \---------------------------/------------------------/
```

1. **Template:** Print [`wing design.jpeg`](./wing%20design.jpeg) on A4 paper scaled to fit portrait orientation. Fasten to a flat building board and cover with wax paper or cling film.
2. **Cutting Rods:** Cut the 1.5mm leading edge spar (~190–200mm), 1.0mm forewing ribs (~120–160mm), and 0.8mm hindwing spars (~110–150mm).
3. **Lashing & Glue:** Insert rods into `ARMV5` sockets with Medium CA. Wrap junctions with 4–6 turns of Kevlar thread, wick with Thin CA, and mist with kicker.
4. **Installing Flexible LED Filaments (Optional for Night Flying):**
   * Lay a 130mm 3V flexible LED filament along the 1.5mm leading edge carbon rod.
   * Secure with tiny drops of Thin CA every 30mm.
   * Solder ultra-thin 32 AWG enamel wire to the filament pads and route back to the central chassis.
5. **Skinning:** Lightly spray the skeleton with 3M 77 adhesive, lay Ripstop Nylon or Mylar film smoothly from leading edge to trailing edge, and trim excess with a hot soldering iron tip or #11 knife.

---

## 7. Phase 3: Servo Mechanics & Chassis Assembly

1. **Center Servos:** Power servos with 1500 µs pulse from your flight controller before attaching horns.
2. **Mount Horns:** Install single-arm horns perpendicular to the servo body at 90° neutral.
3. **Chassis Installation:** Mount servos into `FC Mount V5` using M2 self-tapping screws.
4. **Attach Arms:** Secure the prepared left and right wing assemblies to the servo horns. Manually cycle wings through ±65° to verify zero mechanical binding.

---

## 8. Phase 4: Electrical Wiring & Power Conditioning

### Wiring Diagram: Seeed Studio XIAO ESP32-C3 (Track 2)

```
                  +-----------------------------------+
                  |   2S LiPo Battery (7.4V - 8.4V)  |
                  +-----------------+-----------------+
                                    |
                            [ Micro SPDT Switch ]
                                    |
               +--------------------+--------------------+
               | (+) Red                                 | (-) Black
               |                                         |
     +---------+---------+                     +---------+---------+
     |   Low-ESR Cap     |                     |                   |
     |  470uF - 1000uF   |                     |                   |
     +---------+---------+                     |                   |
               |                               |                   |
               +--------------------+          |                   |
               | (If HV Servos)     |          |                   |
          +----+----+          +----+----+     |                   |
          | Servos  |          | 49k Res |     |                   |
          | V+ Rail |          +----+----+     |                   |
          +----+----+               |          |                   |
               |               (D2 Pin)        |                   |
               |                    |          |                   |
               |               +----+----+     |                   |
               |               | 18k Res |     |                   |
               |               +----+----+     |                   |
               |                    |          |                   |
               +--------------------+----------+---------+---------+
               |                                         |
       +-------+-------+                         +-------+-------+
       | XIAO '5V' Pin |                         | XIAO 'GND' Pin|
       +-------+-------+                         +-------+-------+

     SIGNAL CONNECTIONS:
     * Left Servo Signal  ---------------> XIAO Pin D0
     * Right Servo Signal ---------------> XIAO Pin D1
     * Battery Voltage Divider ----------> XIAO Pin D2
     * LED Filament Gate (via 33Ω res) --> XIAO Pin D3
```

---

## 9. Phase 5: Firmware Configuration & Flashing

### Option A: Arduino Pro Micro (`Bionic_Butterfly_Enhanced.ino`)
1. Open [`Bionic_Butterfly_Enhanced.ino`](./Bionic_Butterfly_Enhanced.ino) in Arduino IDE.
2. Select board: **Arduino Leonardo / Micro**.
3. Verify `#define USE_UPDATED_PINS 1` (Pin 2 PPM, Pin 4 Left Servo, Pin 5 Right Servo).
4. Upload and open Serial Monitor at 115200 baud.

### Option B: Seeed Studio XIAO ESP32-C3 (`ESP32_C3_Firmware/ESP32_Bionic_Butterfly.ino`)
1. Install **ESP32 by Espressif** board package in Arduino IDE.
2. Install `ESP32Servo` library.
3. Select board: **Seeed Studio XIAO ESP32C3**.
4. Upload [`ESP32_Bionic_Butterfly.ino`](./ESP32_C3_Firmware/ESP32_Bionic_Butterfly.ino).

---

## 10. Phase 6: Flying via Smartphone Web App or RC Transmitter

### Flying via Smartphone Web App (ESP32-C3)
1. Power on the butterfly.
2. Open your phone's Wi-Fi settings and connect to **`Butterfly_RC`** (Password: **`12345678`**).
3. Open Safari or Chrome and navigate to:
   ```
   http://192.168.4.1
   ```
4. **Controls:**
   * **Left Slider:** Proportional throttle (0% to 100%).
   * **Right Touch Joystick:** Steering (left/right differential flapping) and Pitch (up/down bias).
   * **LED Button:** Toggles Off, Breathing, and Night Strobe modes.
   * **Arm/Disarm Button:** Instantly centers and locks wings for safe handling.

### Flying via RC Transmitter (FlySky FS-i6 / EdgeTX)
* **CH1 (Aileron):** Differential-amplitude steering.
* **CH2 (Elevator):** Pitch bias.
* **CH3 (Throttle):** Flap frequency and amplitude.
* **CH5 (Pot):** Steering fine trim.
* **CH7 (Switch):** Perch flutter mode.
* **CH8 (Switch):** Night LED mode toggle.

---

## 11. Phase 7: Pre-Flight Checks, CG Balancing & Maiden Flight

1. **Center of Gravity (CG):** Balance point must be at **35% to 40% of the mean wing chord** from the leading edge root. Slide battery forward or backward until level.
2. **Pre-Flight Function Check:**
   * Verify wings hold 1500µs horizontal neutral at zero throttle.
   * Move steering stick right $\rightarrow$ Left wing amplitude increases, Right wing amplitude decreases.
   * Verify Failsafe: Switching off transmitter smoothly glides wings into a +18° upward "V" dihedral.
3. **Maiden Flight:** Launch indoors or in dead calm wind. Advance throttle to 65%, gently push forward level with the horizon, and fly using small, gentle stick inputs.

---

## 12. Troubleshooting & Flight Tuning

| Symptom | Probable Cause | Corrective Action |
| :--- | :--- | :--- |
| **Butterfly flaps violently but doesn't climb** | Wing membrane is too stiff, or rods lack flex | Verify leading edge is 1.5mm and ribs are 0.8mm–1.0mm. Ensure membrane has slight trailing-edge billow. |
| **Microcontroller reboots during flapping** | Inductive voltage sag / brownout | Solder a 470µF–1000µF Low-ESR capacitor directly across the servo power rail. |
| **Butterfly drops altitude during turns** | Missing turn lift compensation | In firmware, verify `TURN_LIFT_BOOST` is enabled (automatically boosts flap amplitude by +15% during steering). |
| **Butterfly rolls hard to one side** | Asymmetric wing construction or servo centering | Check wing weights on a precision scale. Trim out differential angle using CH5 pot or web app trim slider. |
| **Servos buzz loudly and run hot at full stroke** | Servo travel clipping against physical limit | Increase `SERVO_US_MIN` and decrease `SERVO_US_MAX` in firmware, or reduce `MAX_FLAP_AMP_DEG`. |
| **Butterfly dives immediately upon launch** | Nose-heavy (CG too far forward) or negative pitch bias | Slide LiPo backward toward wing trailing edge, or increase pitch trim via CH6 pot / web app. |
| **Butterfly stalls, wobbles, and falls backward** | Tail-heavy (CG too far aft) | Slide LiPo forward toward the nose until the model balances at 35%–40% chord. |
