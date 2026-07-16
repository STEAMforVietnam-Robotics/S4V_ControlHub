"""phone_sensor.py -- Accel/Gyro/Mag/GPS pushed from a phone over BLE.

Ported from src/modules/PhoneSensor.cpp. Push-only (no RTR snapshot).
"""

import time

from .module import Module
from ..config import (
    MOD_PHONE_SENSOR, CH_PHONE_SENSOR_TIMEOUT_MS,
    PHONE_MSG_ACCEL, PHONE_MSG_GYRO, PHONE_MSG_MAG,
    PHONE_MSG_GPS, PHONE_MSG_GPS_ALT,
)


def _s16(hi, lo):
    v = ((hi << 8) | lo) & 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


def _s32(b0, b1, b2, b3):
    v = ((b0 << 24) | (b1 << 16) | (b2 << 8) | b3) & 0xFFFFFFFF
    return v - 0x100000000 if v & 0x80000000 else v


class PhoneSensor(Module):
    def __init__(self, id=1):
        super().__init__()
        self._id = id
        self.ax = self.ay = self.az = 0.0
        self.gx = self.gy = self.gz = 0.0
        self.mx = self.my = self.mz = 0.0
        self.lat = self.lon = 0.0
        self.alt = 0.0
        self._last_ms = 0
        self._ever_rx = False
        self._cb = None

    def module_type(self):
        return MOD_PHONE_SENSOR

    def id(self):
        return self._id

    def begin(self):
        pass

    # accessors (methods to mirror the C++ API)
    def accel_x(self): return self.ax
    def accel_y(self): return self.ay
    def accel_z(self): return self.az
    def gyro_x(self): return self.gx
    def gyro_y(self): return self.gy
    def gyro_z(self): return self.gz
    def mag_x(self): return self.mx
    def mag_y(self): return self.my
    def mag_z(self): return self.mz
    def latitude(self): return self.lat
    def longitude(self): return self.lon
    def altitude(self): return self.alt

    def connected(self):
        return self._ever_rx and time.ticks_diff(time.ticks_ms(), self._last_ms) < CH_PHONE_SENSOR_TIMEOUT_MS

    def on_update(self, cb):
        self._cb = cb

    def handle_frame(self, f, hub):
        if f.module_type != MOD_PHONE_SENSOR or f.module_id != self._id:
            return
        if f.rtr:
            return
        d = f.data
        if f.msg_id == PHONE_MSG_ACCEL:
            if f.dlc < 6:
                return
            self.ax = _s16(d[0], d[1]) * 0.001
            self.ay = _s16(d[2], d[3]) * 0.001
            self.az = _s16(d[4], d[5]) * 0.001
        elif f.msg_id == PHONE_MSG_GYRO:
            if f.dlc < 6:
                return
            self.gx = _s16(d[0], d[1]) * 0.1
            self.gy = _s16(d[2], d[3]) * 0.1
            self.gz = _s16(d[4], d[5]) * 0.1
        elif f.msg_id == PHONE_MSG_MAG:
            if f.dlc < 6:
                return
            self.mx = _s16(d[0], d[1]) * 0.1
            self.my = _s16(d[2], d[3]) * 0.1
            self.mz = _s16(d[4], d[5]) * 0.1
        elif f.msg_id == PHONE_MSG_GPS:
            if f.dlc < 8:
                return
            self.lat = _s32(d[0], d[1], d[2], d[3]) * 1e-7
            self.lon = _s32(d[4], d[5], d[6], d[7]) * 1e-7
        elif f.msg_id == PHONE_MSG_GPS_ALT:
            if f.dlc < 2:
                return
            self.alt = _s16(d[0], d[1]) * 0.1
        else:
            return
        self._last_ms = time.ticks_ms()
        self._ever_rx = True
        if self._cb:
            self._cb(self)
