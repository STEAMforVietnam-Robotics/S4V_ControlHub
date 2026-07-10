/*  app.js -- wires UI <-> BLE <-> protocol, log + gamepad streaming + PWA.  */
'use strict';

const P = window.CHProto;
const ble = new ControlHubBLE();

const $ = (id) => document.getElementById(id);

// ---- Logging --------------------------------------------------------------
const logEl = $('log');
const autoscroll = $('autoscroll');

function log(cls, text) {
  const line = document.createElement('div');
  line.className = `line ${cls}`;
  const t = new Date().toLocaleTimeString('en-GB', { hour12: false });
  line.innerHTML = `<span class="time">${t}</span> ${escapeHtml(text)}`;
  logEl.appendChild(line);
  while (logEl.childElementCount > 500) logEl.removeChild(logEl.firstChild);
  if (autoscroll.checked) logEl.scrollTop = logEl.scrollHeight;
}
function escapeHtml(s) {
  return s.replace(/[&<>]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;' }[c]));
}

// ---- BLE callbacks --------------------------------------------------------
ble.onLog = (level, msg) => log(level === 'warn' ? 'err' : 'sys', msg);

ble.onState = (connected, name) => {
  $('statusDot').classList.toggle('on', connected);
  $('statusText').textContent = connected ? `Đã kết nối: ${name}` : 'Chưa kết nối';
  $('connectBtn').hidden = connected;
  $('disconnectBtn').hidden = !connected;
};

ble.onRawTx = (bytes) => {
  log('tx', `TX  ${P.toHex(bytes)}`);
};

ble.onFrame = (f) => {
  const flag = f.crcOk ? '' : `  ⚠ CRC sai (chờ 0x${f.expectedCrc.toString(16).padStart(2, '0')})`;
  log(f.crcOk ? 'rx' : 'err', `RX  ${P.describeFrame(f)}${flag}`);
};

// ---- Send helper ----------------------------------------------------------
async function send(bytes) {
  try {
    await ble.sendFrame(bytes);
  } catch (e) {
    log('err', `Lỗi gửi: ${e.message}`);
  }
}

// ---- Connect / disconnect -------------------------------------------------
$('connectBtn').addEventListener('click', async () => {
  try {
    await ble.connect();
  } catch (e) {
    if (e.name !== 'NotFoundError') log('err', `Kết nối lỗi: ${e.message}`);
  }
});
$('disconnectBtn').addEventListener('click', () => ble.disconnect());

// ---- Motor ----------------------------------------------------------------
const motorDuty = $('motorDuty');
const bindOut = (input, out, fmt = (v) => v) => {
  const o = $(out);
  const upd = () => { o.textContent = fmt(input.value); };
  input.addEventListener('input', upd); upd();
};
bindOut(motorDuty, 'motorDutyOut');

function sendMotor() {
  send(P.frameMotor(+$('motorId').value, +motorDuty.value, +$('motorMode').value));
}
$('motorSend').addEventListener('click', sendMotor);
$('motorStop').addEventListener('click', () => {
  motorDuty.value = 0; motorDuty.dispatchEvent(new Event('input'));
  send(P.frameMotor(+$('motorId').value, 0, +$('motorMode').value));
});
motorDuty.addEventListener('input', () => { if ($('motorLive').checked) sendMotor(); });

// ---- Servo ----------------------------------------------------------------
const servoAngle = $('servoAngle');
bindOut(servoAngle, 'servoAngleOut');
function sendServo() { send(P.frameServo(+$('servoId').value, +servoAngle.value)); }
$('servoSend').addEventListener('click', sendServo);
servoAngle.addEventListener('input', () => { if ($('servoLive').checked) sendServo(); });

// ---- LED ------------------------------------------------------------------
bindOut($('ledBright'), 'ledBrightOut');
function hexToRgb(h) {
  const n = parseInt(h.slice(1), 16);
  return [(n >> 16) & 0xff, (n >> 8) & 0xff, n & 0xff];
}
function sendLed(off = false) {
  const id = +$('ledId').value;
  const index = $('ledAll').checked ? 0xff : (+$('ledIndex').value & 0xff);
  const [r, g, b] = off ? [0, 0, 0] : hexToRgb($('ledColor').value);
  const bright = +$('ledBright').value;
  send(P.frameLed(id, index, r, g, b, bright));
}
$('ledSend').addEventListener('click', () => sendLed(false));
$('ledOff').addEventListener('click', () => sendLed(true));

// ---- Gamepad --------------------------------------------------------------
const gpState = { buttons: 0, lx: 128, ly: 128, rx: 128, ry: 128 };

// Send immediately on interaction only when not streaming (avoids double sends).
function gpNudge() { if (!$('gpStream').checked && ble.connected) sendGamepad(); }

// Two draggable joysticks -> axis pairs (128 = center, 0..255).
document.querySelectorAll('.joystick').forEach((pad) => {
  const axX = pad.dataset.x, axY = pad.dataset.y;
  const knob = pad.querySelector('.knob');
  const outX = $(axX + 'Out'), outY = $(axY + 'Out');

  const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v));

  const apply = (nx, ny) => {
    // nx, ny in [-1, 1]; up/left = smaller value.
    gpState[axX] = clamp(Math.round(128 + nx * 127), 0, 255);
    gpState[axY] = clamp(Math.round(128 + ny * 127), 0, 255);
    const r = pad.clientWidth / 2 - knob.offsetWidth / 2;
    knob.style.transform = `translate(calc(-50% + ${nx * r}px), calc(-50% + ${ny * r}px))`;
    outX.textContent = gpState[axX];
    outY.textContent = gpState[axY];
  };

  const moveFrom = (e) => {
    const rect = pad.getBoundingClientRect();
    let nx = (e.clientX - rect.left) / (rect.width / 2) - 1;
    let ny = (e.clientY - rect.top) / (rect.height / 2) - 1;
    const mag = Math.hypot(nx, ny);
    if (mag > 1) { nx /= mag; ny /= mag; } // clamp to unit circle
    apply(nx, ny);
    gpNudge();
  };

  const recenter = () => {
    pad.classList.remove('dragging');
    apply(0, 0);
    gpNudge();
  };

  pad.addEventListener('pointerdown', (e) => {
    e.preventDefault();
    pad.setPointerCapture(e.pointerId);
    pad.classList.add('dragging');
    moveFrom(e);
  });
  pad.addEventListener('pointermove', (e) => {
    if (pad.classList.contains('dragging')) moveFrom(e);
  });
  pad.addEventListener('pointerup', recenter);
  pad.addEventListener('pointercancel', recenter);
});

