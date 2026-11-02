# RC Bionic Butterfly (Biomimetic Flapping-Wing Ornithopter)

An ultra-responsive, lightweight radio-controlled **Bionic Butterfly** ornithopter project featuring dual-servo independent wing kinematics, differential-amplitude steering, turn lift boost, smartphone web control, and glowing night-flight LED filaments.

Inspired by mechanical research from **S-DiY / TruongVanSu91**, **CTorque Tech**, and **Max Imagination's ESP-FLUTTER** ([YouTube: Make an ESP32 RC Bionic Butterfly That Flies](https://youtu.be/5Le4I-1Pf9I)).

---

## 🦋 Project Highlights

* **Wingspan:** ~380 mm – 400 mm (~15.5 inches)
* **All-Up-Weight (AUW):** ~55 g – 65 g
* **Actuation:** Dual 9g High-Voltage (HV) or Metal-Gear digital servos
* **Power:** 2S 7.4V 250mAh – 350mAh LiPo
* **Dual Flight Controller Support:**
  * **Track 1:** Arduino Pro Micro (ATmega32U4) for traditional PPM/SBUS RC transmitters.
  * **Track 2 (Recommended):** Seeed Studio XIAO ESP32-C3 for **zero-receiver-weight smartphone web app control**, 333Hz PWM, and ESP-NOW/ELRS telemetry.

---

## 📁 Repository Structure

```
├── README.md                          # Project overview and quick start
├── DIY_BUILD_GUIDE.md                 # Step-by-step scratch build manual & video analysis
├── Bionic_Butterfly_Enhanced.ino      # Upgraded v2.5 Arduino Pro Micro flight firmware
├── ESP32_C3_Firmware/                 # High-performance ESP32-C3 firmware track
│   └── ESP32_Bionic_Butterfly.ino     # Smartphone Web App controller & 333Hz PWM driver
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

## ⚡ Key Innovations & Aerodynamic Features

1. **Differential-Amplitude Steering (Max Imagination Architecture):**
   Instead of merely shifting the mean servo angle, steering dynamically varies the stroke amplitude between left and right wings (e.g. left flaps 75°, right flaps 45°). This delivers crisp, agile yaw/roll turning without wing stall!
2. **Automatic Turn Lift Boost:**
   During banked turns, aerodynamic lift is tilted sideways. The controller automatically injects a **+15% flap amplitude boost** during hard steering to prevent altitude loss.
3. **Zero-Receiver Smartphone Web App (ESP32-C3):**
   Connect your phone to **`Butterfly_RC`** and open `http://192.168.4.1` for a full on-screen touch joystick, throttle slider, and live battery voltage telemetry. Saves 5.3g of dead weight!
4. **Zero-Throttle Idle Lean:**
   Below 5% throttle, stick inputs do not flap the wings; instead, they pose the wings in a smooth, organic resting tilt (up to ±15°).
5. **Night-Flight Glowing Flexible LED Filaments:**
   Drives ultra-lightweight 3V flexible LED filaments along the wing leading edge with Solid, Breathing, and Strobe modes.
6. **Aerodynamic Dihedral Failsafe:**
   Transitions wings into a stable +18° upward "V" dihedral glide if signal is lost, preventing flat-spin crashes.

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
* Optional: Bond a 130mm 3V flexible LED filament along each leading edge for night flight.
* Skin with **0.5 oz Ripstop Nylon** or **15 µm Mylar film** using 3M 77 spray adhesive or 3mm double-sided tape.

### 3. Electrical Wiring & Power
* **Power Rail:** Solder a **470µF – 1000µF Low-ESR capacitor** directly across the servo power rail (+ and -) to protect the flight controller from inductive back-EMF spikes.
* **Arduino Track:** Left Servo $\rightarrow$ Pin 4, Right Servo $\rightarrow$ Pin 5, Receiver PPM $\rightarrow$ Pin 2.
* **ESP32-C3 Track:** Left Servo $\rightarrow$ Pin D0, Right Servo $\rightarrow$ Pin D1, Battery Divider $\rightarrow$ Pin D2, LED $\rightarrow$ Pin D3.

### 4. Flashing Firmware
* **For Arduino Pro Micro:** Open [`Bionic_Butterfly_Enhanced.ino`](./Bionic_Butterfly_Enhanced.ino) and upload as *Arduino Leonardo / Micro*.
* **For Seeed Studio XIAO ESP32-C3:** Open [`ESP32_C3_Firmware/ESP32_Bionic_Butterfly.ino`](./ESP32_C3_Firmware/ESP32_Bionic_Butterfly.ino) and upload as *Seeed Studio XIAO ESP32C3*.

---

## 📋 Complete Documentation

For the full step-by-step build manual, wiring schematics, video analysis, and flight trimming procedures, consult:
👉 **[Complete DIY Build Guide (`DIY_BUILD_GUIDE.md`)](./DIY_BUILD_GUIDE.md)**

---

## 📜 Credits & Acknowledgments
* Mechanical 3D models and ornithopter research inspired by **CTorque Tech** and **S-DiY / TruongVanSu91**.
* Differential-amplitude kinematics, web-app control, and night-flight LED filament concepts based on **Max Imagination's ESP-FLUTTER** ([YouTube](https://youtu.be/5Le4I-1Pf9I)).
