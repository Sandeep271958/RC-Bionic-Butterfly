/*
 * ==============================================================================
 * PROJECT: ENHANCED RC BIONIC BUTTERFLY (BIOMIMETIC FLIGHT CONTROLLER v2.5)
 * BASED ON: S-DiY / TruongVanSu91, CTorque Architecture & Max Imagination ESP-FLUTTER
 * ==============================================================================
 *
 * KEY HIGHLIGHTS & NEW INNOVATIONS:
 * 1. DIFFERENTIAL-AMPLITUDE STEERING:
 *    Instead of merely shifting the mean servo angle, steering dynamically varies 
 *    the flapping amplitude between left and right wings (e.g. left flaps 80°, 
 *    right flaps 50°). This produces natural, sharp, high-agility yaw/roll turns!
 * 2. AUTOMATIC TURN LIFT BOOST:
 *    During banked turns, aerodynamic lift is tilted sideways. The controller 
 *    automatically injects a +15% flap amplitude boost during hard steering so 
 *    the butterfly maintains altitude without dipping or stalling.
 * 3. ZERO-THROTTLE IDLE LEAN:
 *    Below 5% throttle, stick inputs do not flap the wings; instead, they pose 
 *    the wings in a smooth, organic resting lean (up to ±15°).
 * 4. DETERMINISTIC MICROSECOND TIMEBASE (dt):
 *    Jitter-free flapping engine synchronized to real time, completely immune to 
 *    serial print overhead or receiver frame delays.
 * 5. HIGH-RESOLUTION MICROSECOND SERVO PULSES:
 *    Drives servos with Servo.writeMicroseconds(us) for buttery smooth stroke arcs 
 *    without 1-degree gear rattle.
 * 6. BIOMIMETIC ASYMMETRIC FLAP:
 *    Cycloidal power downstroke (generating high leading-edge vortex lift) and 
 *    cushioned recovery upstroke for maximum climb efficiency.
 * 7. GLOWING NIGHT FLIGHT LED EFFECTS:
 *    Drives ultra-light flexible LED filaments or status LED with Solid, Breathing, 
 *    and Strobe modes.
 * 8. FAILSAFE WITH AERODYNAMIC "V" DIHEDRAL GLIDE:
 *    Smoothly transitions wings into a positive dihedral (+18°) when signal is lost.
 * ==============================================================================
 */

#include <Servo.h>
#include <PPMReader.h>

// ==============================================================================
// 1. PIN CONFIGURATION (Select your wiring setup)
// ==============================================================================
// Set to 1 if wired according to updated code (Left = D4, Right = D5)
// Set to 0 if wired according to legacy circuit.jpeg (Left = D3, Right = D10)
#define USE_UPDATED_PINS 1

#if USE_UPDATED_PINS
  const byte PIN_PPM_RECEIVER = 2; // Receiver PPM output (ATmega32U4 INT1)
  const byte PIN_SERVO_LEFT   = 4; // Left wing servo signal
  const byte PIN_SERVO_RIGHT  = 5; // Right wing servo signal
#else
  const byte PIN_PPM_RECEIVER = 2; // Receiver PPM output (INT1)
  const byte PIN_SERVO_LEFT   = 3; // Left wing servo signal
  const byte PIN_SERVO_RIGHT  = 10;// Right wing servo signal
#endif

const byte PIN_STATUS_LED     = 6; // PWM pin for Status LED or Flexible LED filament
const byte PIN_VBAT_SENSE     = A0;// Optional: 2S battery voltage divider (49k/18k)
const byte PPM_CHANNEL_COUNT  = 8; // Standard 8CH RC receiver

// ==============================================================================
// 2. FLIGHT ENVELOPE & SERVO GEOMETRY TUNING
// ==============================================================================
const int SERVO_US_MIN        = 600;   // Physical minimum limit (microseconds)
const int SERVO_US_MAX        = 2400;  // Physical maximum limit (microseconds)
const int SERVO_US_CENTER     = 1500;  // Neutral / horizontal wing position