document.querySelectorAll('.btn.gp[data-btn]').forEach((btn) => {
  const bit = P.GP_BTN[btn.dataset.btn];
  const press = (on) => {
    if (on) gpState.buttons |= (1 << bit);
    else gpState.buttons &= ~(1 << bit);
    btn.classList.toggle('held', on);
  };
  btn.addEventListener('pointerdown', (e) => { e.preventDefault(); press(true); });
  btn.addEventListener('pointerup', () => press(false));
  btn.addEventListener('pointerleave', () => press(false));
  btn.addEventListener('pointercancel', () => press(false));
});

function sendGamepad() {
  const s = gpState;
  send(P.frameGamepad(1, s.buttons, s.lx, s.ly, s.rx, s.ry));
}
$('gpSendOnce').addEventListener('click', sendGamepad);

let gpTimer = null;
$('gpStream').addEventListener('change', (e) => {
  if (e.target.checked) {
    gpTimer = setInterval(() => { if (ble.connected) sendGamepad(); }, 150);
    log('sys', 'Gamepad streaming: ON (150ms)');
  } else {
    clearInterval(gpTimer); gpTimer = null;
    log('sys', 'Gamepad streaming: OFF');
  }
});

// ---- Raw hex console ------------------------------------------------------
$('rawSend').addEventListener('click', () => {
  try {
    const bytes = P.parseHexString($('rawHex').value);
    if (bytes.length) send(Uint8Array.from(bytes));
  } catch (e) { log('err', e.message); }
});
$('rawAppendCrc').addEventListener('click', () => {
  try {
    const bytes = P.parseHexString($('rawHex').value);
    if (!bytes.length) return;
    const withCrc = [...bytes, P.crc8(bytes)];
    send(Uint8Array.from(withCrc));
  } catch (e) { log('err', e.message); }
});
$('clearLog').addEventListener('click', () => { logEl.innerHTML = ''; });

// ---- PWA: install + service worker ---------------------------------------
let deferredPrompt = null;
window.addEventListener('beforeinstallprompt', (e) => {
  e.preventDefault();
  deferredPrompt = e;
  $('installBtn').hidden = false;
});
$('installBtn').addEventListener('click', async () => {
  if (!deferredPrompt) return;
  deferredPrompt.prompt();
  await deferredPrompt.userChoice;
  deferredPrompt = null;
  $('installBtn').hidden = true;
});

if ('serviceWorker' in navigator) {
  window.addEventListener('load', () => {
    navigator.serviceWorker.register('sw.js').catch((e) => log('err', `SW: ${e.message}`));
  });
}

// ---- Startup checks + self-test ------------------------------------------
if (!ControlHubBLE.available) {
  log('err', 'Web Bluetooth KHÔNG khả dụng. Cần Chrome/Edge trên HTTPS hoặc localhost.');
} else {
  log('sys', 'Sẵn sàng. Bấm "Kết nối" và chọn thiết bị "ControlHub".');
}

// Self-test protocol against a known vector from USB_BLE_Bridge example:
// motor id=0 50% forward -> 55 05 00 01 00 03 13 88 00 <CRC>
(function selfTest() {
  const f = P.frameMotor(0, 50, 0); // duty x100 = 5000 = 0x1388
  const hex = P.toHex(f);
  const okPrefix = hex.startsWith('55 05 00 01 00 03 13 88 00');
  const round = new P.FrameParser().feedBytes(f);
  const rtOk = round.length === 1 && round[0].crcOk && round[0].moduleType === 0x05;
  console.log(`[selftest] frameMotor(0,50)=${hex}  prefixOk=${okPrefix}  roundtripOk=${rtOk}`);
  if (!okPrefix || !rtOk) log('err', 'Self-test protocol THẤT BẠI - xem console.');
})();
