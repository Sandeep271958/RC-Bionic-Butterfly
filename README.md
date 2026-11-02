# RC Bionic Butterfly (ESP-FLUTTER Architecture) 🦋

An ultra-responsive, lightweight radio-controlled **Bionic RC Butterfly** ornithopter project powered by the **Seeed Studio XIAO ESP32-C3**, featuring differential-amplitude steering, automatic turn lift boost, smartphone web app control, 333Hz digital servo actuation, and glowing night-flight LED filaments.

Based on **Max Imagination's ESP-FLUTTER** ([YouTube Build Video](https://youtu.be/5Le4I-1Pf9I)) and **CTorque Robotics** mechanics.

---

## 🦋 Core Specifications

* **Wingspan:** 390 mm – 410 mm (~15.5 inches)
* **Flying Weight (AUW):** ~65 g – 72 g
* **Flight Controller:** **Seeed Studio XIAO ESP32-C3** (1.2g, 32-bit RISC-V @ 160MHz)
* **Control Links:**
  * **Primary:** Smartphone Web App via on-board Wi-Fi AP (`http://192.168.4.1`)
  * **Secondary:** ESP-NOW direct transmitter or ExpressLRS (ELRS 2.4GHz) CRSF UART
* **Servos:** Dual 2S High-Voltage Coreless Metal-Gear Digital Servos (**Servomy SDS1601**, **C-Torque 9g HV**, or **PTK 7465MG**)
* **Power:** 2S 7.4V–8.4V 220mAh–350mAh LiPo
* **Lighting:** Flexible 3V LED filaments for night flight (Breathing / Strobe modes)

---

## 📁 Repository Structure

```
├── README.md                          # Master overview and quick start
├── DIY_BUILD_GUIDE.md                 # Complete scratch-build engineering manual
├── ESP32_C3_Firmware/                 # Ready-to-flash Arduino IDE sketch for XIAO ESP32-C3
│   └── ESP32_Bionic_Butterfly.ino     # Embedded Web App, 333Hz PWM, battery ADC cutoff
├── ESP-FLUTTER-Source/                # Upstream PlatformIO modular firmware source code
├── ESP_FLUTTER_Resources/             # Official blueprints, cut charts, and schematics
│   ├── Left_Wing_Blueprint.pdf        # 1:1 Vector printable left wing template
│   ├── Right_Wing_Blueprint.pdf       # 1:1 Vector printable right wing template
│   ├── Rod_Length_and_Quantity_Chart.jpg # Exact carbon fiber cut list
│   ├── Wing_Carbon_Rod_Thickness_Schematic.png # Spar layout diagram
│   ├── Wiring_Diagram_Modular_ESP32C3.jpg # Official modular wiring schematic
│   ├── PCB_Schematic.jpg              # Official flight controller circuit schematic
│   ├── Print_Settings.jpg             # Bambu/OrcaSlicer verified print settings
│   └── SDS1601_Servo_Specs.jpg        # Servo technical datasheet
├── bionic butterfly mount 20260113/   # 3D printable CAD models
│   ├── FC Mount V5.STL                # Central chassis (mounts servos & electronics)
│   ├── ARMV5.STL                      # Wing hub drive arm (print 1x normal, 1x mirrored!)
│   └── 3.STL                          # Duplicate backup of FC Mount V5
├── Bionic_Butterfly_Enhanced.ino      # Legacy Arduino Pro Micro (ATmega32U4) v2.5 firmware
├── circuit.jpeg                       # Legacy Arduino Pro Micro wiring schematic
├── wing design.jpeg                   # Legacy 2D wing image
└── Ctorque 3d models.zip              # Compressed archive of original 3D models
```

---

## ⚡ Key Flight Innovations

1. **Differential-Amplitude Steering:**
   Dynamically shifts stroke depth between the left and right wings (e.g. left wing flaps 75°, right wing flaps 45°) to produce sharp, high-agility banking turns without stalling the inside wing.
2. **Automatic Turn Lift Boost:**
   Injects an automatic **+15% flap amplitude boost** during hard steering to compensate for tilted lift and prevent altitude loss during tight turns.
