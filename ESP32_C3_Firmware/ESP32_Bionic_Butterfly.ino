/*
 * ==============================================================================
 * PROJECT: ESP32-C3 RC BIONIC BUTTERFLY (WEB APP & MULTI-PROTOCOL CONTROLLER)
 * BASED ON: Max Imagination ESP-FLUTTER Architecture & CTorque Mechanics
 * HARDWARE: Seeed Studio XIAO ESP32-C3 (1.2g)
 * ==============================================================================
 *
 * FEATURES:
 * 1. ZERO RECEIVER WEIGHT: Fly directly from your smartphone via Wi-Fi AP!
 *    Connect your phone to "Butterfly_RC" (password: 12345678) and open
 *    http://192.168.4.1 in Safari or Chrome for a full virtual joystick interface.
 * 2. DIFFERENTIAL-AMPLITUDE STEERING: Shifts stroke depth between wings for
 *    sharp, biomimetic turns without wing stall.
 * 3. TURN LIFT BOOST: Automatically injects +15% extra flap amplitude during
 *    hard steering to maintain altitude during tight bank turns.
 * 4. 250Hz - 333Hz DIGITAL SERVO PWM: High-speed hardware PWM eliminates servo
 *    delay and holding flutter.
 * 5. NIGHT FLIGHT LEDS: Drives ultra-thin flexible 3V LED filaments on D3.
 * 6. BATTERY MONITORING: Reads 2S LiPo voltage on D2 with auto-neutral cutoff
 *    to protect lithium batteries from over-discharge.
 * ==============================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <math.h>

// ==============================================================================
// 1. PIN DEFINITIONS (Seeed Studio XIAO ESP32-C3)
// ==============================================================================
#define PIN_SERVO_LEFT      D0 // Left wing servo signal
#define PIN_SERVO_RIGHT     D1 // Right wing servo signal
#define PIN_BATTERY_ADC     D2 // 2S Battery voltage divider (49k / 18k)
#define PIN_LED_FILAMENT    D3 // Flexible LED filament (PWM)
#define PIN_STATUS_LED      D6 // Built-in status LED

// ==============================================================================
// 2. KINEMATIC & FLIGHT CONSTANTS
// ==============================================================================
const int SERVO_US_MIN        = 600;   // Physical minimum limit (us)
const int SERVO_US_MAX        = 2400;  // Physical maximum limit (us)
const int SERVO_US_CENTER     = 1500;  // Horizontal wing neutral
const float US_PER_DEGREE      = 11.11f;

const float MAX_FLAP_AMP_DEG   = 65.0f;
const float MIN_FLAP_AMP_DEG   = 20.0f;
const float MIN_FLAP_FREQ_HZ   = 1.4f;
const float MAX_FLAP_FREQ_HZ   = 4.2f;

const float STEER_DIFF_FRAC    = 0.45f;
const float TURN_LIFT_BOOST    = 0.15f;
const float DIHEDRAL_GLIDE_DEG = 18.0f;

// Battery divider: 49k top, 18k bottom -> Ratio = (49 + 18) / 18 = 3.722
const float VBAT_DIVIDER_RATIO = 3.722f;
const float VBAT_CRITICAL      = 7.0f;  // Auto-cutoff voltage (2S LiPo)

// ==============================================================================
// 3. OBJECTS & STATE
// ==============================================================================
WebServer server(80);
Servo servoLeft, servoRight;

// Pilot control targets (received from Web App)
float pilotThrottle = 0.0f;  // 0.0 to 1.0
float pilotSteering = 0.0f;  // -1.0 (left) to +1.0 (right)
float pilotPitch    = 0.0f;  // -1.0 (dive) to +1.0 (climb)
int   ledState      = 1;     // 0=Off, 1=Breathing, 2=Strobe
bool  isArmed       = true;

// Kinematic states
float flapPhase     = 0.0f;
float currentLeftUs = 0.0f;
float currentRightUs= 0.0f;
float smoothedSteer = 0.0f;
float ledPhase      = 0.0f;
float batteryVoltage= 8.0f;

unsigned long lastMicros = 0;
unsigned long lastPacketTime = 0;

// ==============================================================================
// 4. EMBEDDED SMARTPHONE WEB APP CONTROLLER (HTML / JS / CSS)
// ==============================================================================
const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Bionic Butterfly RC</title>
  <style>
    * { box-sizing: border-box; touch-action: none; -webkit-touch-callout: none; user-select: none; }
    body { margin: 0; background: #0f172a; color: #f8fafc; font-family: -apple-system, BlinkMacSystemFont, sans-serif; display: flex; flex-direction: column; align-items: center; height: 100vh; overflow: hidden; }
    header { width: 100%; padding: 12px 20px; background: #1e293b; display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid #334155; }
    h1 { margin: 0; font-size: 1.1rem; color: #38bdf8; font-weight: 700; }
    .badge { padding: 4px 10px; border-radius: 999px; font-size: 0.8rem; font-weight: 600; background: #22c55e; color: #000; }
    #vbat { font-size: 0.85rem; color: #94a3b8; font-weight: bold; }
    .cockpit { flex: 1; width: 100%; max-width: 480px; display: flex; justify-content: space-around; align-items: center; padding: 20px 10px; }
    .slider-box { display: flex; flex-direction: column; align-items: center; height: 260px; justify-content: space-between; }
    .v-slider { -webkit-appearance: slider-vertical; width: 44px; height: 200px; }
    .label { font-size: 0.85rem; color: #cbd5e1; font-weight: 600; text-transform: uppercase; }
    .joystick-zone { width: 220px; height: 220px; border-radius: 50%; background: #1e293b; border: 3px solid #38bdf8; position: relative; box-shadow: 0 0 25px rgba(56, 189, 248, 0.2); }
    .nipple { width: 70px; height: 70px; border-radius: 50%; background: #38bdf8; position: absolute; top: 75px; left: 75px; box-shadow: 0 4px 15px rgba(0,0,0,0.5); }
    footer { width: 100%; max-width: 480px; padding: 15px; display: flex; gap: 10px; }
    button { flex: 1; padding: 14px; border: none; border-radius: 10px; font-weight: bold; font-size: 0.95rem; cursor: pointer; }
    .btn-led { background: #6366f1; color: #fff; }
    .btn-arm { background: #ef4444; color: #fff; }
  </style>
</head>
<body>
  <header>
    <h1>🦋 BIONIC BUTTERFLY</h1>
    <div id="vbat">8.2V</div>
    <div class="badge" id="statusBadge">READY</div>
  </header>

  <div class="cockpit">
    <div class="slider-box">
      <div class="label">THR <span id="thrVal">0%</span></div>
      <input type="range" class="v-slider" id="thrSlider" min="0" max="100" value="0">
    </div>

    <div class="joystick-zone" id="joyZone">
      <div class="nipple" id="nipple"></div>
    </div>
  </div>

  <footer>
    <button class="btn-led" id="ledBtn">LED: BREATH</button>
    <button class="btn-arm" id="armBtn">DISARM</button>
  </footer>

  <script>
    let thr = 0, steer = 0, pitch = 0, armed = true, ledMode = 1;
    const nipple = document.getElementById('nipple');
    const joyZone = document.getElementById('joyZone');
    const thrSlider = document.getElementById('thrSlider');
    const thrVal = document.getElementById('thrVal');
    const vbatEl = document.getElementById('vbat');
    const ledBtn = document.getElementById('ledBtn');
    const armBtn = document.getElementById('armBtn');

    thrSlider.addEventListener('input', e => {
      thr = e.target.value / 100;
      thrVal.textContent = e.target.value + '%';
      sendPacket();
    });

    let touching = false;
    const maxR = 75;

    function handleJoy(x, y) {
      const rect = joyZone.getBoundingClientRect();
      const cx = rect.left + rect.width / 2;
      const cy = rect.top + rect.height / 2;
      let dx = x - cx;
      let dy = y - cy;
      const dist = Math.sqrt(dx * dx + dy * dy);
      if (dist > maxR) { dx = (dx / dist) * maxR; dy = (dy / dist) * maxR; }
      nipple.style.transform = `translate(${dx}px, ${dy}px)`;
      steer = +(dx / maxR).toFixed(2);
      pitch = +(-dy / maxR).toFixed(2);
      sendPacket();
    }

    joyZone.addEventListener('touchstart', e => { touching = true; handleJoy(e.touches[0].clientX, e.touches[0].clientY); });
    window.addEventListener('touchmove', e => { if (touching) handleJoy(e.touches[0].clientX, e.touches[0].clientY); });
    window.addEventListener('touchend', () => {
      touching = false;
      nipple.style.transform = 'translate(0px, 0px)';
      steer = 0; pitch = 0;
      sendPacket();
    });

    ledBtn.addEventListener('click', () => {
      ledMode = (ledMode + 1) % 3;
      const names = ['LED: OFF', 'LED: BREATH', 'LED: STROBE'];
      ledBtn.textContent = names[ledMode];
      sendPacket();
    });

    armBtn.addEventListener('click', () => {
      armed = !armed;
      armBtn.textContent = armed ? 'DISARM' : 'ARM';
      armBtn.style.background = armed ? '#ef4444' : '#22c55e';
      sendPacket();
    });

    function sendPacket() {
      fetch(`/cmd?t=${thr}&s=${steer}&p=${pitch}&l=${ledMode}&a=${armed ? 1 : 0}`)
        .then(r => r.json())
        .then(d => { vbatEl.textContent = d.vbat.toFixed(1) + 'V'; })
        .catch(() => {});
    }

    setInterval(sendPacket, 120);
  </script>
</body>
</html>
)rawliteral";

// ==============================================================================
// 5. WEB SERVER HANDLERS
// ==============================================================================
void handleRoot() {
  server.send_P(200, "text/html", PAGE_INDEX);
}

void handleCommand() {
  if (server.hasArg("t")) pilotThrottle = server.arg("t").toFloat();
  if (server.hasArg("s")) pilotSteering = server.arg("s").toFloat();
  if (server.hasArg("p")) pilotPitch    = server.arg("p").toFloat();
  if (server.hasArg("l")) ledState      = server.arg("l").toInt();
  if (server.hasArg("a")) isArmed       = (server.arg("a").toInt() == 1);

  lastPacketTime = millis();

  // Return live battery telemetry JSON
  char json[48];
  snprintf(json, sizeof(json), "{\"vbat\":%.2f}", batteryVoltage);
  server.send(200, "application/json", json);
}

// ==============================================================================
// 6. SETUP
// ==============================================================================
void setup() {
  Serial.begin(115200);

  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_LED_FILAMENT, OUTPUT);
  pinMode(PIN_BATTERY_ADC, INPUT);

  // Configure high-speed 333Hz PWM servos
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoLeft.setPeriodHertz(333);
  servoRight.setPeriodHertz(333);

  servoLeft.attach(PIN_SERVO_LEFT, SERVO_US_MIN, SERVO_US_MAX);
  servoRight.attach(PIN_SERVO_RIGHT, SERVO_US_MIN, SERVO_US_MAX);

  servoLeft.writeMicroseconds(SERVO_US_CENTER);
  servoRight.writeMicroseconds(SERVO_US_CENTER);

  // Start Wi-Fi Access Point
  WiFi.softAP("Butterfly_RC", "12345678");
  Serial.println(F("\n--- BIONIC BUTTERFLY AP READY ---"));
  Serial.print(F("Connect to 'Butterfly_RC' and open: http://"));
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.begin();

  lastMicros = micros();
  lastPacketTime = millis();
}

// ==============================================================================
// 7. MAIN LOOP
// ==============================================================================
void loop() {
  server.handleClient();

  unsigned long nowMicros = micros();
  float dt = (nowMicros - lastMicros) * 1e-6f;
  if (dt <= 0.0f || dt > 0.05f) dt = 0.003f;
  lastMicros = nowMicros;

  // Read Battery Voltage (every ~500ms)
  static unsigned long lastAdcTime = 0;
  if (millis() - lastAdcTime > 500) {
    lastAdcTime = millis();
    int raw = analogRead(PIN_BATTERY_ADC);
    float pinV = (raw / 4095.0f) * 3.3f;
    batteryVoltage = pinV * VBAT_DIVIDER_RATIO;
  }

  // Connection Failsafe (cuts wings if phone disconnects for > 1.5s)
  bool isConnected = (millis() - lastPacketTime < 1500) && isArmed;

  // LED Lighting Effects
  updateLeds(dt);

  if (!isConnected || batteryVoltage < VBAT_CRITICAL) {
    // Glide / Failsafe
    float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE;
    currentLeftUs  += (glideUs - currentLeftUs)  * constrain(dt * 8.0f, 0.05f, 1.0f);
    currentRightUs += (glideUs - currentRightUs) * constrain(dt * 8.0f, 0.05f, 1.0f);
  } else if (pilotThrottle < 0.05f) {
    // Zero-throttle Idle Lean
    float glideUs = DIHEDRAL_GLIDE_DEG * US_PER_DEGREE;
    float leanUs  = (pilotSteering * 15.0f) * US_PER_DEGREE;
    currentLeftUs  += ((glideUs - leanUs) - currentLeftUs)  * constrain(dt * 15.0f, 0.05f, 1.0f);
    currentRightUs += ((glideUs + leanUs) - currentRightUs) * constrain(dt * 15.0f, 0.05f, 1.0f);
  } else {
    // Active Differential-Amplitude Flapping
    updateFlapping(pilotThrottle, pilotSteering, dt);
  }

  // Pitch bias
  float pitchUs = (pilotPitch * 16.0f) * US_PER_DEGREE;

  // Output Servo Pulses
  float pulseL = SERVO_US_CENTER + currentLeftUs  + pitchUs;
  float pulseR = SERVO_US_CENTER - currentRightUs - pitchUs;

  pulseL = constrain(pulseL, SERVO_US_MIN, SERVO_US_MAX);
  pulseR = constrain(pulseR, SERVO_US_MIN, SERVO_US_MAX);

  servoLeft.writeMicroseconds((int)pulseL);
  servoRight.writeMicroseconds((int)pulseR);
}

// ==============================================================================
// 8. KINEMATIC WAVE ENGINE
// ==============================================================================
void updateFlapping(float thr, float steer, float dt) {
  // Frequency: 1.4 to 4.2 Hz
  float currentFreq = MIN_FLAP_FREQ_HZ + thr * (MAX_FLAP_FREQ_HZ - MIN_FLAP_FREQ_HZ);

  // Amplitude: 20 to 65 deg with Turn Lift Boost
  float baseAmpDeg = MIN_FLAP_AMP_DEG + thr * (MAX_FLAP_AMP_DEG - MIN_FLAP_AMP_DEG);
  baseAmpDeg *= (1.0f + fabsf(steer) * TURN_LIFT_BOOST);

  // Differential allocation
  float maxDelta = baseAmpDeg * STEER_DIFF_FRAC;
  float delta = steer * maxDelta;

  float leftAmpUs  = constrain(baseAmpDeg + delta, 10.0f, 75.0f) * US_PER_DEGREE;
  float rightAmpUs = constrain(baseAmpDeg - delta, 10.0f, 75.0f) * US_PER_DEGREE;

  // Phase increment
  flapPhase += 2.0f * PI * currentFreq * dt;
  if (flapPhase >= 2.0f * PI) flapPhase -= 2.0f * PI;

  // Cycloidal power stroke
  float wave = sin(flapPhase + 0.35f * sin(flapPhase));

  float targetL = wave * leftAmpUs;
  float targetR = wave * rightAmpUs;

  currentLeftUs  += (targetL - currentLeftUs)  * constrain(dt * 30.0f, 0.1f, 1.0f);
  currentRightUs += (targetR - currentRightUs) * constrain(dt * 30.0f, 0.1f, 1.0f);
}

// ==============================================================================
// 9. LED DRIVER
// ==============================================================================
void updateLeds(float dt) {
  if (ledState == 0) {
    analogWrite(PIN_LED_FILAMENT, 0);
  } else if (ledState == 1) {
    // Breathing Glow
    ledPhase += 2.5f * dt;
    if (ledPhase >= 2.0f * PI) ledPhase -= 2.0f * PI;
    int brightness = (int)((sin(ledPhase) + 1.0f) * 110.0f + 35.0f);
    analogWrite(PIN_LED_FILAMENT, brightness);
  } else {
    // Night Strobe
    ledPhase += 16.0f * dt;
    if (ledPhase >= 2.0f * PI) ledPhase -= 2.0f * PI;
    analogWrite(PIN_LED_FILAMENT, (sin(ledPhase) > 0.0f) ? 255 : 0);
  }
}
