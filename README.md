# RC Bionic Butterfly (ESP32-C3 Architecture) 🦋

An ultra-responsive, lightweight radio-controlled **Bionic RC Butterfly** ornithopter project powered by the **Seeed Studio XIAO ESP32-C3**, featuring **Dual Piloting Support (Smartphone Web App + PPM RC Controller)**, 333Hz digital servo PWM, differential-amplitude steering, turn lift boost, and glowing night-flight LED filaments.

Inspired by **Max Imagination's ESP-FLUTTER** ([YouTube Build Video](https://youtu.be/5Le4I-1Pf9I)) and **CTorque Robotics** mechanics.

---

## 🦋 Core Specifications

* **Wingspan:** 390 mm – 410 mm (~15.5 inches)
* **Flying Weight (AUW):** ~65 g – 72 g
* **Flight Controller:** **Seeed Studio XIAO ESP32-C3** (1.2g, 32-bit RISC-V @ 160MHz)
* **Dual Control Links:**
  * **Primary:** Smartphone Web App via on-board Wi-Fi AP (`http://192.168.4.1`)
  * **Secondary:** Traditional RC Receiver via hardware interrupt **PPM decoding on Pin D9** (FlySky FS-A8S, etc.)
* **Servos:** Dual 2S High-Voltage Coreless Metal-Gear Digital Servos (**Servomy SDS1601**, **C-Torque 9g HV**, or **PTK 7465MG**)
* **Servo PWM Frame Rate:** **333 Hz**
* **Power:** 2S 7.4V–8.4V 220mAh–350mAh LiPo
* **Lighting:** Flexible 3V LED filaments for night flight (Breathing / Strobe modes)

---

## 📁 Consolidated Repository Structure

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

## ⚡ Key Flight Innovations

1. **Dual Control Arbitration (Web Cockpit + PPM RC):**
   Fly from your smartphone via Wi-Fi AP or plug in your FlySky/PPM receiver on Pin D9. The flight controller detects active PPM pulses and automatically grants full priority to your physical transmitter!
2. **Differential-Amplitude Steering:**
   Dynamically shifts stroke depth between the left and right wings (e.g. left wing flaps 75°, right wing flaps 45°) to produce sharp, high-agility banking turns without stalling the inside wing.
3. **Automatic Turn Lift Boost:**
   Injects an automatic **+15% flap amplitude boost** during hard steering to compensate for tilted lift and prevent altitude loss during tight turns.
4. **Night Flight Glowing Flexible LED Filaments:**
   Drives ultra-lightweight 3V flexible LED filaments along the wing leading edges with Solid, Breathing Glow, and Night Strobe modes.
5. **333Hz High-Speed Servo PWM:**
   Drives digital servos at 333Hz with sub-microsecond pulse precision for instant aerodynamic authority.

---

## 🛠️ Quick Start Build Checklist

### 1. 3D Printing (PETG)
Print files from [`hardware/`](./hardware/):
* **Central Chassis ([`hardware/FC_Mount_V5.stl`](./hardware/FC_Mount_V5.stl)):** Print 1× with 15% infill, 2 walls, no supports.
* **Right Wing Arm ([`hardware/ARMV5_Right.stl`](./hardware/ARMV5_Right.stl)):** Print 1× at 100% solid infill.
* **Left Wing Arm ([`hardware/ARMV5_Left_Mirrored.stl`](./hardware/ARMV5_Left_Mirrored.stl)):** Print 1× at 100% solid infill (pre-mirrored).

### 2. Wing Fabrication
* Print [`documentation/Left_Wing_Blueprint.pdf`](./documentation/Left_Wing_Blueprint.pdf) and [`Right_Wing_Blueprint.pdf`](./documentation/Right_Wing_Blueprint.pdf) on A3 or two A4 sheets at 100% scale.
* Cut solid carbon fiber rods to exact lengths from [`documentation/Rod_Length_and_Quantity_Chart.jpg`](./documentation/Rod_Length_and_Quantity_Chart.jpg):
  * **1.5 mm Carbon:** Main Vein (315mm), Root (90mm), LE Support (60mm), Hindwing Side (35mm).
  * **1.0 mm Carbon:** Leading Edge (400mm), Top Vein (255mm), Upper-mid (247mm), Lower-mid (237mm), Bottom (226mm), Hindwing (2x 356mm).
* Lash joints with Kevlar thread, bond with Thin CA, and skin with **0.5 oz Ripstop Nylon** or **15 µm Mylar film**.

### 3. Wiring (Seeed Studio XIAO ESP32-C3)
* Refer to [`documentation/Wiring_Diagram_ESP32.jpg`](./documentation/Wiring_Diagram_ESP32.jpg).
* **Left Servo Signal:** $\rightarrow$ Pin **D0**
* **Right Servo Signal:** $\rightarrow$ Pin **D1**
* **PPM Receiver Signal (Optional):** $\rightarrow$ Pin **D9**
* **Servo Power Rail:** 8.4V direct from 2S LiPo with a **470µF–1000µF Low-ESR capacitor**.
* **XIAO Power:** 8.4V $\rightarrow$ Micro Buck Converter (5.0V) $\rightarrow$ XIAO **5V** Pin.
* **Battery Sensing:** 51kΩ / 18kΩ divider to Pin **D2**.
* **Wing LED Filament:** SI2300 N-MOSFET to Pin **D3**.

### 4. Firmware Flashing
* Open [`firmware/ESP32_Bionic_Butterfly/ESP32_Bionic_Butterfly.ino`](./firmware/ESP32_Bionic_Butterfly/ESP32_Bionic_Butterfly.ino) in Arduino IDE.
* Board: **Seeed Studio XIAO ESP32C3**.
* Upload and connect to `Butterfly_RC` on your smartphone (or turn on your FlySky RC transmitter)!

---

## 📖 Complete Documentation

For the comprehensive step-by-step assembly guide, exact measurements, radio programming, and flight trimming procedures:
👉 **[Read the Master Scratch-Build DIY Manual (`DIY_BUILD_GUIDE.md`)](./DIY_BUILD_GUIDE.md)**

---

## 📜 Credits & Acknowledgments
* **Max Imagination** for the open-source [ESP-FLUTTER](https://github.com/maximagination1/ESP-FLUTTER) project, differential steering architecture, and wing blueprints.
* **CTorque Robotics** & **S-DiY / TruongVanSu91** for original mechanical ornithopter concepts.
