#pragma once
#include <Arduino.h>

// ==============================================================================
// 1. PIN DEFINITIONS (Seeed Studio XIAO ESP32-C3)
// ==============================================================================
#define PIN_SERVO_LEFT       D0 // Left wing servo PWM output
#define PIN_SERVO_RIGHT      D1 // Right wing servo PWM output
#define PIN_BATTERY_ADC      D2 // Battery voltage divider input (51k / 18k)
#define PIN_LED_FILAMENT     D3 // Flexible LED filament driver (PWM via MOSFET)
#define PIN_STATUS_LED       D6 // Status indicator LED (Green)
#define PIN_LOW_BAT_LED      D8 // Low-battery warning LED (Red)
#define PIN_PPM_RECEIVER     D9 // RC Receiver PPM signal input (Hardware Interrupt)

// Optional ELRS / CRSF UART pins (if using serial receiver instead of PPM)
#define PIN_ELRS_RX          D9
#define PIN_ELRS_TX          D10

// ==============================================================================
// 2. SERVO CALIBRATION & REFRESH (Servomy SDS1601 / C-Torque 9g HV / PTK 7465MG)
// ==============================================================================
#define SERVO_REFRESH_HZ     333   // High-speed digital servo frame rate (Hz)
#define SERVO_US_MIN         800   // Minimum physical pulse limit (us)
#define SERVO_US_MAX         2200  // Maximum physical pulse limit (us)
#define SERVO_US_CENTER      1520  // True electrical neutral for SDS1601 (us)
#define US_PER_DEGREE        11.11f// Microseconds per degree deflection (~1000us / 90deg)

// ==============================================================================
// 3. FLIGHT KINEMATICS & STEERING TUNING
// ==============================================================================
#define MIN_FLAP_FREQ_HZ     1.4f  // Flapping frequency at low throttle (Hz)
#define MAX_FLAP_FREQ_HZ     4.2f  // Flapping frequency at full throttle (Hz)

#define MIN_FLAP_AMP_DEG     20.0f // Minimum flapping half-amplitude (degrees)
#define MAX_FLAP_AMP_DEG     65.0f // Maximum flapping half-amplitude (degrees)
#define MAX_PITCH_TRIM_DEG   16.0f // Maximum symmetric pitch trim (degrees)
#define MAX_IDLE_LEAN_DEG    15.0f // Maximum resting wing tilt at zero throttle (degrees)
#define DIHEDRAL_GLIDE_DEG   18.0f // Upward "V" dihedral glide angle (degrees)

// Differential-Amplitude Steering parameters (Max Imagination architecture)
#define STEER_DIFF_MAX_FRAC  0.45f // Fraction of stroke amplitude shifted between wings
#define TURN_LIFT_BOOST_FRAC 0.15f // +15% flap amplitude boost during hard steering

// ==============================================================================
// 4. BATTERY MONITORING & VOLTAGE THRESHOLDS (2S LiPo 7.4V - 8.4V)
// ==============================================================================
// Voltage divider: 51kΩ top, 18kΩ bottom -> Divider ratio = (51 + 18) / 18 = 3.833
#define VBAT_DIVIDER_RATIO   3.833f
#define VBAT_WARNING_VOLTAGE 7.40f // Warning LED triggers below this pack voltage
#define VBAT_CRITICAL_CUTOFF 7.00f // Wings lock to neutral glide below this to save LiPo
#define VBAT_ADC_CALIBRATION 1.00f // Fine-tuning calibration multiplier

// ==============================================================================
// 5. WI-FI ACCESS POINT CREDENTIALS
// ==============================================================================
#define WIFI_AP_SSID         "Butterfly_RC"
#define WIFI_AP_PASSWORD     "12345678"
#define WIFI_AP_CHANNEL      1

// ==============================================================================
// 6. RC PPM RECEIVER SETTINGS
// ==============================================================================
#define PPM_CHANNEL_COUNT    8     // Typical 6-8 channel PPM receivers
#define PPM_TIMEOUT_MS       150   // Switch to Web App control if no PPM frame for 150ms
#define PPM_SYNC_MIN_US      3000  // Sync pulse gap threshold
#define PPM_PULSE_MIN_US     850   // Valid servo pulse minimum
#define PPM_PULSE_MAX_US     2150  // Valid servo pulse maximum
