# RC Bionic Butterfly (Biomimetic Flapping-Wing Ornithopter)

An ultra-responsive, lightweight radio-controlled **Bionic Butterfly** ornithopter project featuring dual-servo independent wing kinematics, biomimetic power stroke profiles, and aerodynamic dihedral gliding.

---

## 🦋 Project Overview

Unlike traditional RC planes or drones, this bionic butterfly generates lift and forward thrust using two independently driven servos flapping flexible carbon-fiber membrane wings.

* **Wingspan:** ~380 mm – 400 mm (~15.5 inches)
* **All-Up-Weight (AUW):** ~55 g – 65 g
* **Flight Controller:** Arduino Pro Micro (ATmega32U4) / Upgradeable to ESP32-C3 & F4 AIO
* **Actuation:** Dual 9g High-Voltage (HV) or Metal-Gear digital servos
* **Power:** 2S 7.4V 250mAh – 350mAh LiPo

---

## 📁 Repository Structure

```
├── README.md                          # Project overview and quick start
├── DIY_BUILD_GUIDE.md                 # Step-by-step scratch build manual & MCU upgrade guide
├── Bionic_Butterfly_Enhanced.ino      # Upgraded v2.0 high-responsiveness flight firmware
├── S_DIY bionic_butterfly code.ino    # Baseline v1.0 source firmware (for reference)
├── circuit.jpeg                       # Wiring schematic reference
├── wing design.jpeg                   # 1:1 printable wing spar & membrane template (2:3 aspect ratio)
├── bionic butterfly mount 20260113/   # 3D printable CAD models
│   ├── FC Mount V5.STL                # Central chassis (mounts servos & electronics)
│   ├── ARMV5.STL                      # Wing hub drive arm (print 1x normal, 1x mirrored!)
│   └── 3.STL                          # Backup copy of FC Mount V5
└── Ctorque 3d models.zip              # Compressed archive of original 3D models
```

---

## ⚡ Key Improvements in Firmware v2.0 (`Bionic_Butterfly_Enhanced.ino`)

1. **Jitter-Free Deterministic Timebase:** Operates on microsecond delta-time (`dt`), eliminating frequency flutter caused by loop delays or serial printing.
2. **Microsecond Precision Servo Drive:** Uses `Servo.writeMicroseconds(us)` instead of 1-degree steps, removing mechanical gear chatter and delivering smooth flapping arcs.
3. **Dynamic Throttle Authority:** Simultaneously scales flapping frequency (1.4 Hz to 4.2 Hz) and stroke amplitude (20° flutter to 65° power stroke).
4. **Biomimetic Asymmetric Power Stroke:** Uses a cycloidal waveform for a faster downstroke (vortex lift) and a cushioned recovery upstroke.
5. **Anti-Clipping Headroom Protection:** Prevents servos from stalling against physical travel limits when steering or pitch offsets are applied.
6. **Aerodynamic Dihedral Failsafe:** Transitions wings into a stable "V" dihedral glide if signal is lost, preventing flat-spin crashes.

---

## 🛠️ Quick Start Guide

### 1. 3D Printing
* **Central Chassis (`FC Mount V5.STL`):** Print 1× in **PETG** or Tough PLA (20% gyroid infill).
* **Wing Drive Arms (`ARMV5.STL`):** 
  * Print **1× Normal** (Right Wing Arm) at **100% infill**.
  * Print **1× Mirrored along X/Y axis in your slicer** (Left Wing Arm) at **100% infill**.

### 2. Wing Construction
* Print [`wing design.jpeg`](./wing%20design.jpeg) on A4 paper scaled to fit portrait orientation.
* Cut solid carbon fiber rods:
  * **1.5 mm OD:** Leading edge spars (~190–200 mm each).
  * **1.0 mm OD:** Forewing radiating ribs (~120–160 mm each).
  * **0.8 mm OD:** Hindwing frame & diagonals (~110–150 mm each).
* Lash rod junctions to `ARMV5` sockets with Kevlar thread and bond with Thin CA glue.
* Skin with **0.5 oz Ripstop Nylon** or **15 µm Mylar film** using 3M 77 spray adhesive or 3mm double-sided tape.

### 3. Electrical Wiring & Power
* **Receiver PPM Out** $\rightarrow$ Arduino Pin 2
* **Left Servo Signal** $\rightarrow$ Arduino Pin 4
* **Right Servo Signal** $\rightarrow$ Arduino Pin 5
* **Power Rail:** Solder a **470µF – 1000µF Low-ESR capacitor** directly across the servo power rail (+ and -) to protect the flight controller from inductive back-EMF spikes.
* *Note:* If using standard 5V-6V servos (MG90S, ES08MD), use a **5V/6V 3A Micro BEC** from the 2S LiPo.

### 4. Flashing Firmware
1. Open [`Bionic_Butterfly_Enhanced.ino`](./Bionic_Butterfly_Enhanced.ino) in Arduino IDE.
2. Install `PPMReader` library.
3. Select board **Arduino Leonardo / Micro** and upload.

---

## 🔬 Microcontroller Upgrade Research

See the comprehensive research in [`DIY_BUILD_GUIDE.md`](./DIY_BUILD_GUIDE.md#2-microcontroller-upgrade-research--flight-response-analysis) on upgrading from the stock Arduino Pro Micro:
* **Seeed Studio XIAO ESP32-C3:** Eliminates the external receiver via **ESP-NOW**, drops latency to 2–4 ms, and cuts **5.3 grams** of total weight.
* **FPV Micro AIO (STM32F411 with Gyro & ELRS):** Adds active 6-axis gyroscopic attitude damping to eliminate roll/pitch wobbling in wind.

---

## 📋 Complete Documentation

For the full step-by-step build manual, wiring schematics, radio calibration, and flight trimming procedures, consult:
👉 **[Complete DIY Build Guide (`DIY_BUILD_GUIDE.md`)](./DIY_BUILD_GUIDE.md)**

---

## 📜 Credits & Acknowledgments
* Based on open-source mechanical concepts and research from **S-DiY / TruongVanSu91** and **CTorque Tech**.
* Enhanced firmware and engineering build manual developed for biomimetic robotics enthusiasts.
