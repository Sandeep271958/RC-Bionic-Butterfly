/*
 * ==============================================================================
 * PROJECT: ENHANCED RC BIONIC BUTTERFLY (BIOMIMETIC FLIGHT CONTROLLER)
 * BASED ON: S-DiY / TruongVanSu91 & CTorque Architecture
 * VERSION: 2.0 (HIGH-RESPONSIVENESS & ANTI-CLIPPING EDITION)
 * ==============================================================================
 *
 * KEY IMPROVEMENTS OVER VERSION 1.0:
 * 1. Jitter-free microsecond delta-time (dt) flapping generator (no loop jitter).
 * 2. High-resolution microsecond servo pulses (Servo.writeMicroseconds) for
 *    buttery smooth wing motion without 1-degree servo gear chatter.
 * 3. Dynamic Throttle Scaling: Scales BOTH flapping frequency (1.2 - 4.2 Hz)
 *    and amplitude (25° - 65°), giving crisp, immediate throttle responsiveness.
 * 4. Biomimetic Asymmetric Downstroke: Faster power downstroke (vortex lift)
 *    and cushioned recovery upstroke for real butterfly flight efficiency.
 * 5. Dynamic Headroom & Anti-Clipping: Offsets from steering and pitch automatically
 *    soft-compress flap amplitude near travel limits to prevent servo stalling.
 * 6. Aerodynamic Dihedral Gliding Failsafe: Glides in a stable "V" dihedral 
 *    rather than flat stalling when signal is lost.
 * 7. Expo (Exponential) stick curves for silky smooth center-stick trim and 
 *    aggressive full-deflection turn agility.
 * 8. Pin Config Switcher: Pre-configured for either Updated Code (Pins 4 & 5) 
 *    or Legacy Schematic (Pins 3 & 10).
 * ==============================================================================
 */

#include <Servo.h>
#include <PPMReader.h>

// ==============================================================================
// 1. PIN CONFIGURATION (Select your wiring setup)
// ==============================================================================
// Set to 1 if wired according to code comments (Left = D4, Right = D5)
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

const byte PIN_STATUS_LED     = 6; // Onboard or external status LED
const byte PIN_VBAT_SENSE     = A0;// Optional: Battery voltage divider (optional)
const byte PPM_CHANNEL_COUNT  = 8; // FlySky FS-i6 / FS-A8S typical channels

// ==============================================================================
// 2. FLIGHT ENVELOPE & SERVO GEOMETRY TUNING
// ==============================================================================
// Microsecond timing calibration (standard 1500us center, 500-2500us range)
const int SERVO_US_MIN        = 600;   // Physical minimum limit
const int SERVO_US_MAX        = 2400;  // Physical maximum limit
const int SERVO_US_CENTER     = 1500;  // Neutral / horizontal wing position

// Flapping limits (in degrees converted to microseconds)
// Standard 1000us span ~ 90 degrees (approx 11.1 us per degree)
const float US_PER_DEGREE      = 11.11f; 
const float MAX_FLAP_AMP_DEG   = 65.0f; // Max flapping half-amplitude (degrees)
const float MIN_FLAP_AMP_DEG   = 20.0f; // Soft-flutter half-amplitude (degrees)
const float MAX_STEER_DEG      = 18.0f; // Max differential roll trim (degrees)
const float MAX_PITCH_DEG      = 18.0f; // Max symmetric pitch trim (degrees)
const float DIHEDRAL_GLIDE_DEG = 18.0f; // Upward V-angle during glide (degrees)

// Frequency limits (Hz)
const float MIN_FLAP_FREQ_HZ   = 1.4f;  // Flapping frequency at low throttle
const float MAX_FLAP_FREQ_HZ   = 4.2f;  // Flapping frequency at full throttle

// Flight Kinematics Mode:
// 0 = Pure Symmetrical Sine Wave
// 1 = Biomimetic Asymmetric Power Stroke (Faster downstroke, higher lift)
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