const float US_PER_DEGREE      = 11.11f; // Standard ~1000us span per 90 degrees
const float MAX_FLAP_AMP_DEG   = 65.0f;  // Max flapping half-amplitude (degrees)
const float MIN_FLAP_AMP_DEG   = 20.0f;  // Soft-flutter half-amplitude (degrees)
const float MAX_PITCH_DEG      = 16.0f;  // Max symmetric pitch trim (degrees)
const float DIHEDRAL_GLIDE_DEG = 18.0f;  // Upward V-angle during glide (degrees)
const float MAX_IDLE_LEAN_DEG  = 15.0f;  // Max resting wing tilt at zero throttle

// Steering Parameters (Max Imagination Differential Amplitude Architecture)
const float STEER_DIFF_MAX_FRAC  = 0.45f; // Max fraction of flap amplitude shifted for steering
const float TURN_LIFT_BOOST_FRAC = 0.15f; // +15% flap boost at full stick to prevent altitude loss

// Flapping Frequency Limits (Hz)
const float MIN_FLAP_FREQ_HZ   = 1.4f;   // Flapping frequency at low throttle
const float MAX_FLAP_FREQ_HZ   = 4.2f;   // Flapping frequency at full throttle

// Flight Kinematics Mode: 0 = Pure Sine, 1 = Biomimetic Asymmetric Power Stroke
int flightMode = 1;

// ==============================================================================
// 3. OBJECTS & STATE VARIABLES
// ==============================================================================
PPMReader ppm(PIN_PPM_RECEIVER, PPM_CHANNEL_COUNT);
Servo servoLeft, servoRight;

// Timing variables
unsigned long lastMicros = 0;
unsigned long lastLogMillis = 0;
const unsigned long LOG_INTERVAL_MS = 1000;

// Kinematic phase accumulators
float flapPhase = 0.0f;
float perchPhase = 0.0f;
float ledPhase = 0.0f;

// Smoothed states
float currentLeftFlapUs = 0.0f;
float currentRightFlapUs = 0.0f;
float targetLeftFlapUs = 0.0f;
float targetRightFlapUs = 0.0f;
float smoothedSteerNorm = 0.0f;

// Filtered RC Channel values
int rcChannels[8] = {1500, 1500, 1000, 1500, 1500, 1500, 1500, 1500};
const int PPM_DEADBAND = 4;

// Flight state flags
bool isFailsafe = true;
bool isPerching = false;
int ledMode = 1; // 0=Off, 1=Breathing/Solid, 2=Strobe

// ==============================================================================
// 4. SETUP
// ==============================================================================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_STATUS_LED, OUTPUT);
  analogWrite(PIN_STATUS_LED, 128); // Initial soft glow

  servoLeft.attach(PIN_SERVO_LEFT, SERVO_US_MIN, SERVO_US_MAX);
  servoRight.attach(PIN_SERVO_RIGHT, SERVO_US_MIN, SERVO_US_MAX);

  // Hold center neutral during power-up
  servoLeft.writeMicroseconds(SERVO_US_CENTER);
  servoRight.writeMicroseconds(SERVO_US_CENTER);

  lastMicros = micros();

  Serial.println(F("=========================================================="));
  Serial.println(F("  RC BIONIC BUTTERFLY CONTROLLER v2.5 (DIFFERENTIAL FLAP) "));
  Serial.print  (F("  Left Pin: ")); Serial.print(PIN_SERVO_LEFT);
  Serial.print  (F(" | Right Pin: ")); Serial.println(PIN_SERVO_RIGHT);
  Serial.println(F("=========================================================="));
}

// ==============================================================================
// 5. HELPER: EXPONENTIAL FILTER & INPUT READ
// ==============================================================================
int readChannelFiltered(byte ch, int defaultVal) {
  int raw = ppm.latestValidChannelValue(ch + 1, defaultVal);
  if (raw < 800 || raw > 2200) raw = defaultVal;
  if (abs(raw - rcChannels[ch]) > PPM_DEADBAND) {
    rcChannels[ch] = (rcChannels[ch] * 3 + raw) / 4; // 4-tap EMA smoothing
  }
  return rcChannels[ch];
}

