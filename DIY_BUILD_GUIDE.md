# RC Bionic Butterfly: Master Scratch-Build Manual

An end-to-end, engineering-grade build guide for the **ESP32 Bionic RC Butterfly (ESP-FLUTTER Architecture)** featuring **Dual Control: Smartphone Web Cockpit & Traditional PPM RC Controller**.

---

## Table of Contents
1. [Project Overview & Core Specifications](#1-project-overview--core-specifications)
2. [Flight Dynamics & Aerodynamic Breakthroughs](#2-flight-dynamics--aerodynamic-breakthroughs)
3. [Repository Architecture & File Map](#3-repository-architecture--file-map)
4. [Complete Bill of Materials (Exact Procurement List)](#4-complete-bill-of-materials-exact-procurement-list)
5. [Phase 1: 3D Printing & Slicer Configuration](#5-phase-1-3d-printing--slicer-configuration)
6. [Phase 2: Wing Skeleton & Exact Carbon Fiber Cut List](#6-phase-2-wing-skeleton--exact-carbon-fiber-cut-list)
7. [Phase 3: Wing Skinning & Night-Flight Flexible LED Filaments](#7-phase-3-wing-skinning--night-flight-flexible-led-filaments)
8. [Phase 4: Servo Actuation & Spline Indexing](#8-phase-4-servo-actuation--spline-indexing)
9. [Phase 5: Electrical Wiring & Power Conditioning](#9-phase-5-electrical-wiring--power-conditioning)
10. [Phase 6: Firmware Setup & Flashing](#10-phase-6-firmware-setup--flashing)
11. [Phase 7: Dual Piloting: Smartphone Web App & PPM RC Controller](#11-phase-7-dual-piloting-smartphone-web-app--ppm-rc-controller)
12. [Phase 8: CG Balancing, Flight Trimming & Maiden Flight](#12-phase-8-cg-balancing-flight-trimming--maiden-flight)
13. [Troubleshooting & Diagnostics](#13-troubleshooting--diagnostics)

---

## 1. Project Overview & Core Specifications

* **Wingspan:** 390 mm – 410 mm (~15.5 inches)
* **Flying Weight (AUW):** ~65 g – 72 g
* **Flight Controller:** **Seeed Studio XIAO ESP32-C3** (1.2g, 32-bit RISC-V @ 160MHz)
* **Dual Control Links:**
  * **Interface 1:** Embedded HTML5 Touch Web Cockpit hosted on-board via Wi-Fi Soft-AP (`http://192.168.4.1`)
  * **Interface 2:** Traditional RC Receiver via hardware interrupt **PPM decoding on Pin D9** (with auto-arbitration priority)
* **Servos:** Dual 2S High-Voltage Coreless Metal-Gear Digital Servos (**Servomy SDS1601**, **C-Torque 9g HV**, or **PTK 7465MG**)
* **PWM Servo Frame Rate:** **333 Hz** (sub-microsecond precision)
* **Power:** 2S 7.4V–8.4V 220mAh–350mAh LiPo (45C burst)
* **Lighting:** Flexible 3V LED filaments for night flight (Breathing / Strobe modes)

---

## 2. Flight Dynamics & Aerodynamic Breakthroughs

```
       Flapping Axis                     Flapping Axis
           \                                 /
            \                               /
      [ Left Wing ]                   [ Right Wing ]
             \                             /
              +---[ Left Servo ]-[ Right Servo ]---+
                           [ Central Chassis ]
```

### Why This Architecture Outperforms Previous Designs
1. **Differential-Amplitude Steering:**
   * Traditional builds tilted the servo neutral angle, which frequently stalled the inside wing and caused the butterfly to drop altitude.
   * **Differential-Amplitude Breakthrough:** Flap amplitude is dynamically redistributed between the wings:
     $$\text{Wing Delta} = \text{SteerNorm} \times \text{BaseAmplitude} \times 0.45$$
     $$\text{Left Amplitude} = \text{BaseAmplitude} + \text{Wing Delta}$$
     $$\text{Right Amplitude} = \text{BaseAmplitude} - \text{Wing Delta}$$
     When turning right, the left wing flaps through a deeper arc (producing higher thrust/lift) while the right wing flaps through a shallower arc. This generates smooth, natural banking turns.
2. **Automatic Turn Lift Boost:**
   * Banking tilts aerodynamic lift sideways, reducing vertical lift. The flight controller automatically injects an extra flap amplitude boost during hard steering:
     $$\text{TurnLiftBoost} = 1.0 + (|\text{SteerNorm}| \times 0.15)$$
     At full stick deflection, flap amplitude surges by **+15%**, maintaining altitude throughout tight bank turns.
3. **Zero-Throttle Idle Lean:**
   * Below 5% throttle, stick inputs do not flap the wings; instead, they gently pose the wings in an organic resting lean (up to ±15°).
4. **333Hz Digital Servo Refresh:**
   * Traditional 50Hz Arduino servo libraries introduce up to 20ms of mechanical lag. Driving SDS1601 digital servos at **333Hz** delivers instantaneous aerodynamic authority.

---

## 3. Repository Architecture & File Map

```
├── README.md                                # Master overview, pinouts, and quickstart
├── DIY_BUILD_GUIDE.md                       # Complete scratch-build engineering manual
├── firmware/
│   └── ESP32_Bionic_Butterfly/              # Unified Arduino IDE flight firmware
│       ├── ESP32_Bionic_Butterfly.ino       # Main flight controller & dual-control engine
│       ├── config.h                         # Pin definitions, servo calibration & tuning
│       ├── ppm_receiver.h                   # High-speed interrupt PPM decoder
│       └── web_ui.h                         # Embedded responsive touch cockpit
├── hardware/                                # 3D models (pre-mirrored & flight-ready)
│   ├── FC_Mount_V5.stl                      # Central chassis (mounts servos & electronics)
│   ├── ARMV5_Right.stl                      # Right wing drive arm
│   └── ARMV5_Left_Mirrored.stl              # Pre-mirrored Left wing drive arm
└── documentation/                           # Official schematics, blueprints & cut charts
    ├── Left_Wing_Blueprint.pdf              # 1:1 Vector printable left wing template
    ├── Right_Wing_Blueprint.pdf             # 1:1 Vector printable right wing template
    ├── Rod_Length_and_Quantity_Chart.jpg    # Exact carbon fiber cut list
    ├── Wing_Carbon_Rod_Thickness_Schematic.png # Spar layout diagram
    ├── Wiring_Diagram_ESP32.jpg             # Official modular wiring schematic
    ├── Flight_Controller_PCB_Schematic.jpg  # Circuit schematic
    ├── Print_Settings.jpg                   # Bambu/OrcaSlicer verified print settings
    └── SDS1601_Servo_Datasheet.jpg          # Servo technical datasheet
```

---

## 4. Complete Bill of Materials (Exact Procurement List)

### A. Flight Electronics & Actuation
| Item | Exact Model / Spec | Qty | Purpose & Notes |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | **Seeed Studio XIAO ESP32-C3** | 1 | 32-bit RISC-V @ 160MHz. Built-in Wi-Fi AP, ESP-NOW, and Bluetooth. Weighs only 1.2g. |
| **Flapping Servos** | **Servomy SDS1601** (or *C-Torque 9g HV* / *PTK 7465MG*) | 2 | **Must be 8.4V (2S) compatible**, Coreless, Metal-Gear, speed $\le$ 0.08s/60°, 7.5 kg·cm stall torque. |
| **PPM Receiver (Optional)** | **FlySky FS-A8S** (or FS-iA6B / micro PPM receiver) | 1 | Connects to Pin D9 for physical RC transmitter control. |
| **Step-Down Buck Regulator** | **Micro 1.8A–3A Buck Converter** (MP2307 or Mini 360) | 1 | Steps down 2S 8.4V battery voltage to clean 5.0V for the XIAO ESP32-C3. |
| **Power Decoupling Capacitor** | **470µF – 1000µF Low-ESR Electrolytic (10V or 16V)** | 1 | Connects across the 8.4V servo power rail to absorb back-EMF spikes. |
| **Battery Voltage Divider** | 1× **51 kΩ** (top) & 1× **18 kΩ / 20 kΩ** (bottom) | 1 set | Scales 8.4V LiPo down to < 2.5V for the XIAO D2 ADC pin. |
| **Flight Battery** | **2S 7.4V–8.4V 220mAh–350mAh LiPo (45C)** | 2–3 | e.g. Tattu 220mAh 45C (2x 1S in series with 3-pin balance plug) or 2S 300mAh pack. |
| **LED Driver (Night Flight)** | 1× **SI2300DS (N-MOSFET)** + 1× 10kΩ + 2× 150Ω (or 4× 82Ω) | 1 set | PWM-switches the flexible wing LED filaments from pin D3. |
| **Flexible LED Filaments** | **3V 130 mm Ultra-thin Flexible LED Filaments** | 2 | Ice Blue, Warm White, or Pink. Weighs under 0.3g per wing. |
| **Connectors & Wire** | Micro JST 1.25mm 2-pin (for wings) + 28/30 AWG silicone wire | 1 set | Quick disconnect for wings and clean wiring. |

---

## 5. Phase 1: 3D Printing & Slicer Configuration

All models are pre-processed in [`hardware/`](./hardware/):
* [`hardware/FC_Mount_V5.stl`](./hardware/FC_Mount_V5.stl) (Central Chassis)
* [`hardware/ARMV5_Right.stl`](./hardware/ARMV5_Right.stl) (Right Wing Arm)
* [`hardware/ARMV5_Left_Mirrored.stl`](./hardware/ARMV5_Left_Mirrored.stl) (**Pre-mirrored** Left Wing Arm)

### Verified Slicer Profile (from [`documentation/Print_Settings.jpg`](./documentation/Print_Settings.jpg)):
* **Slicer:** Bambu Studio / OrcaSlicer / PrusaSlicer
* **Material:** **PETG** (Recommended for vibration damping and impact resistance) or Tough PLA
* **Layer Height:** `0.16 mm` (First Layer: `0.20 mm`)
* **Wall Loops:** `2`
* **Top Shell:** `5 layers` (1.0 mm) with **Monotonic** surface pattern
* **Bottom Shell:** `3 layers` (0.6 mm) with **Monotonic** surface pattern
* **Sparse Infill Density:**
  * Chassis (`FC_Mount_V5.stl`): **15% Rectilinear** (~5.2g)
  * Wing Arms (`ARMV5_Right.stl` & `ARMV5_Left_Mirrored.stl`): **100% Solid** (~3.0g each)
* **Supports:** **Disabled** (Prints cleanly without support material when oriented flat on plate)
* **Seam Position:** **Aligned**

---

## 6. Phase 2: Wing Skeleton & Exact Carbon Fiber Cut List

The wing structure follows the official engineering blueprints:
* Vector PDF: [`documentation/Left_Wing_Blueprint.pdf`](./documentation/Left_Wing_Blueprint.pdf)
* Vector PDF: [`documentation/Right_Wing_Blueprint.pdf`](./documentation/Right_Wing_Blueprint.pdf)

### Exact Rod Cut List (Per Wing)
*(Refer to [`documentation/Rod_Length_and_Quantity_Chart.jpg`](./documentation/Rod_Length_and_Quantity_Chart.jpg))*

```
                      1.0 mm Leading Edge Rod (400 mm)
            ====================================================\
           /                                                     \
          /   1.5 mm Main Vein (315 mm)                           \
         /  =========================================\             \
 [HUB] =<     1.0 mm Top Vein (255 mm)                \             ) Forewing
  ||     \  -----------------------------------\       )           /
  ||      \   1.0 mm Upper-mid Vein (247 mm)    )     /           /
  ||       \  ----------------------------\    /     /           /
  ||        \ 1.0 mm Lower-mid Vein (237 mm)  /     /           /
  ||         \ -------------------------\    /     /           /
  ||          \ 1.0 mm Bottom Vein (226mm)  /     /           /
  ||           \ -----------------------\  /     /           /
  ||            \                        \/     /           /
  ||             \                             /           /
                  \   1.0 mm Hindwing         /           /
                   \  Loop (2x 356 mm)       /           /
                    \-----------------------/-----------/
```

#### 1.5 mm Diameter Solid Carbon Fiber Rods (Per Wing):
1. **Main Vein:** 315 mm (x1)
2. **Root Attachment:** 90 mm (x1)
3. **Leading Edge Support:** 60 mm (x1)
4. **Hindwing Side Attachment:** 35 mm (x1)
* *Total 1.5 mm per wing:* 500 mm (1.0 meter total for both wings)

#### 1.0 mm Diameter Solid Carbon Fiber Rods (Per Wing):
1. **Leading Edge:** 400 mm (x1)
2. **Top Vein:** 255 mm (x1)
3. **Upper-mid Vein:** 247 mm (x1)
4. **Lower-mid Vein:** 237 mm (x1)
5. **Bottom Vein:** 226 mm (x1)
6. **Hindwing Perimeter Half:** 356 mm (x2) = 712 mm
* *Total 1.0 mm per wing:* 2,077 mm (~4.2 meters total for both wings, approx 5 standard 1m rods)

### Skeleton Assembly Steps:
1. Print the left and right PDF blueprints on A3 or two A4 sheets at **100% scale (no scaling)**. Check scale bar.
2. Tape blueprints to a flat cork or dense foam building board and cover with wax paper.
3. Cut carbon rods to the exact lengths listed above using a diamond cutting disc or fine rotary tool (wear a dust mask).
4. Sand the tips lightly with 400-grit sandpaper.
5. Insert root spars into the 3D-printed `ARMV5` sockets with **Medium CA glue**.
6. At each intersection, lash tightly with **4–6 turns of braided Kevlar thread (30–50 lb)**, apply a drop of **Thin CA glue**, and spray with **CA Kicker**.

---

## 7. Phase 3: Wing Skinning & Night-Flight Flexible LED Filaments

1. **Flexible LED Filament (Optional for Night Flying):**
   * Lay one 130mm 3V flexible LED filament along the 1.0mm leading edge carbon rod.
   * Secure with tiny dabs of Thin CA every 30mm.
   * Solder ultra-thin 32 AWG magnet wire to the copper pads and terminate with a Micro JST 1.25mm plug at the hub.
2. **Wing Membrane:**
   * Material: **0.5 oz (approx 20–30 g/m²) Ripstop Nylon** or **15–20 µm Mylar / BoPET film**.
   * Lightly mist the back of the skeleton with **3M 77 spray adhesive** from 30cm away (wait 45 seconds to get tacky).
   * Smooth the membrane from the leading edge outward toward the trailing edge, eliminating wrinkles.
   * Trim perimeter leaving a 2mm border using a sharp #11 hobby knife or hot soldering iron tip (which seals nylon edges).

---

## 8. Phase 4: Servo Actuation & Spline Indexing

Refer to [`documentation/SDS1601_Servo_Datasheet.jpg`](./documentation/SDS1601_Servo_Datasheet.jpg):
* **Neutral Pulse:** 1520 µs
* **Travel Range:** 800 µs – 2200 µs
* **Refresh Rate:** 333 Hz

1. **Electrical Centering:**
   * Power on your Seeed Studio XIAO ESP32-C3 with the firmware loaded.
   * Connect servos so they seek their neutral **1520 µs** position.
2. **Spline Indexing:**
   * Mount the single-arm horn perpendicular (90°) to the servo body.
   * Fasten with the central spline retention screw.
3. **Chassis Installation:**
   * Slide servos into [`hardware/FC_Mount_V5.stl`](./hardware/FC_Mount_V5.stl) and secure using M2 self-tapping screws.
   * Fasten the wing drive arms (`ARMV5_Right.stl` and `ARMV5_Left_Mirrored.stl`) to the servo horns. Manually check for zero mechanical binding through ±65° of travel.

---

## 9. Phase 5: Electrical Wiring & Power Conditioning

Refer to the official schematic: [`documentation/Wiring_Diagram_ESP32.jpg`](./documentation/Wiring_Diagram_ESP32.jpg).

```
                      +-----------------------------------+
                      |   2S LiPo Battery (7.4V - 8.4V)  |
                      +-----------------+-----------------+
                                        |
                                [ Micro SPDT Switch ]
                                        |
                   +--------------------+--------------------+
                   | (+) 8.4V                                | (-) GND
                   |                                         |
         +---------+---------+                     +---------+---------+
         |   Low-ESR Cap     |                     |                   |
         |  470uF - 1000uF   |                     |                   |
         +---------+---------+                     |                   |
                   |                               |                   |
         +---------+---------+                     |                   |
         |  Step-Down Buck   |                     |                   |
         |   (8.4V -> 5.0V)  |                     |                   |
         +---------+---------+                     |                   |
                   | (+) 5.0V                      |                   |
                   |                               |                   |
        +----------+----------+                    |                   |
        |  XIAO '5V' Pin      |                    |  XIAO 'GND' Pin   |
        +---------------------+                    +-------------------+
```

### Full Pinout Map:
* **Left Servo Signal:** $\rightarrow$ XIAO Pin **D0**
* **Right Servo Signal:** $\rightarrow$ XIAO Pin **D1**
* **Battery Voltage Divider:**
  * Battery (+) $\rightarrow$ 51 kΩ resistor $\rightarrow$ XIAO Pin **D2** $\rightarrow$ 18 kΩ / 20 kΩ resistor $\rightarrow$ GND
* **Wing LED Filament Switch:**
  * XIAO Pin **D3** $\rightarrow$ Gate of SI2300 N-MOSFET (with 10kΩ pull-down to GND).
  * Drain switches negative side of Wing LEDs through two 150Ω resistors to 5V.
* **Status LED (Green):** $\rightarrow$ XIAO Pin **D6** (via 220Ω resistor)
* **Low Battery LED (Red):** $\rightarrow$ XIAO Pin **D8** (via 68Ω resistor)
* **PPM RC Receiver Signal:** $\rightarrow$ XIAO Pin **D9** (powered from 5V rail)

---

## 10. Phase 6: Firmware Setup & Flashing

The firmware is located in [`firmware/ESP32_Bionic_Butterfly/`](./firmware/ESP32_Bionic_Butterfly/):
* [`firmware/ESP32_Bionic_Butterfly/ESP32_Bionic_Butterfly.ino`](./firmware/ESP32_Bionic_Butterfly/ESP32_Bionic_Butterfly.ino)
* [`firmware/ESP32_Bionic_Butterfly/config.h`](./firmware/ESP32_Bionic_Butterfly/config.h)
* [`firmware/ESP32_Bionic_Butterfly/ppm_receiver.h`](./firmware/ESP32_Bionic_Butterfly/ppm_receiver.h)
* [`firmware/ESP32_Bionic_Butterfly/web_ui.h`](./firmware/ESP32_Bionic_Butterfly/web_ui.h)

### Flashing via Arduino IDE:
1. Open **Arduino IDE -> Settings** and add the ESP32 board URL:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
2. Install **esp32 by Espressif** (version 2.0.14 or later) from Boards Manager.
3. Install **ESP32Servo** library from Library Manager.
4. Select board: **Tools -> Board -> ESP32 Arduino -> Seeed Studio XIAO ESP32C3**.
5. Set:
   * **USB CDC On Boot:** `Enabled`
   * **Flash Frequency:** `80MHz`
   * **Upload Speed:** `921600`
6. Open `firmware/ESP32_Bionic_Butterfly/ESP32_Bionic_Butterfly.ino` and click **Upload**.

---

## 11. Phase 7: Dual Piloting: Smartphone Web App & PPM RC Controller

### Automatic Arbitration Logic:
* **When PPM RC is Active:** If your FlySky/RC transmitter is powered on and sending pulses to Pin D9, the flight controller automatically gives **100% control to your physical transmitter**. The web app badge displays `PPM RC ACTIVE`.
* **When Smartphone Web App is Active:** If the RC transmitter is turned off or unplugged, the smartphone web cockpit seamlessly takes control!

### Method A: Smartphone Web App
1. Turn on the butterfly.
2. In your phone's Wi-Fi settings, connect to **`Butterfly_RC`** (Password: **`12345678`**).
3. Open Safari or Chrome and navigate to **`http://192.168.4.1`**.
4. **Cockpit Interface:**
   * **Vertical Throttle Slider:** 0% to 100% flapping frequency and amplitude.
   * **Touch Joystick:** Steering (differential amplitude) and Pitch (climb/dive bias).
   * **LED Toggle:** Cycles Off, Breathing Glow, and Night Strobe.
   * **Live Battery Gauge:** Real-time 2S pack voltage.
   * **Arm/Disarm Button:** Instantly cuts power to wings.

### Method B: PPM RC Transmitter (FlySky FS-i6 / EdgeTX)
* **Channel 1 (Aileron):** Differential-amplitude steering.
* **Channel 2 (Elevator):** Symmetric neutral pitch bias.
* **Channel 3 (Throttle):** Flapping frequency and amplitude modulation (1.4 Hz to 4.2 Hz).
* **Channel 5 (Arm):** 2-position switch to arm/disarm.
* **Channel 6 (Bias Pot):** Fine pitch trim.
* **Channel 7 (Perch):** Activates gentle resting flutter.
* **Channel 8 (LED):** Toggles night lighting modes.

---

## 12. Phase 8: CG Balancing, Flight Trimming & Maiden Flight

1. **Center of Gravity (CG) Balancing:**
   * Support the butterfly under the wing arms at **35% to 40% of the mean wing chord**.
   * Slide the 2S battery pack forward or backward until the body sits horizontally level.
2. **Bench Pre-Flight Check:**
   * At 0% throttle, moving joystick left/right should tilt wings into an **idle lean**.
   * At 40% throttle, pushing joystick right should cause the Left wing to flap deeper and the Right wing to flap shallower.
   * Verify Failsafe: Disconnecting Wi-Fi or switching off transmitter should smoothly lock wings into an upward **+18° "V" dihedral glide**.
3. **Maiden Flight:**
   * Fly in a gymnasium or outdoors in dead calm air (zero wind).
   * Hold the chassis from underneath, advance throttle to **65%**, and gently release forward into level air.
   * Use small, gentle steering inputs to circle. Cut throttle to glide down smoothly.

---

## 13. Troubleshooting & Diagnostics

| Symptom | Probable Cause | Solution |
| :--- | :--- | :--- |
| **Butterfly flaps but fails to climb** | Wing ribs too rigid or insufficient trailing edge billow | Verify leading edge is 1.0mm, main vein is 1.5mm, and side ribs are 1.0mm. Ensure membrane is flexible. |
| **Microcontroller crashes during flapping** | Servo back-EMF spike or voltage sag | Ensure the 470µF–1000µF Low-ESR capacitor is soldered across the 8.4V rail and the 5V buck converter is working. |
| **Butterfly drops altitude in turns** | Insufficient turn lift compensation | In firmware, verify `TURN_LIFT_BOOST_FRAC` is set to `0.15f` (+15% boost). |
| **Butterfly banks aggressively to one side** | Asymmetric wing construction or servo neutral offset | Check wing weights on a precision scale. Adjust trim slider in web app or CH6 pot. |
| **Servos buzz or overheat** | Travel clipping against physical travel end-stop | Verify pulse bounds (`SERVO_US_MIN = 800`, `SERVO_US_MAX = 2200`). |
| **Butterfly stalls backward upon release** | Tail-heavy (CG too far aft) | Slide battery forward toward the nose. |
