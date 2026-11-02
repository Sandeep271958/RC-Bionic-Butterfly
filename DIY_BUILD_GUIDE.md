# RC Bionic Butterfly: Complete Scratch-Build DIY Manual & Engineering Guide

---

## Table of Contents
1. [Introduction & Aerodynamic Principles](#1-introduction--aerodynamic-principles)
2. [Microcontroller Upgrade Research & Flight Response Analysis](#2-microcontroller-upgrade-research--flight-response-analysis)
3. [Complete Bill of Materials (Procurement Inventory)](#3-complete-bill-of-materials-procurement-inventory)
4. [Phase 1: 3D Printing & Component Preparation](#4-phase-1-3d-printing--component-preparation)
5. [Phase 2: Wing Skeleton & Aerodynamic Skinning](#5-phase-2-wing-skeleton--aerodynamic-skinning)
6. [Phase 3: Servo Mechanics & Chassis Assembly](#6-phase-3-servo-mechanics--chassis-assembly)
7. [Phase 4: Electrical Wiring & Power Conditioning](#7-phase-4-electrical-wiring--power-conditioning)
8. [Phase 5: Firmware Configuration & Flashing](#8-phase-5-firmware-configuration--flashing)
9. [Phase 6: Radio Transmitter Setup & Channel Calibration](#9-phase-6-radio-transmitter-setup--channel-calibration)
10. [Phase 7: Pre-Flight Checks, CG Balancing & Maiden Flight](#10-phase-7-pre-flight-checks-cg-balancing--maiden-flight)
11. [Troubleshooting & Flight Tuning](#11-troubleshooting--flight-tuning)

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
2. **Steering (Roll / Bank):** Achieved through differential wing dihedral. By biasing the left servo slightly upward and the right servo downward, asymmetric aerodynamic drag and lift roll the butterfly smoothly in the commanded direction.
3. **Pitch (Climb / Dive):** Achieved through symmetric wing bias. Shifting the neutral stroke angle of both wings upward shifts the center of aerodynamic pressure forward relative to the Center of Gravity (CG), pitching the nose up to climb. Shifting them downward pitches the nose down.

---

## 2. Microcontroller Upgrade Research & Flight Response Analysis

The baseline build uses an **Arduino Pro Micro (ATmega32U4, 8-bit AVR @ 16MHz)** with an external PPM receiver. While functional and accessible, replacing the microcontroller can substantially improve flight responsiveness, weight, and stability.

### Deep Comparative Analysis

| Feature / Metric | Stock: Arduino Pro Micro (ATmega32U4) | Upgrade 1: Seeed Studio XIAO ESP32-C3 | Upgrade 2: FPV Micro AIO (STM32F411 + Gyro + ELRS) | Upgrade 3: Waveshare RP2040-Zero |
| :--- | :--- | :--- | :--- | :--- |
| **CPU Architecture** | 8-bit AVR @ 16 MHz | 32-bit RISC-V @ 160 MHz | 32-bit ARM Cortex-M4 @ 100 MHz | Dual-Core ARM Cortex-M0+ @ 133 MHz |
| **Weight (Board only)** | ~5.0 g | **1.2 g** | ~3.0 g (All-in-One) | ~1.5 g |
| **Built-in Wireless** | None (Needs ~1.5g RX) | **ESP-NOW (2.4GHz Direct)** | **ExpressLRS (ELRS 2.4GHz)** | None (Needs ~0.5g RX) |
| **Control Latency** | 20 ms (PPM frame) | **2 – 4 ms (ESP-NOW packet)** | **2 ms (ELRS 250Hz)** | 2 – 3 ms (CRSF UART) |
| **Servo Refresh Rate** | 50 Hz (standard Servo.h) | **250 Hz – 333 Hz (LEDC)** | **333 Hz (Timer PWM)** | **250 Hz – 333 Hz (PIO)** |
| **Active Stabilization** | None (Software open-loop) | Optional via I2C MPU6050 | **Integrated 6-Axis Gyro (BMI270)** | Optional via I2C IMU |
| **Brownout Sensitivity** | High (Weak LDO regulator) | Medium (Low drop-out) | Ultra-low (High-efficiency switching BEC) | Low |
| **Total System Weight** | ~6.5 g (with RX + cables) | **1.2 g (No RX needed!)** | **3.0 g (All integrated)** | ~2.0 g (with micro RX) |

### Key Improvements from Upgrading

#### 1. Seeed Studio XIAO ESP32-C3 (Best DIY Upgrade for Weight & Speed)
* **Eliminates the Separate Receiver:** Using the ESP-NOW protocol, the Xiao ESP32-C3 receives control packets directly from an ESP32 transmitter module or handheld remote. This eliminates the external receiver PCB, antenna, and wiring harness, shaving **5.3 grams** off the aircraft. On a 55g butterfly, this represents an immediate **~10% weight reduction**!
* **Ultra-Low Latency:** Latency drops from 20ms (PPM) down to **2–4 milliseconds**, providing instantaneous control authority.
* **High-Speed PWM:** Generates hardware PWM pulses at 250Hz–333Hz, enabling digital servos to react 5x faster with tighter holding torque.

#### 2. FPV Micro AIO (STM32F411 with Integrated Gyro & ELRS) (The Ultimate Flight Experience)
* **Active Gyroscopic Damping:** Real flapping butterflies are buffeted by wind and cyclic flapping torque. An onboard 6-axis gyroscope (BMI270) running an active PID damping loop automatically adjusts servo angles to counter gusts in real time. The butterfly stops wobbling and tracks straight as if flying on invisible rails.
* **ExpressLRS (ELRS) Integration:** Long-range, zero-jitter 250Hz/500Hz digital radio link built right onto the board.

> **Recommendation:** 
> * **Phase 1 (Today):** Build with the **Arduino Pro Micro** using [`Bionic_Butterfly_Enhanced.ino`](./Bionic_Butterfly_Enhanced.ino) to get your model flying quickly.
> * **Phase 2 (Performance Upgrade):** Swap in a **Seeed Studio XIAO ESP32-C3** to eliminate the external receiver and cut 5g of dead weight, or an **F4 AIO with Gyro** for active auto-leveling stabilization.

---

## 3. Complete Bill of Materials (Procurement Inventory)

### A. Electronics & Actuation
* [ ] **1× Arduino Pro Micro (ATmega32U4):** 5V / 16MHz, USB-C (recommended) or Micro-USB.
* [ ] **2× Micro Flapping Servos:**
  * *Option A (High-Voltage Direct 2S - Best):* **C-Torque 9g HV Bionic Butterfly Servos** or **PTK 7465 MG-D HV** (Digital Metal Gear, 0.06s/60°, 2.5 kg·cm torque).
  * *Option B (Standard 5V-6V):* **EMAX ES08MD II** or **TowerPro MG90S** (requires 5V/6V 3A Micro BEC).
* [ ] **1× Micro RC Receiver:** **FlySky FS-A8S** (PPM/i-Bus, 1.2g) or **Radiomaster RP1 / BetaFPV ELRS** (0.5g).
* [ ] **1× Power Decoupling Capacitor:** **470µF – 1000µF Low-ESR Electrolytic Capacitor (10V or 16V)**.
* [ ] **2–3× Flight Batteries:** **2S 7.4V 250mAh – 350mAh LiPo (30C–75C)** (e.g. Tattu 300mAh 2S 75C or GNB 2S 350mAh) with XT30 or JST-PH 2.0 connector.
* [ ] **1× Micro Power Switch:** Ultra-light SPDT slide switch (or direct battery plug).
* [ ] **1× Micro 5V/6V 3A BEC:** *(Only needed if using standard 5V-6V servos)*.

### B. Airframe & Wing Structure
* [ ] **1× 3D Printed Central Chassis:** `FC Mount V5.STL` (PETG or LW-PLA).
* [ ] **2× 3D Printed Wing Drive Arms:** `ARMV5.STL` (1× normal, 1× mirrored in slicer; 100% solid infill).
* [ ] **Carbon Fiber Solid Rods:**
  * **1.5 mm OD × 500 mm:** 1 rod (cut into 2× 200 mm leading edge spars).
  * **1.0 mm OD × 1000 mm:** 1–2 rods (cut into 4× 150 mm forewing ribs).
  * **0.8 mm OD × 1000 mm:** 1 rod (cut into 4× 140 mm hindwing and flexible struts).
* [ ] **Wing Membrane:** **0.5 oz (20–30 g/m²) Ripstop Nylon** or **15–20 µm Mylar / BoPET Film** (~0.5 m²).
* [ ] **Lashing Thread:** **Braided Kevlar thread (30–50 lb)** or **0.15 mm Dyneema / Spectra fishing line**.
* [ ] **Hardware:** 4× M2 × 6mm or M2 × 8mm micro self-tapping screws for servo mounting.

### C. Adhesives & Workshop Consumables
* [ ] **Thin Cyanoacrylate (CA) Glue** (wicking into thread lashings).
* [ ] **Medium Cyanoacrylate (CA) Glue** (bonding carbon rods into 3D-printed arm sockets).
* [ ] **CA Accelerator / Kicker Spray** (instant curing).
* [ ] **3M 77 Multipurpose Spray Adhesive** or **3 mm high-tack double-sided tissue tape** (attaching membrane to carbon rods).
* [ ] **28 AWG / 30 AWG Ultra-flexible Silicone Wire** (Black, Red, Yellow).
* [ ] **Heat shrink tubing assortment** (1.5mm, 2.5mm, 4.0mm).

---

## 4. Phase 1: 3D Printing & Component Preparation

```
   +-------------------------------------------------------+
   |                  3D PRINTING OVERVIEW                 |
   |                                                       |
   |  [ FC Mount V5 ]           [ ARMV5 - Right ]          |
   |  (Central Chassis)         (Print as-is)              |
   |  Material: PETG            Material: PETG (100%)      |
   |                                                       |
   |                            [ ARMV5 - Left ]           |
   |                            (Mirror in slicer!)        |
   |                            Material: PETG (100%)      |
   +-------------------------------------------------------+
```

### Slicer Configuration
1. **Chassis (`FC Mount V5.STL`):**
   * **Material:** PETG (recommended for impact toughness and vibration damping) or Tough PLA.
   * **Layer Height:** 0.16 mm or 0.20 mm.
   * **Walls / Perimeters:** 2 to 3.
   * **Infill:** 15% – 20% Gyroid.
   * **Target Weight:** ~5.0 g – 5.5 g.
2. **Wing Hub Arms (`ARMV5.STL`):**
   * **CRITICAL:** The wing hub arm is **asymmetric**.
     * **Right Arm:** Load `ARMV5.STL` and slice as-is.
     * **Left Arm:** Load `ARMV5.STL`, right-click in your slicer (Cura, PrusaSlicer, Bambu Studio, OrcaSlicer) -> **Mirror along X or Y axis**, then slice.
   * **Infill:** **100% Solid** (or 5+ perimeters). This part absorbs cyclic bending stress on every flap.
   * **Target Weight:** ~3.0 g per arm.
3. **Post-Print Cleanup:**
   * Test-fit a 1.5mm carbon fiber rod into the leading edge socket of each arm. If snug, lightly clean with a 1.5mm drill bit spun by hand.
   * Clear any stringing inside the servo pockets of `FC Mount V5`.

---

## 5. Phase 2: Wing Skeleton & Aerodynamic Skinning

```
                 Leading Edge Spar (1.5mm Carbon Rod)
             ====================================================\
            /  Forewing Rib 1 (1.0mm)                             \
           / ------------------------------------------\           \
   [HUB] =<    Forewing Rib 2 (1.0mm)                   \           ) Forewing
     ||    \ -------------------------------------\      )         /
     ||     \  Forewing Rib 3 (1.0mm)              )    /         /
     ||      \ ------------------------------\    /    /         /
     ||       \                               \  /    /         /
     ||        \ Hindwing Spar (0.8mm)         \/    /         /
     ||         \ --------------------\             /         /
     ||          \ Diagonal (0.8mm)    ) Hindwing  /         /
     ||           \ ------------------/           /         /
                   \                             /         /
                    \---------------------------/---------/
```

### Step 1: Prepare the Wing Template
1. Open [`wing design.jpeg`](./wing%20design.jpeg).
2. Print the image on standard A4 paper scaled to fit portrait orientation (~260 mm height × 175 mm width per wing).
3. Secure the paper template to a flat, rigid building board (corkboard, balsa board, or dense foam board).
4. Lay a sheet of clear plastic food wrap or wax paper over the template to prevent glue from adhering to the paper.

### Step 2: Cut & Fit Carbon Fiber Rods
1. **Leading Edge Spar:** Cut a 1.5 mm carbon fiber rod to follow the upper outer curve (~190 mm – 200 mm). Insert the root end into the upper socket of the 3D-printed `ARMV5` arm.
2. **Forewing Radiating Ribs:** Cut four 1.0 mm carbon rods matching the lines radiating from the root hub toward the wing margin (~120 mm to 160 mm).
3. **Hindwing Spar & Struts:** Cut 0.8 mm carbon rods for the lower wing outline and cross-braces (~110 mm to 150 mm).
4. Lightly scuff the rod tips and root ends with 400-grit sandpaper to promote adhesive bonding.

### Step 3: Secure the Hub & Lash Joints
1. Insert the carbon rods into the matching sockets of the `ARMV5` arm using **Medium CA glue**.
2. At all junctions where carbon rods intersect or meet the hub, tightly wrap **4 to 6 turns of braided Kevlar thread** (or Dyneema line).
3. Apply a single drop of **Thin CA glue** onto the thread; capillary action will instantly saturate the fibers.
4. Mist with **CA Accelerator / Kicker** for an instantaneous, unbreakable joint.
5. Repeat for the mirrored left wing arm.

### Step 4: Skin the Wing Membrane
1. Cut a piece of **0.5 oz Ripstop Nylon** or **15 µm Mylar film** roughly 20 mm larger than the wing outline.
2. If using **3M 77 Spray Adhesive**:
   * Lightly mist the back of the carbon fiber skeleton from 30 cm away (avoid heavy spraying to keep weight minimal).
   * Wait 60 seconds until tacky.
   * Carefully lay the membrane down onto the skeleton, working from the leading edge backward toward the trailing edge, smoothing out all wrinkles.
3. If using **3mm Double-Sided Tape**:
   * Apply tape along the carbon rods, peel backing, and roll the membrane smoothly into place.
4. **Trimming:**
   * Trim excess material along the perimeter leaving a 2 mm margin using a sharp #11 hobby knife.
   * If using Ripstop Nylon, run the tip of a fine hot soldering iron along the perimeter to heat-seal the edges against fraying.

---

## 6. Phase 3: Servo Mechanics & Chassis Assembly

```
         +---------------------------------------------+
         |            [ FC Mount V5 Chassis ]          |
         |                                             |
         |   [ Left Servo ]           [ Right Servo ]   |
         |   (Spline Upward)          (Spline Upward)  |
         |         |                         |         |
         |   [ Horn at 90° ]           [ Horn at 90° ] |
         |         |                         |         |
         |   [ ARMV5 Left ]            [ ARMV5 Right ] |
         +---------------------------------------------+
```

### Step 1: Electrically Center the Servos
1. **Do not install servo horns yet.**
2. Connect your Arduino Pro Micro or a servo tester to send a neutral **1500 µs pulse**.
3. Power the servos so they seek their true electrical center.
4. Mount the single-arm servo horn onto the servo output spline as close to a perpendicular **90° neutral angle** as the teeth permit.
5. Secure the horn with its center retention screw.

### Step 2: Install Servos into Chassis
1. Slide the two micro servos into the designated pockets of `FC Mount V5`.
2. Secure each servo using two M2 × 6mm or M2 × 8mm self-tapping micro screws through the mounting tabs.
3. Ensure both servo output splines are aligned along the same horizontal rotational axis.

### Step 3: Attach Wing Drive Arms
1. Mount the prepared wing assemblies (`ARMV5 Left` and `ARMV5 Right`) onto the servo horns.
2. Fasten them using M2 hardware or tie-wire through the horn mounting holes.
3. Manually cycle the wings gently through their full ±65° range to confirm zero mechanical binding against the chassis.

---

## 7. Phase 4: Electrical Wiring & Power Conditioning

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
               |                    |          |                   |
        (If HV Servos)       (If 5V-6V)        |                   |
               |            [5V/6V BEC]        |                   |
               |                    |          |                   |
          +----+----+               |     +----+----+              |
          | Servos  |               |     | Servos  |              |
          | V+ Rail |               |     | GND Rail|              |
          +----+----+               |     +----+----+              |
               |                    |          |                   |
               +-------+------------+          +---------+---------+
                       |                                 |
                       | (8.4V or BEC 5V)                |
              +--------+--------+                        |
              | Pro Micro 'RAW' |                        |
              |    (or VCC)     |                        |
              +--------+--------+                        |
                       |                                 |
              +--------+--------+               +--------+--------+
              | Pro Micro 'VCC' |               | Pro Micro 'GND' |
              +--------+--------+               +--------+--------+
                       | 5.0V Regulated                  | Common GND
                       |                                 |
               +-------+-------+                 +-------+-------+
               |  FS-A8S VCC   |                 |  FS-A8S GND   |
               +---------------+                 +---------------+

     SIGNAL WIRING:
     * FS-A8S PPM Signal (White/Yellow) ----> Arduino Pin 2 (INT1)
     * Left Servo Signal (Orange/Yellow) ---> Arduino Pin 4 (PWM)
     * Right Servo Signal (Orange/Yellow) --> Arduino Pin 5 (PWM)
     * Status LED (Optional) ---------------> Arduino Pin 6 (via 220Ω resistor)
```

> [!CAUTION]
> **Brownout & Voltage Spike Protection:**
> Flapping wing servos generate high inductive back-EMF spikes when changing stroke directions.
> * Always solder a **470µF to 1000µF Low-ESR capacitor** directly across the main power bus near the servos.
> * If using standard 5V/6V servos (like MG90S or ES08MD), **do not feed them 8.4V directly**. Use a lightweight 5V/6V 3A Micro BEC.
> * If using 8.4V HV servos (like C-Torque 9g HV or PTK 7465 MG-D), connect them directly to the battery rail with the capacitor in parallel.

---

## 8. Phase 5: Firmware Configuration & Flashing

The firmware is located at [`Bionic_Butterfly_Enhanced.ino`](./Bionic_Butterfly_Enhanced.ino).

### Flashing Procedure
1. Open **Arduino IDE** (version 1.8.19 or 2.x).
2. Install the required libraries via **Sketch -> Include Library -> Manage Libraries...**:
   * `Servo` (built-in standard library)
   * `PPMReader` by Dimitar Kovachev
3. Select board: **Tools -> Board -> Arduino AVR Boards -> Arduino Leonardo / Micro** (or *SparkFun Pro Micro 5V/16MHz*).
4. Select the matching serial port under **Tools -> Port**.
5. Check lines 28–36 of [`Bionic_Butterfly_Enhanced.ino`](./Bionic_Butterfly_Enhanced.ino):
   ```cpp
   #define USE_UPDATED_PINS 1
   ```
   * Set `USE_UPDATED_PINS 1` for **Pin 2 (PPM), Pin 4 (Left Servo), Pin 5 (Right Servo)**.
   * Set `USE_UPDATED_PINS 0` if wired according to the legacy diagram (Pin 3 & Pin 10).
6. Click **Upload**.
7. Open **Serial Monitor** at **115200 baud** to verify startup telemetry:
   ```
   ==================================================
     RC BIONIC BUTTERFLY CONTROLLER v2.0 READY      
     Left Pin: 4 | Right Pin: 5
   ==================================================
   ```

---

## 9. Phase 6: Radio Transmitter Setup & Channel Calibration

### Channel Assignment (FlySky FS-i6 / FS-i6X / EdgeTX)
* **Channel 1 (Aileron / Roll):** Wing steering trim (differential roll).
* **Channel 2 (Elevator / Pitch):** Wing pitch bias (symmetric tilt up/down).
* **Channel 3 (Throttle):** Flapping frequency and amplitude modulation.
* **Channel 5 (Aux Pot VRA):** Steering fine trim pot.
* **Channel 6 (Aux Pot VRB):** Pitch trim bias pot.
* **Channel 7 (Switch SWA/SWC):** Perch mode enable switch (gentle fluttering when stationary).
* **Channel 8 (Switch SWD):** Flight Mode toggle:
  * `LOW` = **Biomimetic Asymmetric Mode** (High-lift, faster downstroke).
  * `HIGH` = **Pure Sine Wave Mode** (Smooth, classic sine).

### Transmitter Setup Settings
1. Set Model Type: **Airplane / Glider (Acro)**.
2. Endpoints (E.POINT): Set all channels to **100%** (1000 µs to 2000 µs).
3. Dual Rates & Expo:
   * **CH1 (Aileron):** Rate 100%, Expo +30% (softens stick feel around center).
   * **CH2 (Elevator):** Rate 100%, Expo +25%.

---

## 10. Phase 7: Pre-Flight Checks, CG Balancing & Maiden Flight

```
                 <--- 35% - 40% Chord --->
                 +-----------------------+
   [Nose] =======|==========[ CG ]=======|=======> [Tail / Trailing Edge]
                 +-----------------------+
                     ^ Balance Point ^
```

### Step 1: Center of Gravity (CG) Balancing
* The Center of Gravity must sit at **35% to 40% of the mean wing chord** measured from the leading edge root.
* Support the butterfly gently on two fingertips under the wing root arm at the 35% chord line.
* If the tail drops: Slide the 2S LiPo battery forward toward the nose.
* If the nose drops: Slide the LiPo battery backward toward the servos.
* Secure the battery firmly with micro velcro and a lightweight silicone band.

### Step 2: Bench Function Check
1. Turn on the transmitter with throttle at zero.
2. Power on the butterfly. The status LED should light up solid and the wings should hold horizontal neutral (1500 µs).
3. **Throttle Test:** Advance throttle slowly to 20%. The wings should begin a gentle, smooth flutter. At 80% throttle, they should flap with high frequency and full amplitude without gear chatter.
4. **Steering Test:** Push the right stick right. The left wing should bias upward and right wing downward.
5. **Pitch Test:** Push the right stick forward (pitch down). Both wings should bias slightly downward.
6. **Failsafe Test:** At 30% throttle, switch off the transmitter. The wings must smoothly transition into an upward **"V" dihedral glide posture (approx +18°)** and stop flapping within 0.5 seconds.

### Step 3: Maiden Flight Procedure
1. Choose an indoor gymnasium, large hall, or a completely calm outdoor evening (zero wind is essential for initial trimming).
2. Hold the central chassis between thumb and index finger from underneath.
3. Advance throttle smoothly to **60%–70%**.
4. Gently launch the butterfly straight and level into the air with a smooth forward push (do not throw forcefully).
5. Use small, gentle stick inputs to trim straight-line flight using the Channel 5 pot.
6. To descend, reduce throttle smoothly to 15%–20%; to glide, cut throttle to zero so the wings enter the stable "V" dihedral glide.

---

## 11. Troubleshooting & Flight Tuning

| Symptom | Probable Cause | Corrective Action |
| :--- | :--- | :--- |
| **Butterfly flaps violently but doesn't climb** | Wing membrane is too stiff, or rods lack flex | Verify leading edge is 1.5mm and ribs are 0.8mm–1.0mm. Ensure membrane has slight trailing-edge billow. |
| **Microcontroller reboots during flapping** | Inductive voltage sag / brownout | Solder a 470µF–1000µF Low-ESR capacitor directly across the servo power rail. |
| **Butterfly rolls hard to one side** | Asymmetric wing construction or servo centering | Check wing weights on a precision scale. Trim out differential angle using CH5 pot or sub-trim. |
| **Servos buzz loudly and run hot at full stroke** | Servo travel clipping against physical limit | Increase `SERVO_US_MIN` and decrease `SERVO_US_MAX` in firmware, or reduce `MAX_FLAP_AMP_DEG`. |
| **Butterfly dives immediately upon launch** | Nose-heavy (CG too far forward) or negative pitch bias | Slide LiPo backward toward wing trailing edge, or increase pitch trim via CH6 pot. |
| **Butterfly stalls, wobbles, and falls backward** | Tail-heavy (CG too far aft) | Slide LiPo forward toward the nose until the model balances at 35%–40% chord. |
