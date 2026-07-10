/*  ble.js -- Web Bluetooth transport for ControlHub (Nordic UART Service).
 *
 *  Usage:
 *    const ble = new ControlHubBLE();
 *    ble.onFrame = (f) => ...;      // decoded RX frame (CHProto.FrameParser output)
 *    ble.onRawTx = (bytes) => ...;  // bytes we sent
 *    ble.onState = (connected, name) => ...;
 *    ble.onLog   = (level, msg) => ...;
 *    await ble.connect();
 *    ble.sendFrame(CHProto.frameMotor(1, 50));
 */

'use strict';

const NUS_SERVICE = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const NUS_RX = '6e400002-b5a3-f393-e0a9-e50e24dcca9e'; // host -> hub (write)
const NUS_TX = '6e400003-b5a3-f393-e0a9-e50e24dcca9e'; // hub -> host (notify)

class ControlHubBLE {
  constructor() {
    this.device = null;
    this.server = null;
    this.rxChar = null;
    this.txChar = null;
    this._parser = new CHProto.FrameParser();

    this.onFrame = null;
    this.onRawTx = null;
    this.onState = null;
    this.onLog = (level, msg) => console.log(`[${level}]`, msg);

    this._onDisconnected = this._onDisconnected.bind(this);
    this._onNotify = this._onNotify.bind(this);
  }

  get connected() {
    return !!(this.server && this.server.connected);
  }

  static get available() {
    return typeof navigator !== 'undefined' && !!navigator.bluetooth;
  }

  async connect() {
    if (!ControlHubBLE.available) {
      throw new Error('Web Bluetooth không khả dụng. Dùng Chrome/Edge trên HTTPS hoặc localhost.');
    }
    this.onLog('info', 'Đang yêu cầu thiết bị...');
    this.device = await navigator.bluetooth.requestDevice({
      filters: [{ services: [NUS_SERVICE] }],
      optionalServices: [NUS_SERVICE],
    });
    this.device.addEventListener('gattserverdisconnected', this._onDisconnected);

    this.onLog('info', `Kết nối GATT: ${this.device.name || '(no name)'}`);
    this.server = await this.device.gatt.connect();

    const service = await this.server.getPrimaryService(NUS_SERVICE);
    this.rxChar = await service.getCharacteristic(NUS_RX);
    this.txChar = await service.getCharacteristic(NUS_TX);

    this._parser.reset();
    await this.txChar.startNotifications();
    this.txChar.addEventListener('characteristicvaluechanged', this._onNotify);

    this.onLog('info', 'Đã kết nối & subscribe TX notifications.');
    if (this.onState) this.onState(true, this.device.name || 'ControlHub');
  }

  async disconnect() {
    try {
      if (this.txChar) {
        this.txChar.removeEventListener('characteristicvaluechanged', this._onNotify);
      }
      if (this.device && this.device.gatt && this.device.gatt.connected) {
        this.device.gatt.disconnect();
      }
    } catch (e) {
      this.onLog('warn', `disconnect: ${e.message}`);
    }
  }

  _onDisconnected() {
    this.onLog('warn', 'Thiết bị đã ngắt kết nối.');
    this.server = null;
    this.rxChar = null;
    this.txChar = null;
    if (this.onState) this.onState(false, null);
  }

  _onNotify(event) {
    const dv = event.target.value; // DataView
    const bytes = new Uint8Array(dv.buffer, dv.byteOffset, dv.byteLength);
    const frames = this._parser.feedBytes(bytes);
    for (const f of frames) {
      if (this.onFrame) this.onFrame(f);
    }
  }

  // Send an already-serialized frame (Uint8Array from CHProto.buildFrame).
  async sendFrame(bytes) {
    if (!this.connected || !this.rxChar) {
      throw new Error('Chưa kết nối.');
    }
    // Frames are <= 15 bytes so a single write is always fine.
    if (this.rxChar.writeValueWithoutResponse) {
      await this.rxChar.writeValueWithoutResponse(bytes);
    } else {
      await this.rxChar.writeValue(bytes);
    }
    if (this.onRawTx) this.onRawTx(bytes);
  }
}

if (typeof window !== 'undefined') {
  window.ControlHubBLE = ControlHubBLE;
}
