/*
 * ==============================================================================
 * PROJECT: ESP32 BIONIC BUTTERFLY FLIGHT CONTROLLER (DUAL WEB APP & PPM RC)
 * HARDWARE: Seeed Studio XIAO ESP32-C3 / S3
 * ARCHITECTURE: Max Imagination ESP-FLUTTER & CTorque High-Speed Kinematics
 * ==============================================================================
 *
 * DUAL PILOTING INTERFACES:
 * 1. SMARTPHONE WEB APP: Connect to Wi-Fi "Butterfly_RC" (password: 12345678)
 *    and open http://192.168.4.1 for a full touch cockpit.
 * 2. TRADITIONAL PPM RC CONTROLLER: Connect your micro RC receiver's PPM wire
 *    to pin D9. The system automatically detects valid PPM pulses and gives
 *    instant priority to your physical transmitter!
 *
 * AERODYNAMIC & KINEMATIC FEATURES:
 * - 333Hz High-Speed Digital Servo Refresh (Servomy SDS1601 / C-Torque 9g HV)
 * - Differential-Amplitude Steering (modulates stroke depth between wings)
 * - Automatic Turn Lift Boost (+15% flap boost during hard banking)
 * - Zero-Throttle Idle Lean (poses wings in organic tilt at rest)
 * - 2S LiPo ADC Voltage Sensing with Low-Voltage Auto-Neutral Cutoff (< 7.0V)
 * - Night-Flight Flexible LED Filament Modes (Solid, Breathing Glow, Strobe)
 * ==============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <math.h>

#include "config.h"
#include "web_ui.h"
#include "ppm_receiver.h"

// ==============================================================================
// 1. HARDWARE OBJECTS & TIMING
// ==============================================================================
WebServer server(80);
Servo servoLeft, servoRight;
PpmReceiver ppm;

unsigned long lastLoopMicros = 0;
unsigned long lastWebPacketMillis = 0;
unsigned long lastAdcMillis = 0;
unsigned long lastTelemetryMillis = 0;

// ==============================================================================
// 2. FLIGHT CONTROL COMMANDS & STATES
// ==============================================================================
// Active flight commands (normalized -1.0 to +1.0, or 0.0 to 1.0)
float cmdThrottle  = 0.0f;  // 0.0 to 1.0
float cmdSteering  = 0.0f;  // -1.0 (left) to +1.0 (right)
float cmdPitch     = 0.0f;  // -1.0 (dive) to +1.0 (climb)
bool  cmdArmed     = true;
bool  cmdPerch     = false;
int   cmdLedMode   = 1;     // 0=Off, 1=Breathing Glow, 2=Strobe
const char* activeSource = "WEB"; // "PPM" or "WEB"

// Web App targets
float webThrottle  = 0.0f;
float webSteering  = 0.0f;
float webPitch     = 0.0f;
bool  webArmed     = true;
bool  webPerch     = false;
int   webLedMode   = 1;

// Kinematic accumulators & smoothed values
float flapPhase      = 0.0f;
float perchPhase     = 0.0f;
float ledPhase       = 0.0f;
float currentLeftUs  = 0.0f;
float currentRightUs = 0.0f;
float targetLeftUs   = 0.0f;
float targetRightUs  = 0.0f;
float smoothedSteer  = 0.0f;
float currentFreqHz  = 0.0f;
float batteryVoltage = 8.2f;

// ==============================================================================
// 3. WEB SERVER REQUEST HANDLERS
// ==============================================================================
void handleRoot() {
  server.send_P(200, "text/html", PAGE_INDEX);
}

void handleCommand() {
  if (server.hasArg("t"))  webThrottle = server.arg("t").toFloat();
  if (server.hasArg("s"))  webSteering = server.arg("s").toFloat();
  if (server.hasArg("p"))  webPitch    = server.arg("p").toFloat();
  if (server.hasArg("l"))  webLedMode  = server.arg("l").toInt();
  if (server.hasArg("a"))  webArmed    = (server.arg("a").toInt() == 1);
  if (server.hasArg("pr")) webPerch    = (server.arg("pr").toInt() == 1);

  lastWebPacketMillis = millis();

  // Return telemetry JSON
  char json[96];
  snprintf(json, sizeof(json),
           "{\"vbat\":%.2f,\"src\":\"%s\",\"freq\":%.2f}",
           batteryVoltage, activeSource, currentFreqHz);
  server.send(200, "application/json", json);
}

// ==============================================================================
// 4. SETUP
// ==============================================================================
void setup() {
  Serial.begin(115200);

  // Initialize GPIO pins
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_LOW_BAT_LED, OUTPUT);
  pinMode(PIN_LED_FILAMENT, OUTPUT);
  pinMode(PIN_BATTERY_ADC, INPUT);

  digitalWrite(PIN_STATUS_LED, LOW);
  digitalWrite(PIN_LOW_BAT_LED, LOW);
  analogWrite(PIN_LED_FILAMENT, 128); // Soft glow

  // Allocate ESP32 hardware PWM timers for 333Hz refresh
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoLeft.setPeriodHertz(SERVO_REFRESH_HZ);
  servoRight.setPeriodHertz(SERVO_REFRESH_HZ);

  servoLeft.attach(PIN_SERVO_LEFT, SERVO_US_MIN, SERVO_US_MAX);
  servoRight.attach(PIN_SERVO_RIGHT, SERVO_US_MIN, SERVO_US_MAX);

  // Hold center neutral during power-up
  servoLeft.writeMicroseconds(SERVO_US_CENTER);
  servoRight.writeMicroseconds(SERVO_US_CENTER);

  // Initialize hardware interrupt PPM receiver on Pin D9
  ppm.begin(PIN_PPM_RECEIVER);

  // Start Wi-Fi Soft Access Point
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.begin();

  Serial.println(F("=============================================================="));
  Serial.println(F("  ESP32 BIONIC BUTTERFLY FLIGHT CONTROLLER (WEB + PPM DUAL)   "));
  Serial.print  (F("  Wi-Fi AP: '")); Serial.print(WIFI_AP_SSID);
  Serial.print  (F("' | IP: http://")); Serial.println(WiFi.softAPIP());
  Serial.print  (F("  PPM RX Pin: ")); Serial.print(PIN_PPM_RECEIVER);
  Serial.print  (F(" | Left Servo: ")); Serial.print(PIN_SERVO_LEFT);
  Serial.print  (F(" | Right Servo: ")); Serial.println(PIN_SERVO_RIGHT);
  Serial.println(F("=============================================================="));

  lastLoopMicros = micros();
  lastWebPacketMillis = millis();
}

// ==============================================================================
// 5. HELPER: EXPONENTIAL SHAPING
// ==============================================================================
float applyExpo(float inputNorm, float expoFactor) {
  return (1.0f - expoFactor) * inputNorm + expoFactor * (inputNorm * inputNorm * inputNorm);
}

// ==============================================================================
// 6. MAIN LOOP
// ==============================================================================
void loop() {
  server.handleClient();

  unsigned long nowMicros = micros();
  float dt = (nowMicros - lastLoopMicros) * 1e-6f;
  if (dt <= 0.0f || dt > 0.05f) dt = 0.003f;
  lastLoopMicros = nowMicros;

  // 1. Read 2S Battery Pack Voltage (every 400ms)
  if (millis() - lastAdcMillis > 400) {
    lastAdcMillis = millis();
    int raw = analogRead(PIN_BATTERY_ADC);
    float pinV = (raw / 4095.0f) * 3.3f;
    batteryVoltage = pinV * VBAT_DIVIDER_RATIO * VBAT_ADC_CALIBRATION;

    // Red LED triggers when pack drops below warning threshold
    digitalWrite(PIN_LOW_BAT_LED, (batteryVoltage < VBAT_WARNING_VOLTAGE) ? HIGH : LOW);
  }

  // 2. Control Source Arbitration (PPM RC Controller vs Smartphone Web App)
  bool ppmActive = ppm.isConnected();

  if (ppmActive) {
    activeSource = "PPM";
    digitalWrite(PIN_STATUS_LED, HIGH);

    // Read RC Channels (CH1 Steer, CH2 Pitch, CH3 Throttle, CH5 Arm, CH6 Bias, CH7 Perch, CH8 LED)
    uint16_t rawSteer = ppm.getChannel(1, 1500);
    uint16_t rawPitch = ppm.getChannel(2, 1500);
    uint16_t rawThr   = ppm.getChannel(3, 1000);
    uint16_t rawArm   = ppm.getChannel(5, 1500);
    uint16_t rawBias  = ppm.getChannel(6, 1500);
    uint16_t rawPerch = ppm.getChannel(7, 1000);
    uint16_t rawLed   = ppm.getChannel(8, 1000);

    // Normalize inputs
    cmdThrottle = constrain((float)(rawThr - 1050) / 950.0f, 0.0f, 1.0f);
    
    float steerDev = (float)((int)rawSteer - 1500) / 500.0f;
    cmdSteering = applyExpo(constrain(steerDev, -1.0f, 1.0f), 0.35f);

    float pitchDev = (float)((int)rawPitch - 1500) / 500.0f;
    float biasDev  = (float)((int)rawBias  - 1500) / 500.0f * 0.4f;
    cmdPitch = constrain(pitchDev + biasDev, -1.0f, 1.0f);

    cmdArmed   = (rawArm > 1300);
    cmdPerch   = (rawPerch > 1600 && cmdThrottle < 0.08f);
    cmdLedMode = (rawLed > 1650) ? 2 : ((rawLed > 1350) ? 1 : 0);

  } else {
    activeSource = "WEB";

    // Check Web App connection timeout (failsafe if phone disconnects > 1.5s)
    bool webConnected = (millis() - lastWebPacketMillis < 1500);

    if (webConnected) {
      cmdThrottle = webThrottle;
      cmdSteering = applyExpo(webSteering, 0.35f);
      cmdPitch    = webPitch;
      cmdArmed    = webArmed;
      cmdPerch    = webPerch;
      cmdLedMode  = webLedMode;
      digitalWrite(PIN_STATUS_LED, LOW);
    } else {
      // Full Signal Loss Failsafe
      cmdThrottle = 0.0f;
      cmdSteering = 0.0f;
      cmdPitch    = 0.0f;
      cmdArmed    = false;
      cmdPerch    = false;
    }
  }

  // Smooth steering filter (eliminates sudden servo shock)
  smoothedSteer += (cmdSteering - smoothedSteer) * constrain(dt * 20.0f, 0.1f, 1.0f);

  // 3. Update Night Flight LED Driver
  updateLedEffects(cmdLedMode, dt);

  // 4. Flight State Machine
  bool batteryCritical = (batteryVoltage < VBAT_CRITICAL_CUTOFF);

  if (!cmdArmed || batteryCritical) {
    // DISARMED OR LOW-VOLTAGE CUTOFF: Hold wings in horizontal neutral
    float neutralUs = SERVO_US_CENTER;
    currentLeftUs  += (0.0f - currentLeftUs)  * constrain(dt * 10.0f, 0.05f, 1.0f);
    currentRightUs += (0.0f - currentRightUs) * constrain(dt * 10.0f, 0.05f, 1.0f);
    currentFreqHz   = 0.0f;

  } else if (cmdPerch) {
    // PERCH MODE: Gentle resting flutter
    updatePerchMotion(dt);

  } else if (cmdThrottle < 0.05f) {
    // ZERO-THROTTLE: IDLE LEAN & AERODYNAMIC GLIDE
    updateIdleLean(smoothedSteer, dt);
    currentFreqHz = 0.0f;

  } else {
    // ACTIVE FLIGHT: HIGH-SPEED DIFFERENTIAL-AMPLITUDE KINEMATICS
    updateDifferentialFlap(cmdThrottle, smoothedSteer, dt);
  }

  // 5. Symmetric Pitch Bias (Climb vs Dive)
  float pitchUs = (cmdPitch * MAX_PITCH_TRIM_DEG) * US_PER_DEGREE;

  // 6. Output Servo Pulses:
  // Left and Right servos are mirror-mounted:
  // Flapping: Left (+), Right (-) -> Wings move together up and down
  // Pitch:    Left (+), Right (-) -> Both wings bias up/down together
  float pulseL = SERVO_US_CENTER + currentLeftUs  + pitchUs;
  float pulseR = SERVO_US_CENTER - currentRightUs - pitchUs;

  pulseL = constrain(pulseL, SERVO_US_MIN, SERVO_US_MAX);
  pulseR = constrain(pulseR, SERVO_US_MIN, SERVO_US_MAX);

  servoLeft.writeMicroseconds((int)pulseL);
  servoRight.writeMicroseconds((int)pulseR);

  // 7. Diagnostic Telemetry Stream (every 1s)
  if (millis() - lastTelemetryMillis > 1000) {
    lastTelemetryMillis = millis();
    Serial.print(F("[")); Serial.print(activeSource); Serial.print(F("] "));
    Serial.print(F("THR: ")); Serial.print((int)(cmdThrottle * 100)); Serial.print(F("%"));
    Serial.print(F(" | STEER: ")); Serial.print(cmdSteering, 2);
    Serial.print(F(" | VBAT: ")); Serial.print(batteryVoltage, 2); Serial.print(F("V"));
    Serial.print(F(" | FREQ: ")); Serial.print(currentFreqHz, 1); Serial.println(F("Hz"));
  }
}

// ==============================================================================
// 7. KINEMATIC WAVE ENGINES
// ==============================================================================

// Active High-Performance Differential Flapping Engine
void updateDifferentialFlap(float thr, float steer, float dt) {
  // 1. Dynamic Frequency Scaling (1.4 Hz to 4.2 Hz)
  currentFreqHz = MIN_FLAP_FREQ_HZ + thr * (MAX_FLAP_FREQ_HZ - MIN_FLAP_FREQ_HZ);

  // 2. Base Amplitude Scaling (20° to 65°)
  float baseAmpDeg = MIN_FLAP_AMP_DEG + thr * (MAX_FLAP_AMP_DEG - MIN_FLAP_AMP_DEG);

  // 3. Turn Lift Boost: Hard steering adds +15% extra flap amplitude to prevent altitude dip
  float turnLiftBoost = 1.0f + (fabsf(steer) * TURN_LIFT_BOOST_FRAC);
  baseAmpDeg *= turnLiftBoost;

  // 4. Differential Stroke Allocation:
  // When steering right (+steer): Left wing flaps deeper, Right wing flaps shallower
  // When steering left  (-steer): Right wing flaps deeper, Left wing flaps shallower
  float maxDeltaDeg = baseAmpDeg * STEER_DIFF_MAX_FRAC;
  float wingDeltaDeg = steer * maxDeltaDeg;

  float leftAmpDeg  = constrain(baseAmpDeg + wingDeltaDeg, 10.0f, MAX_FLAP_AMP_DEG + 10.0f);
  float rightAmpDeg = constrain(baseAmpDeg - wingDeltaDeg, 10.0f, MAX_FLAP_AMP_DEG + 10.0f);

  float leftAmpUs  = leftAmpDeg  * US_PER_DEGREE;
  float rightAmpUs = rightAmpDeg * US_PER_DEGREE;

  // 5. Real-Time Phase Advancement
  flapPhase += 2.0f * PI * currentFreqHz * dt;
  if (flapPhase >= 2.0f * PI) flapPhase -= 2.0f * PI;

  // 6. Biomimetic Cycloidal Waveform (Punchy vortex downstroke, gentle recovery upstroke)
  float wave = sin(flapPhase + 0.35f * sin(flapPhase));

  targetLeftUs  = wave * leftAmpUs;
  targetRightUs = wave * rightAmpUs;

  // Smooth filter
  currentLeftUs  += (targetLeftUs  - currentLeftUs)  * constrain(dt * 30.0f, 0.1f, 1.0f);
  currentRightUs += (targetRightUs - currentRightUs) * constrain(dt * 30.0f, 0.1f, 1.0f);
}

// Zero-Throttle Idle Lean: Tilts wings into an organic resting pose
void updateIdleLean(float steer, float dt) {
  float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE;
  float leanUs  = (steer * MAX_IDLE_LEAN_DEG) * US_PER_DEGREE;

  float tgtL = glideUs - leanUs;
  float tgtR = glideUs + leanUs;

  currentLeftUs  += (tgtL - currentLeftUs)  * constrain(dt * 15.0f, 0.05f, 1.0f);
  currentRightUs += (tgtR - currentRightUs) * constrain(dt * 15.0f, 0.05f, 1.0f);
}

// Perch Mode: Gentle organic fluttering while resting
void updatePerchMotion(float dt) {
  perchPhase += 1.8f * dt;
  if (perchPhase >= 2.0f * PI) perchPhase -= 2.0f * PI;

  float perchOffsetDeg = 40.0f + (sin(perchPhase) * 12.0f) - 12.0f;
  float goalUs = perchOffsetDeg * US_PER_DEGREE;

  currentLeftUs  += (goalUs - currentLeftUs)  * constrain(dt * 8.0f, 0.05f, 1.0f);
  currentRightUs += (goalUs - currentRightUs) * constrain(dt * 8.0f, 0.05f, 1.0f);
  currentFreqHz   = 0.6f;
}

// Night-Flight Flexible LED Filament Driver
void updateLedEffects(int mode, float dt) {
  if (mode == 0) {
    analogWrite(PIN_LED_FILAMENT, 0); // Off
  } else if (mode == 1) {
    // Breathing Glow Effect
    ledPhase += 2.5f * dt;
    if (ledPhase >= 2.0f * PI) ledPhase -= 2.0f * PI;
    int brightness = (int)((sin(ledPhase) + 1.0f) * 110.0f + 35.0f);
    analogWrite(PIN_LED_FILAMENT, brightness);
  } else {
    // Night Strobe Effect
    ledPhase += 16.0f * dt;
    if (ledPhase >= 2.0f * PI) ledPhase -= 2.0f * PI;
    analogWrite(PIN_LED_FILAMENT, (sin(ledPhase) > 0.0f) ? 255 : 0);
  }
}