3. **Zero-Receiver Smartphone Web App:**
   Connect to Wi-Fi **`Butterfly_RC`** and open `http://192.168.4.1` for a full on-screen touch joystick, throttle slider, and real-time battery telemetry. Shaves **5.3 grams** of receiver weight!
4. **Night Flight Glowing Flexible LED Filaments:**
   Drives ultra-lightweight 3V flexible LED filaments along the wing leading edges with Solid, Breathing Glow, and Night Strobe modes.
5. **333Hz High-Speed Servo PWM:**
   Drives digital servos at 333Hz with sub-microsecond pulse precision for instant aerodynamic authority.

---

## 🛠️ Quick Start Build Checklist

### 1. 3D Printing (PETG)
* **Central Chassis (`FC Mount V5.STL`):** Print 1× with 15% infill, 2 walls, no supports.
* **Wing Hub Arms (`ARMV5.STL`):** 
  * Print **1× Normal** (Right Arm) at **100% solid infill**.
  * Print **1× Mirrored in Slicer** (Left Arm) at **100% solid infill**.

### 2. Wing Fabrication
* Print [`ESP_FLUTTER_Resources/Left_Wing_Blueprint.pdf`](./ESP_FLUTTER_Resources/Left_Wing_Blueprint.pdf) and [`Right_Wing_Blueprint.pdf`](./ESP_FLUTTER_Resources/Right_Wing_Blueprint.pdf) on A3 or two A4 sheets at 100% scale.
* Cut solid carbon fiber rods to exact lengths from [`ESP_FLUTTER_Resources/Rod_Length_and_Quantity_Chart.jpg`](./ESP_FLUTTER_Resources/Rod_Length_and_Quantity_Chart.jpg):
  * **1.5 mm Carbon:** Main Vein (315mm), Root (90mm), LE Support (60mm), Hindwing Side (35mm).
  * **1.0 mm Carbon:** Leading Edge (400mm), Top Vein (255mm), Upper-mid (247mm), Lower-mid (237mm), Bottom (226mm), Hindwing (2x 356mm).
* Lash joints with Kevlar thread, bond with Thin CA, and skin with **0.5 oz Ripstop Nylon** or **15 µm Mylar film**.

### 3. Wiring (Seeed Studio XIAO ESP32-C3)
* Refer to [`ESP_FLUTTER_Resources/Wiring_Diagram_Modular_ESP32C3.jpg`](./ESP_FLUTTER_Resources/Wiring_Diagram_Modular_ESP32C3.jpg).
* **Left Servo Signal:** $\rightarrow$ Pin **D0**
* **Right Servo Signal:** $\rightarrow$ Pin **D1**
* **Servo Power Rail:** 8.4V direct from 2S LiPo with a **470µF–1000µF Low-ESR capacitor**.
* **XIAO Power:** 8.4V $\rightarrow$ Micro Buck Converter (5.0V) $\rightarrow$ XIAO **5V** Pin.
* **Battery Sensing:** 51kΩ / 18kΩ divider to Pin **D2**.
* **Wing LED Filament:** SI2300 N-MOSFET to Pin **D3**.

### 4. Firmware Flashing
* Open [`ESP32_C3_Firmware/ESP32_Bionic_Butterfly.ino`](./ESP32_C3_Firmware/ESP32_Bionic_Butterfly.ino) in Arduino IDE.
* Board: **Seeed Studio XIAO ESP32C3**.
* Upload and connect to `Butterfly_RC` on your smartphone!

---

## 📖 Complete Documentation

For the comprehensive step-by-step assembly guide, exact measurements, radio programming, and flight trimming procedures:
👉 **[Read the Master Scratch-Build DIY Manual (`DIY_BUILD_GUIDE.md`)](./DIY_BUILD_GUIDE.md)**

---

## 📜 Credits & Acknowledgments
* **Max Imagination** for the open-source [ESP-FLUTTER](https://github.com/maximagination1/ESP-FLUTTER) project, differential steering architecture, and wing blueprints.
* **CTorque Robotics** & **S-DiY / TruongVanSu91** for original mechanical ornithopter concepts.