// Kinematic phase accumulator
float flapPhase = 0.0f;
float currentFlapOffsetUs = 0.0f;
float targetFlapOffsetUs = 0.0f;
float perchPhase = 0.0f;

// Filtered RC Channel values
int rcChannels[8] = {1500, 1500, 1000, 1500, 1500, 1500, 1500, 1500};
const int PPM_DEADBAND = 4;

// Flight state flags
bool isFailsafe = true;
bool isPerching = false;
int ledDebounce = 0;

// ==============================================================================
// 4. SETUP
// ==============================================================================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Attach servos with extended pulse-width bounds
  servoLeft.attach(PIN_SERVO_LEFT, SERVO_US_MIN, SERVO_US_MAX);
  servoRight.attach(PIN_SERVO_RIGHT, SERVO_US_MIN, SERVO_US_MAX);

  // Hold center neutral during startup
  servoLeft.writeMicroseconds(SERVO_US_CENTER);
  servoRight.writeMicroseconds(SERVO_US_CENTER);

  lastMicros = micros();

  Serial.println(F("=================================================="));
  Serial.println(F("  RC BIONIC BUTTERFLY CONTROLLER v2.0 READY      "));
  Serial.print  (F("  Left Pin: ")); Serial.print(PIN_SERVO_LEFT);
  Serial.print  (F(" | Right Pin: ")); Serial.println(PIN_SERVO_RIGHT);
  Serial.println(F("=================================================="));
}

// ==============================================================================
// 5. HELPER: EXPONENTIAL FILTER & INPUT READ
// ==============================================================================
int readChannelFiltered(byte ch, int defaultVal) {
  int raw = ppm.latestValidChannelValue(ch + 1, defaultVal);
  if (raw < 800 || raw > 2200) raw = defaultVal; // Glitch filter
  if (abs(raw - rcChannels[ch]) > PPM_DEADBAND) {
    rcChannels[ch] = (rcChannels[ch] * 3 + raw) / 4; // 4-tap EMA smoothing
  }
  return rcChannels[ch];
}

// Applies deadband and cubic expo curve for smooth center control
float applyDeadbandAndExpo(int inputUs, float maxDeflectionDeg, float expoFactor) {
  int deviation = inputUs - 1500;
  if (abs(deviation) < 15) return 0.0f; // 15us deadband

  float normalized = (float)deviation / 500.0f; // -1.0 to +1.0
  normalized = constrain(normalized, -1.0f, 1.0f);

  // Expo formula: output = (1 - k)*x + k*x^3
  float shaped = (1.0f - expoFactor) * normalized + expoFactor * (normalized * normalized * normalized);
  return shaped * maxDeflectionDeg;
}