// Cubic exponential curve for fine control near center stick
float applyExpo(int inputUs, float maxDeflection, float expoFactor) {
  int deviation = inputUs - 1500;
  if (abs(deviation) < 15) return 0.0f; // Deadband

  float normalized = (float)deviation / 500.0f;
  normalized = constrain(normalized, -1.0f, 1.0f);

  float shaped = (1.0f - expoFactor) * normalized + expoFactor * (normalized * normalized * normalized);
  return shaped * maxDeflection;
}

// ==============================================================================
// 6. MAIN LOOP
// ==============================================================================
void loop() {
  unsigned long nowMicros = micros();
  float dt = (nowMicros - lastMicros) * 1e-6f;
  if (dt <= 0.0f || dt > 0.1f) dt = 0.004f;
  lastMicros = nowMicros;

  // Update LED lighting effects
  updateLedEffects(dt);

  // Check receiver connection status (CH3 = Throttle)
  unsigned long ch3Raw = ppm.latestValidChannelValue(3, 0);
  bool signalOk = (ch3Raw >= 900 && ch3Raw <= 2100);

  if (!signalOk) {
    handleFailsafe(dt);
  } else {
    // Read RC Channels
    int chSteering = readChannelFiltered(0, 1500); // CH1: Steering
    int chElevator = readChannelFiltered(1, 1500); // CH2: Pitch
    int chThrottle = readChannelFiltered(2, 1000); // CH3: Throttle
    int chAuxTrim  = readChannelFiltered(4, 1500); // CH5: Steering fine trim pot
    int chAuxPitch = readChannelFiltered(5, 1500); // CH6: Pitch bias pot
    int chSwitch7  = readChannelFiltered(6, 1000); // CH7: Perch mode switch
    int chAux8     = readChannelFiltered(7, 1000); // CH8: LED mode & Flight mode

    flightMode = (chAux8 > 1750) ? 0 : 1; // Pure Sine vs Biomimetic
    ledMode    = (chAux8 > 1500) ? 2 : 1; // Strobe vs Breathing

    // Steering input: -1.0 (full left) to +1.0 (full right)
    float steerInput = applyExpo(chSteering, 1.0f, 0.35f);
    float steerTrim  = (float)(chAuxTrim - 1500) / 500.0f * 0.3f;
    float targetSteerNorm = constrain(steerInput + steerTrim, -1.0f, 1.0f);

    // Smooth steering transitions (eliminates sudden servo jerks)
    smoothedSteerNorm += (targetSteerNorm - smoothedSteerNorm) * constrain(dt * 20.0f, 0.1f, 1.0f);

    // Symmetric Pitch bias (climb vs dive)
    float elevDeg  = applyExpo(chElevator, MAX_PITCH_DEG, 0.30f);
    float pitchDeg = ((float)(chAuxPitch - 1500) / 500.0f) * MAX_PITCH_DEG;
    float pitchOffsetUs = (elevDeg + pitchDeg) * US_PER_DEGREE;

    // Throttle normalization (0.0 to 1.0)
    float thrNorm = (float)(chThrottle - 1050) / 950.0f;
    thrNorm = constrain(thrNorm, 0.0f, 1.0f);

    // Flight State Selection
    if (chThrottle > 1100) {
      isPerching = false;
    } else if (chSwitch7 > 1600 && chThrottle < 1100) {
      isPerching = true;
    }
    if (chSwitch7 < 1400) isPerching = false;

    if (isPerching) {
      updatePerchState(chAuxPitch, dt);
    } else if (thrNorm < 0.05f) {
      // ZERO-THROTTLE: IDLE LEAN & GLIDE
      updateIdleLean(smoothedSteerNorm, dt);
    } else {
      // ACTIVE FLIGHT: DIFFERENTIAL-AMPLITUDE FLAPPING
      updateDifferentialFlapping(thrNorm, smoothedSteerNorm, dt);
    }

    // Dynamic Headroom & Anti-Clipping Protection
    float availHeadroom = (SERVO_US_MAX - SERVO_US_CENTER) - abs(pitchOffsetUs) - 40.0f;
    if (availHeadroom < (MIN_FLAP_AMP_DEG * US_PER_DEGREE)) {
      availHeadroom = MIN_FLAP_AMP_DEG * US_PER_DEGREE;
    }

    float safeLeftFlapUs  = constrain(currentLeftFlapUs,  -availHeadroom, availHeadroom);
    float safeRightFlapUs = constrain(currentRightFlapUs, -availHeadroom, availHeadroom);

    // Output Servo Pulses:
    // Flapping: Left (+), Right (-) -> Wings flap together up and down
    // Pitch:    Left (+), Right (-) -> Both wings bias up/down together
    float pulseLeft  = SERVO_US_CENTER + safeLeftFlapUs  + pitchOffsetUs;
    float pulseRight = SERVO_US_CENTER - safeRightFlapUs - pitchOffsetUs;

    pulseLeft  = constrain(pulseLeft,  SERVO_US_MIN, SERVO_US_MAX);
    pulseRight = constrain(pulseRight, SERVO_US_MIN, SERVO_US_MAX);

    servoLeft.writeMicroseconds((int)pulseLeft);
    servoRight.writeMicroseconds((int)pulseRight);

    isFailsafe = false;
  }

  // Diagnostic Telemetry
  if (millis() - lastLogMillis > LOG_INTERVAL_MS) {
    lastLogMillis = millis();
    if (signalOk) {
      Serial.print(F("[OK] THR: ")); Serial.print(rcChannels[2]);
      Serial.print(F(" | STEER: ")); Serial.print(smoothedSteerNorm, 2);
      Serial.print(F(" | FLAP_L: ")); Serial.print((int)currentLeftFlapUs);
      Serial.print(F(" | FLAP_R: ")); Serial.println((int)currentRightFlapUs);
    } else {
      Serial.println(F("[!] FAILSAFE: NO VALID PPM SIGNAL DETECTED"));
    }
  }
}

