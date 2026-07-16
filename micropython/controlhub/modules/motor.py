"""motor.py -- DC motor (DRV8874 + PCA9685). Ported from src/modules/Motor.cpp."""

from .module import Module, get_pca
from ..hal.drv8874 import DRV8874
from ..config import (
    CH_MOTOR_CHANNELS, MOD_MOTOR, MSG_CONTROL, MSG_CONFIG,
    MSG_STATUS, MODE_DUTY, STOP_COAST,
)
from ..frame import Frame


def _s16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


class Motor(Module):
    def __init__(self, port, reverse=False):
        super().__init__()
        self._reverse = reverse
        self._ch1, self._ch2 = CH_MOTOR_CHANNELS.get(port, CH_MOTOR_CHANNELS[1])
        self._id = port                     # wire ID == port number (1..4)
        self._drv = None
        self._begun = False
        self._last_duty = 0                 # duty x100 (signed)
        self._mode = MODE_DUTY
        self._fault = 0

    def module_type(self):
        return MOD_MOTOR

    def id(self):
        return self._id

    def begin(self):
        if self._begun:
            return
        self._drv = DRV8874(get_pca(), self._ch1, self._ch2)
        self._begun = True

    # ---- Level 2 API --------------------------------------------------
    def rotate(self, speed_percent):
        if speed_percent > 100:
            speed_percent = 100
        elif speed_percent < -100:
            speed_percent = -100
        if self._reverse:
            speed_percent = -speed_percent
        self._last_duty = int(speed_percent * 100)
        self._mode = MODE_DUTY
        if self._drv:
            self._drv.drive(float(speed_percent))

    def stop(self, mode=STOP_COAST):
        self._last_duty = 0
        if self._drv:
            self._drv.stop(mode)

    def _write_duty(self, duty_x100):
        self._last_duty = duty_x100
        pct = duty_x100 / 100.0
        if self._reverse:
            pct = -pct
        if self._drv:
            self._drv.drive(pct)

    # ---- Frame protocol -----------------------------------------------
    def make_status_frame(self, fault_code=0):
        d = self._last_duty & 0xFFFF
        f = Frame(MOD_MOTOR, self._id, MSG_STATUS, 0, 8)
        f.data[0] = (d >> 8) & 0xFF
        f.data[1] = d & 0xFF
        f.data[2] = self._mode & 0xFF
        f.data[7] = fault_code if fault_code else self._fault
        return f

    def handle_frame(self, f, hub):
        if f.module_type != MOD_MOTOR or f.module_id != self._id:
            return
        if f.rtr:
            hub.send_frame(self.make_status_frame())
            return
        if f.msg_id == MSG_CONTROL:
            duty = _s16((f.data[0] << 8) | f.data[1])
            self._mode = f.data[2] if f.dlc >= 3 else MODE_DUTY
            self._write_duty(duty)
        elif f.msg_id == MSG_CONFIG:
            if f.dlc >= 1:
                self._id = f.data[0]