// ==============================================================================
// 6. MAIN LOOP
// ==============================================================================
void loop() {
  unsigned long nowMicros = micros();
  float dt = (nowMicros - lastMicros) * 1e-6f;
  if (dt <= 0.0f || dt > 0.1f) dt = 0.004f; // Clamp anomalous loop delays
  lastMicros = nowMicros;

  // Check receiver connection status (Channel 3 = Throttle)
  unsigned long ch3Raw = ppm.latestValidChannelValue(3, 0);
  bool signalOk = (ch3Raw >= 900 && ch3Raw <= 2100);

  if (!signalOk) {
    // Receiver signal lost -> Enter failsafe
    handleFailsafe(dt);
    digitalWrite(PIN_STATUS_LED, LOW);
  } else {
    // Read smoothed RC Channels
    int chSteering = readChannelFiltered(0, 1500); // CH1: Aileron/Rudder
    int chElevator = readChannelFiltered(1, 1500); // CH2: Elevator (pitch trim)
    int chThrottle = readChannelFiltered(2, 1000); // CH3: Throttle
    int chAuxTrim  = readChannelFiltered(4, 1500); // CH5: Steering trim pot
    int chAuxPitch = readChannelFiltered(5, 1500); // CH6: Pitch bias pot
    int chSwitch7  = readChannelFiltered(6, 1000); // CH7: Perch mode switch
    int chAux8     = readChannelFiltered(7, 1000); // CH8: LED / Flight mode switch

    // LED status & Mode switch
    if (chAux8 > 1600) {
      if (ledDebounce < 5) ledDebounce++;
    } else if (chAux8 < 1400) {
      if (ledDebounce > 0) ledDebounce--;
    }
    digitalWrite(PIN_STATUS_LED, (ledDebounce >= 5) ? HIGH : LOW);
    flightMode = (chAux8 > 1750) ? 0 : 1; // High = Pure Sine, Low = Biomimetic

    // Calculate user control offsets
    float steerStickDeg = applyDeadbandAndExpo(chSteering, MAX_STEER_DEG, 0.35f);
    float steerTrimDeg  = ((float)(chAuxTrim - 1500) / 500.0f) * MAX_STEER_DEG;
    float finalSteerDeg = steerStickDeg + steerTrimDeg;

    float elevStickDeg  = applyDeadbandAndExpo(chElevator, MAX_PITCH_DEG, 0.30f);
    float elevTrimDeg   = ((float)(chAuxPitch - 1500) / 500.0f) * MAX_PITCH_DEG;
    float finalPitchDeg = elevStickDeg + elevTrimDeg;

    // State machine: Flapping vs Perching vs Gliding
    if (chThrottle > 1100) {
      isPerching = false;
    } else if (chSwitch7 > 1600 && chThrottle < 1100) {
      isPerching = true;
    }
    if (chSwitch7 < 1400) isPerching = false;

    if (isPerching) {
      updatePerchState(chAuxPitch, dt);
    } else if (chThrottle < 1050) {
      updateGlideState(dt);
    } else {
      updateFlappingFlight(chThrottle, dt);
    }

    // Convert control offsets to microsecond pulse differences
    float steerOffsetUs = finalSteerDeg * US_PER_DEGREE;
    float pitchOffsetUs = finalPitchDeg * US_PER_DEGREE;

    // Apply Headroom & Anti-Clipping Soft Compression
    // Prevents servos from stalling against limits when flap + offsets combine
    float totalOffsetsUs = abs(steerOffsetUs) + abs(pitchOffsetUs);
    float availableHeadroom = (SERVO_US_MAX - SERVO_US_CENTER) - totalOffsetsUs - 50.0f;
    if (availableHeadroom < (MIN_FLAP_AMP_DEG * US_PER_DEGREE)) {
      availableHeadroom = MIN_FLAP_AMP_DEG * US_PER_DEGREE;
    }

    float safeFlapUs = constrain(currentFlapOffsetUs, -availableHeadroom, availableHeadroom);

    // Dynamic servo pulse calculation:
    // Left and Right servos are mirror-mounted:
    // Flapping: Left (+), Right (-) -> Wings move together up/down
    // Steering: Left (+), Right (+) -> Differential roll/bank
    // Pitch:    Left (+), Right (-) -> Both wings bias up/down together
    float pulseLeft  = SERVO_US_CENTER + safeFlapUs + steerOffsetUs + pitchOffsetUs;
    float pulseRight = SERVO_US_CENTER - safeFlapUs + steerOffsetUs - pitchOffsetUs;

    // Hard mechanical constraint
    pulseLeft  = constrain(pulseLeft,  SERVO_US_MIN, SERVO_US_MAX);
    pulseRight = constrain(pulseRight, SERVO_US_MIN, SERVO_US_MAX);

    // Direct microsecond output
    servoLeft.writeMicroseconds((int)pulseLeft);
    servoRight.writeMicroseconds((int)pulseRight);

    isFailsafe = false;
  }

  // Diagnostic telemetry
  if (millis() - lastLogMillis > LOG_INTERVAL_MS) {
    lastLogMillis = millis();
    if (signalOk) {
      Serial.print(F("[OK] THR: ")); Serial.print(rcChannels[2]);
      Serial.print(F(" | FLAP_US: ")); Serial.print((int)currentFlapOffsetUs);
      Serial.print(F(" | MODE: ")); Serial.println((flightMode == 1) ? F("BIOMIMETIC") : F("SINE"));
    } else {
      Serial.println(F("[!] FAILSAFE: NO VALID PPM SIGNAL DETECTED"));
    }
  }
}