// ==============================================================================
// 7. DIFFERENTIAL AMPLITUDE KINEMATICS ENGINE
// ==============================================================================
void updateDifferentialFlapping(float thrNorm, float steerNorm, float dt) {
  // 1. Dynamic Frequency Modulation (1.4 Hz to 4.2 Hz)
  float currentFreq = MIN_FLAP_FREQ_HZ + thrNorm * (MAX_FLAP_FREQ_HZ - MIN_FLAP_FREQ_HZ);

  // 2. Base Amplitude Modulation (20° to 65°)
  float baseAmpDeg = MIN_FLAP_AMP_DEG + thrNorm * (MAX_FLAP_AMP_DEG - MIN_FLAP_AMP_DEG);

  // 3. Turn Lift Boost: hard steering automatically adds up to +15% extra flap
  //    amplitude to prevent the butterfly from losing altitude while banked!
  float turnLiftBoost = 1.0f + (abs(steerNorm) * TURN_LIFT_BOOST_FRAC);
  baseAmpDeg *= turnLiftBoost;

  // 4. Differential Amplitude Allocation:
  //    Steering right (+steerNorm): Left wing flaps LARGER, Right wing flaps SMALLER.
  //    Steering left  (-steerNorm): Right wing flaps LARGER, Left wing flaps SMALLER.
  float maxDeltaDeg = baseAmpDeg * STEER_DIFF_MAX_FRAC;
  float wingDeltaDeg = steerNorm * maxDeltaDeg;

  float leftAmpDeg  = baseAmpDeg + wingDeltaDeg;
  float rightAmpDeg = baseAmpDeg - wingDeltaDeg;

  leftAmpDeg  = constrain(leftAmpDeg,  10.0f, MAX_FLAP_AMP_DEG + 10.0f);
  rightAmpDeg = constrain(rightAmpDeg, 10.0f, MAX_FLAP_AMP_DEG + 10.0f);

  float leftAmpUs  = leftAmpDeg  * US_PER_DEGREE;
  float rightAmpUs = rightAmpDeg * US_PER_DEGREE;

  // 5. Advance Phase Smoothly with exact Delta Time
  flapPhase += 2.0f * PI * currentFreq * dt;
  if (flapPhase >= 2.0f * PI) flapPhase -= 2.0f * PI;

  // 6. Waveform Synthesis: Pure Sine vs Biomimetic Asymmetric Power Stroke
  float wave = 0.0f;
  if (flightMode == 0) {
    wave = sin(flapPhase);
  } else {
    // Cycloidal warping produces a punchy, vortex-generating power downstroke
    wave = sin(flapPhase + 0.35f * sin(flapPhase));
  }

  targetLeftFlapUs  = wave * leftAmpUs;
  targetRightFlapUs = wave * rightAmpUs;

  // Smooth filtering to prevent harsh servo acceleration shocks
  currentLeftFlapUs  += (targetLeftFlapUs  - currentLeftFlapUs)  * constrain(dt * 30.0f, 0.1f, 1.0f);
  currentRightFlapUs += (targetRightFlapUs - currentRightFlapUs) * constrain(dt * 30.0f, 0.1f, 1.0f);
}

