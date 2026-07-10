/*  protocol.js -- ControlHub wire protocol (frame + CRC8) ported from
 *  src/protocol/Frame.cpp & CRC8.cpp. Pure functions, no BLE here.
 *
 *  Frame layout:
 *    0x55 | modType | modId | msgId | rtr | dlc | data[0..8] | crc8
 *  CRC8/SMBUS (poly 0x07, init 0x00), computed over bytes [0..5+dlc]
 *  (header 0x55 included), length 6+dlc.
 */

'use strict';

const FRAME_HEADER = 0x55;
const FRAME_MAX_DATA = 8;

// ---- Module types (byte[1]) ----------------------------------------------
const MOD = {
  SYSTEM: 0x00,
  POWER: 0x01,
  IMU: 0x02,
  TOF: 0x03,
  BUMPER: 0x04,
  MOTOR: 0x05,
  SERVO: 0x06,
  ULTRASONIC: 0x07,
  SMART_MOTOR: 0x08,
  LED: 0x09,
  GAMEPAD: 0x0a,
};

const MOD_NAME = Object.fromEntries(
  Object.entries(MOD).map(([k, v]) => [v, k])
);

// ---- Msg IDs (byte[3]) ----------------------------------------------------
const MSG = {
  CONTROL: 0x01, // universal "set"
  CONFIG: 0x02,
  STATUS: 0x11, // rtr=1 request, rtr=0 reply
};

// ---- Motor modes ----------------------------------------------------------
const MOTOR_MODE = { DUTY: 0x00, SPEED: 0x01, POSITION: 0x02 };

// ---- Gamepad button bit positions (14 buttons, bits 0..13) ---------------
//  Matches enum GamepadButton in src/modules/Gamepad.h. L2/R2 are digital
//  bits (note R2=10, L2=11), there are no analog trigger bytes.
const GP_BTN = {
  A: 0, B: 1, X: 2, Y: 3,
  UP: 4, DOWN: 5, LEFT: 6, RIGHT: 7,
  L1: 8, R1: 9, R2: 10, L2: 11,
  SELECT: 12, START: 13,
};

// ---- CRC8/SMBUS -----------------------------------------------------------
function crc8(bytes) {
  let crc = 0x00;
  for (const b of bytes) {
    crc ^= b & 0xff;
    for (let i = 0; i < 8; i++) {
      crc = crc & 0x80 ? ((crc << 1) ^ 0x07) & 0xff : (crc << 1) & 0xff;
    }
  }
  return crc & 0xff;
}

// ---- Build a serialized frame --------------------------------------------
// Returns Uint8Array of length 7+dlc.
function buildFrame(modType, modId, msgId, rtr, data = []) {
  const dlc = data.length;
  if (dlc > FRAME_MAX_DATA) throw new Error(`dlc ${dlc} > ${FRAME_MAX_DATA}`);
  const head = [
    FRAME_HEADER,
    modType & 0xff,
    modId & 0xff,
    msgId & 0xff,
    rtr & 0xff,
    dlc & 0xff,
    ...data.map((d) => d & 0xff),
  ];
  return Uint8Array.from([...head, crc8(head)]);
}

// int16 big-endian helper (for duty x100 etc.)
function int16BE(v) {
  v = Math.max(-32768, Math.min(32767, v | 0)) & 0xffff;
  return [(v >> 8) & 0xff, v & 0xff];
}

function toInt16BE(hi, lo) {
  const u = ((hi & 0xff) << 8) | (lo & 0xff);
  return u >= 0x8000 ? u - 0x10000 : u;
}

function toUint16BE(hi, lo) {
  return ((hi & 0xff) << 8) | (lo & 0xff);
}

// ---- Convenience frame factories -----------------------------------------
function frameMotor(id, dutyPercent, mode = MOTOR_MODE.DUTY) {
  const dutyX100 = Math.round(dutyPercent * 100);
  return buildFrame(MOD.MOTOR, id, MSG.CONTROL, 0, [...int16BE(dutyX100), mode]);
}

function frameServo(id, angle) {
  return buildFrame(MOD.SERVO, id, MSG.CONTROL, 0, [angle & 0xff]);
}

function frameLed(id, index, r, g, b, brightness = null) {
  const data = [index & 0xff, r & 0xff, g & 0xff, b & 0xff];
  if (brightness !== null) data.push(brightness & 0xff);
  return buildFrame(MOD.LED, id, MSG.CONTROL, 0, data);
}