// ==============================================================================
// 7. KINEMATICS ENGINE
// ==============================================================================

// Normal active flapping flight
void updateFlappingFlight(int throttleUs, float dt) {
  // Normalize throttle from 1050 to 2000 -> 0.0 to 1.0
  float thrNorm = (float)(throttleUs - 1050) / 950.0f;
  thrNorm = constrain(thrNorm, 0.0f, 1.0f);

  // Dynamic Frequency Scaling: 1.4 Hz to 4.2 Hz
  float currentFreq = MIN_FLAP_FREQ_HZ + thrNorm * (MAX_FLAP_FREQ_HZ - MIN_FLAP_FREQ_HZ);

  // Dynamic Amplitude Scaling: 20 degrees to 65 degrees
  float targetAmpDeg = MIN_FLAP_AMP_DEG + thrNorm * (MAX_FLAP_AMP_DEG - MIN_FLAP_AMP_DEG);
  float targetAmpUs  = targetAmpDeg * US_PER_DEGREE;

  // Advance phase smoothly with exact delta time
  flapPhase += 2.0f * PI * currentFreq * dt;
  if (flapPhase >= 2.0f * PI) {
    flapPhase -= 2.0f * PI;
  }

  // Waveform generation
  float rawWave = 0.0f;
  if (flightMode == 0) {
    // Pure Sine Wave
    rawWave = sin(flapPhase);
  } else {
    // Biomimetic Asymmetric Wave (Faster, punchier downstroke; smooth recovery)
    // Cycloidal warping produces high leading-edge vortex lift
    rawWave = sin(flapPhase + 0.35f * sin(flapPhase));
  }

  targetFlapOffsetUs = rawWave * targetAmpUs;

  // Smooth filter to eliminate sharp acceleration spikes
  currentFlapOffsetUs += (targetFlapOffsetUs - currentFlapOffsetUs) * constrain(dt * 30.0f, 0.1f, 1.0f);
}

// Gliding state: wings hold a slight upward "V" dihedral for stability
void updateGlideState(float dt) {
  float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE; // Hold stable positive dihedral
  if (abs(currentFlapOffsetUs - glideUs) > 2.0f) {
    currentFlapOffsetUs += (currentFlapOffsetUs < glideUs) ? (200.0f * dt) : (-200.0f * dt);
  } else {
    currentFlapOffsetUs = glideUs;
  }
}

// Perch mode: gentle organic wing flutter while resting on a branch or hand
void updatePerchState(int rawCh6, float dt) {
  float perchMaxDeg = map(rawCh6, 1000, 2000, 30, 55);
  perchPhase += 1.8f * dt;
  if (perchPhase >= 2.0f * PI) perchPhase -= 2.0f * PI;

  float perchOffsetDeg = perchMaxDeg + (sin(perchPhase) * 12.0f) - 12.0f;
  float goalUs = perchOffsetDeg * US_PER_DEGREE;

  currentFlapOffsetUs += (goalUs - currentFlapOffsetUs) * constrain(dt * 8.0f, 0.05f, 1.0f);
}

// Failsafe state: smoothly moves wings into positive dihedral glide to prevent crash
void handleFailsafe(float dt) {
  float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE;
  if (abs(currentFlapOffsetUs - glideUs) > 5.0f) {
    currentFlapOffsetUs += (currentFlapOffsetUs < glideUs) ? (150.0f * dt) : (-150.0f * dt);
  } else {
    currentFlapOffsetUs = glideUs;
  }

  int outL = SERVO_US_CENTER + (int)currentFlapOffsetUs;
  int outR = SERVO_US_CENTER - (int)currentFlapOffsetUs;
  servoLeft.writeMicroseconds(constrain(outL, SERVO_US_MIN, SERVO_US_MAX));
  servoRight.writeMicroseconds(constrain(outR, SERVO_US_MIN, SERVO_US_MAX));
}