// Zero-throttle Idle Lean: stick tilts wings into a resting pose without flapping
void updateIdleLean(float steerNorm, float dt) {
  float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE;
  float leanUs  = (steerNorm * MAX_IDLE_LEAN_DEG) * US_PER_DEGREE;

  // During idle lean, both wings tilt in unison
  float targetL = glideUs - leanUs;
  float targetR = glideUs + leanUs;

  currentLeftFlapUs  += (targetL - currentLeftFlapUs)  * constrain(dt * 15.0f, 0.05f, 1.0f);
  currentRightFlapUs += (targetR - currentRightFlapUs) * constrain(dt * 15.0f, 0.05f, 1.0f);
}

// Perch Mode: gentle fluttering while resting on a branch or hand
void updatePerchState(int rawCh6, float dt) {
  float perchMaxDeg = map(rawCh6, 1000, 2000, 30, 55);
  perchPhase += 1.8f * dt;
  if (perchPhase >= 2.0f * PI) perchPhase -= 2.0f * PI;

  float perchOffsetDeg = perchMaxDeg + (sin(perchPhase) * 12.0f) - 12.0f;
  float goalUs = perchOffsetDeg * US_PER_DEGREE;

  currentLeftFlapUs  += (goalUs - currentLeftFlapUs)  * constrain(dt * 8.0f, 0.05f, 1.0f);
  currentRightFlapUs += (goalUs - currentRightFlapUs) * constrain(dt * 8.0f, 0.05f, 1.0f);
}

// Failsafe Mode: holds positive dihedral glide posture (+18°)
void handleFailsafe(float dt) {
  float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE;
  currentLeftFlapUs  += (glideUs - currentLeftFlapUs)  * constrain(dt * 8.0f, 0.05f, 1.0f);
  currentRightFlapUs += (glideUs - currentRightFlapUs) * constrain(dt * 8.0f, 0.05f, 1.0f);

  int outL = SERVO_US_CENTER + (int)currentLeftFlapUs;
  int outR = SERVO_US_CENTER - (int)currentRightFlapUs;
  servoLeft.writeMicroseconds(constrain(outL, SERVO_US_MIN, SERVO_US_MAX));
  servoRight.writeMicroseconds(constrain(outR, SERVO_US_MIN, SERVO_US_MAX));
}

// LED Lighting Effects (Flexible LED Filaments or Status LED)
void updateLedEffects(float dt) {
  if (ledMode == 0) {
    analogWrite(PIN_STATUS_LED, 0);
  } else if (ledMode == 1) {
    // Breathing Glow Effect
    ledPhase += 2.5f * dt;
    if (ledPhase >= 2.0f * PI) ledPhase -= 2.0f * PI;
    int brightness = (int)((sin(ledPhase) + 1.0f) * 110.0f + 35.0f);
    analogWrite(PIN_STATUS_LED, brightness);
  } else {
    // Fast Strobe Effect
    ledPhase += 15.0f * dt;
    if (ledPhase >= 2.0f * PI) ledPhase -= 2.0f * PI;
    analogWrite(PIN_STATUS_LED, (sin(ledPhase) > 0.0f) ? 255 : 0);
  }
}