// Gamepad control: dlc=6, [btnHi, btnLo, LX, LY, RX, RY] (axes 128=center).
function frameGamepad(id, buttons, lx, ly, rx, ry) {
  return buildFrame(MOD.GAMEPAD, id, MSG.CONTROL, 0, [
    (buttons >> 8) & 0xff,
    buttons & 0xff,
    lx & 0xff, ly & 0xff, rx & 0xff, ry & 0xff,
  ]);
}

function frameStatusRequest(modType, modId, msgId = MSG.STATUS) {
  return buildFrame(modType, modId, msgId, 1, []);
}

// ---- Streaming parser (port of FrameParser::feed) ------------------------
const S = {
  HEADER: 0, MODTYPE: 1, MODID: 2, MSGID: 3,
  RTR: 4, DLC: 5, DATA: 6, CRC: 7,
};

class FrameParser {
  constructor() {
    this.reset();
  }

  reset() {
    this._state = S.HEADER;
    this._idx = 0;
    this._work = { moduleType: 0, moduleId: 0, msgId: 0, rtr: 0, dlc: 0, data: [] };
  }

  // Feed one byte. Returns a decoded frame object or null.
  feed(b) {
    b &= 0xff;
    const w = this._work;
    switch (this._state) {
      case S.HEADER:
        if (b === FRAME_HEADER) {
          this._work = { moduleType: 0, moduleId: 0, msgId: 0, rtr: 0, dlc: 0, data: [] };
          this._state = S.MODTYPE;
        }
        break;
      case S.MODTYPE:
        w.moduleType = b; this._state = S.MODID; break;
      case S.MODID:
        w.moduleId = b; this._state = S.MSGID; break;
      case S.MSGID:
        w.msgId = b; this._state = S.RTR; break;
      case S.RTR:
        w.rtr = b; this._state = S.DLC; break;
      case S.DLC:
        if (b > FRAME_MAX_DATA) { this.reset(); return null; }
        w.dlc = b; this._idx = 0;
        this._state = b === 0 ? S.CRC : S.DATA;
        break;
      case S.DATA:
        w.data.push(b);
        this._idx++;
        if (this._idx >= w.dlc) this._state = S.CRC;
        break;
      case S.CRC: {
        const head = [FRAME_HEADER, w.moduleType, w.moduleId, w.msgId, w.rtr, w.dlc, ...w.data];
        const expect = crc8(head);
        const frame = { ...w, crc: b, crcOk: expect === b, expectedCrc: expect };
        this.reset();
        return frame.crcOk ? frame : { ...frame }; // return even on bad CRC so UI can flag it
      }
    }
    return null;
  }

  // Feed a byte array/DataView; returns array of decoded frames.
  feedBytes(bytes) {
    const out = [];
    for (let i = 0; i < bytes.length; i++) {
      const f = this.feed(bytes[i]);
      if (f) out.push(f);
    }
    return out;
  }
}

// ---- Formatting helpers ---------------------------------------------------
function toHex(bytes) {
  return Array.from(bytes, (b) => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
}

function parseHexString(str) {
  const cleaned = str.replace(/0x/gi, ' ').replace(/[,]/g, ' ').trim();
  if (!cleaned) return [];
  return cleaned.split(/\s+/).map((t) => {
    const v = parseInt(t, 16);
    if (Number.isNaN(v) || v < 0 || v > 0xff) throw new Error(`byte không hợp lệ: "${t}"`);
    return v;
  });
}

function describeFrame(f) {
  const mod = MOD_NAME[f.moduleType] || `0x${f.moduleType.toString(16)}`;
  const data = f.data && f.data.length ? toHex(f.data) : '-';
  return `${mod} id=${f.moduleId} msg=0x${f.msgId.toString(16).padStart(2, '0')} rtr=${f.rtr} dlc=${f.dlc} data=[${data}]`;
}

// Export for both module and plain-script usage.
const CHProto = {
  FRAME_HEADER, FRAME_MAX_DATA, MOD, MOD_NAME, MSG, MOTOR_MODE, GP_BTN,
  crc8, buildFrame, int16BE, toInt16BE, toUint16BE,
  frameMotor, frameServo, frameLed, frameGamepad, frameStatusRequest,
  FrameParser, toHex, parseHexString, describeFrame,
};

if (typeof window !== 'undefined') {
  window.CHProto = CHProto;
  // expose a few top-level for console debugging (verification step)
  Object.assign(window, { buildFrame, crc8, FrameParser });
}
if (typeof module !== 'undefined' && module.exports) module.exports = CHProto;
