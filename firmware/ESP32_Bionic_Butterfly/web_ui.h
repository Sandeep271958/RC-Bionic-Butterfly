#pragma once
#include <Arduino.h>

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Bionic Butterfly Cockpit</title>
  <style>
    * { box-sizing: border-box; touch-action: none; -webkit-touch-callout: none; user-select: none; }
    body {
      margin: 0; padding: 0;
      background: #090d16;
      color: #f1f5f9;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      display: flex; flex-direction: column; align-items: center; justify-content: space-between;
      height: 100vh; overflow: hidden;
    }
    header {
      width: 100%; max-width: 520px; padding: 14px 18px;
      background: #111827; border-bottom: 1px solid #1f2937;
      display: flex; justify-content: space-between; align-items: center;
    }
    .brand { display: flex; align-items: center; gap: 8px; font-weight: 800; font-size: 1.15rem; color: #38bdf8; letter-spacing: 0.5px; }
    .status-group { display: flex; align-items: center; gap: 8px; }
    .badge {
      padding: 4px 10px; border-radius: 999px; font-size: 0.75rem; font-weight: 700;
      text-transform: uppercase; letter-spacing: 0.5px;
    }
    .badge-web { background: #0284c7; color: #fff; }
    .badge-ppm { background: #8b5cf6; color: #fff; animation: pulse 1.5s infinite; }
    .badge-vbat { background: #1e293b; color: #38bdf8; border: 1px solid #334155; }
    .badge-warn { background: #ef4444; color: #fff; }

    @keyframes pulse { 0% { opacity: 0.7; } 50% { opacity: 1; } 100% { opacity: 0.7; } }

    .cockpit {
      flex: 1; width: 100%; max-width: 520px;
      display: flex; justify-content: space-around; align-items: center;
      padding: 10px 16px;
    }

    .slider-box {
      display: flex; flex-direction: column; align-items: center;
      height: 280px; justify-content: space-between;
    }
    .slider-title { font-size: 0.8rem; font-weight: 700; color: #94a3b8; text-transform: uppercase; }
    .slider-val { font-size: 1.1rem; font-weight: 800; color: #38bdf8; }

    /* Vertical throttle slider */
    .v-slider {
      -webkit-appearance: slider-vertical;
      width: 48px; height: 210px;
      background: #1e293b; border-radius: 12px; outline: none;
      cursor: pointer;
    }

    /* Joystick */
    .joystick-zone {
      width: 230px; height: 230px; border-radius: 50%;
      background: radial-gradient(circle, #1e293b 0%, #0f172a 100%);
      border: 3px solid #38bdf8; position: relative;
      box-shadow: 0 0 35px rgba(56, 189, 248, 0.25);
    }
    .joystick-crosshair-h {
      position: absolute; width: 100%; height: 1px;
      background: rgba(56, 189, 248, 0.2); top: 50%; left: 0;
    }
    .joystick-crosshair-v {
      position: absolute; height: 100%; width: 1px;
      background: rgba(56, 189, 248, 0.2); left: 50%; top: 0;
    }
    .nipple {
      width: 74px; height: 74px; border-radius: 50%;
      background: radial-gradient(circle at 35% 35%, #7dd3fc 0%, #0284c7 100%);
      position: absolute; top: 78px; left: 78px;
      box-shadow: 0 6px 20px rgba(0, 0, 0, 0.6);
      pointer-events: none;
    }

    .telemetry-bar {
      width: 100%; max-width: 520px; padding: 6px 18px;
      background: #0f172a; border-top: 1px solid #1e293b;
      display: flex; justify-content: space-between; font-size: 0.75rem; color: #64748b;
    }

    footer {
      width: 100%; max-width: 520px; padding: 12px 18px 18px;
      display: flex; gap: 10px; background: #111827; border-top: 1px solid #1f2937;
    }
    button {
      flex: 1; padding: 14px 10px; border: none; border-radius: 12px;
      font-weight: 800; font-size: 0.9rem; cursor: pointer;
      display: flex; align-items: center; justify-content: center; gap: 6px;
      transition: all 0.15s ease;
    }
    .btn-led { background: #4f46e5; color: #fff; }
    .btn-perch { background: #059669; color: #fff; }
    .btn-arm { background: #ef4444; color: #fff; }
    .btn-arm.armed { background: #22c55e; color: #000; }
  </style>
</head>
<body>
  <header>
    <div class="brand">🦋 BIONIC BUTTERFLY</div>
    <div class="status-group">
      <div class="badge badge-vbat" id="vbatBadge">--V</div>
      <div class="badge badge-web" id="sourceBadge">WEB APP</div>
    </div>
  </header>

  <div class="cockpit">
    <div class="slider-box">
      <div class="slider-title">Throttle</div>
      <div class="slider-val" id="thrVal">0%</div>
      <input type="range" class="v-slider" id="thrSlider" min="0" max="100" value="0">
    </div>

    <div class="joystick-zone" id="joyZone">
      <div class="joystick-crosshair-h"></div>
      <div class="joystick-crosshair-v"></div>
      <div class="nipple" id="nipple"></div>
    </div>
  </div>

  <div class="telemetry-bar">
    <div>FREQ: <span id="freqVal">0.0</span> Hz</div>
    <div>STEER: <span id="steerVal">0.00</span></div>
    <div>PITCH: <span id="pitchVal">0.00</span></div>
  </div>

  <footer>
    <button class="btn-led" id="ledBtn">💡 LED: BREATH</button>
    <button class="btn-perch" id="perchBtn">🪶 PERCH</button>
    <button class="btn-arm armed" id="armBtn">DISARM</button>
  </footer>

  <script>
    let thr = 0, steer = 0, pitch = 0, armed = true, perch = false, ledMode = 1;
    const nipple = document.getElementById('nipple');
    const joyZone = document.getElementById('joyZone');
    const thrSlider = document.getElementById('thrSlider');
    const thrVal = document.getElementById('thrVal');
    const vbatBadge = document.getElementById('vbatBadge');
    const sourceBadge = document.getElementById('sourceBadge');
    const ledBtn = document.getElementById('ledBtn');
    const perchBtn = document.getElementById('perchBtn');
    const armBtn = document.getElementById('armBtn');
    const freqVal = document.getElementById('freqVal');
    const steerVal = document.getElementById('steerVal');
    const pitchVal = document.getElementById('pitchVal');

    thrSlider.addEventListener('input', e => {
      thr = e.target.value / 100;
      thrVal.textContent = e.target.value + '%';
      sendPacket();
    });

    let touching = false;
    const maxR = 76;

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
      steerVal.textContent = steer;
      pitchVal.textContent = pitch;
      sendPacket();
    }

    joyZone.addEventListener('touchstart', e => { touching = true; handleJoy(e.touches[0].clientX, e.touches[0].clientY); });
    window.addEventListener('touchmove', e => { if (touching) handleJoy(e.touches[0].clientX, e.touches[0].clientY); });
    window.addEventListener('touchend', () => {
      touching = false;
      nipple.style.transform = 'translate(0px, 0px)';
      steer = 0; pitch = 0;
      steerVal.textContent = '0.00';
      pitchVal.textContent = '0.00';
      sendPacket();
    });

    ledBtn.addEventListener('click', () => {
      ledMode = (ledMode + 1) % 3;
      const names = ['💡 LED: OFF', '💡 LED: BREATH', '💡 LED: STROBE'];
      ledBtn.textContent = names[ledMode];
      sendPacket();
    });

    perchBtn.addEventListener('click', () => {
      perch = !perch;
      perchBtn.style.opacity = perch ? '1.0' : '0.6';
      sendPacket();
    });

    armBtn.addEventListener('click', () => {
      armed = !armed;
      armBtn.textContent = armed ? 'DISARM' : 'ARM';
      armBtn.className = 'btn-arm ' + (armed ? 'armed' : '');
      sendPacket();
    });

    function sendPacket() {
      fetch(`/cmd?t=${thr}&s=${steer}&p=${pitch}&l=${ledMode}&a=${armed ? 1 : 0}&pr=${perch ? 1 : 0}`)
        .then(r => r.json())
        .then(d => {
          vbatBadge.textContent = d.vbat.toFixed(1) + 'V';
          if (d.vbat < 7.4) vbatBadge.className = 'badge badge-warn';
          else vbatBadge.className = 'badge badge-vbat';

          if (d.src === 'PPM') {
            sourceBadge.textContent = 'PPM RC ACTIVE';
            sourceBadge.className = 'badge badge-ppm';
          } else {
            sourceBadge.textContent = 'WEB APP';
            sourceBadge.className = 'badge badge-web';
          }
          freqVal.textContent = d.freq.toFixed(1);
        })
        .catch(() => {});
    }

    setInterval(sendPacket, 120);
  </script>
</body>
</html>
)rawliteral";
