"""servo.py -- Hobby servo on a dedicated GPIO via machine.PWM (50 Hz).

Ported from src/modules/Servo.cpp (LEDC -> machine.PWM). Pulse width is set in
microseconds; angle mapping uses servo_type (travel in degrees, default 180).
"""

from machine import Pin, PWM

from .module import Module
from ..config import (
    CH_SERVO_PINS, CH_SERVO_FREQ, CH_SERVO_MIN_US, CH_SERVO_MAX_US,
    MOD_SERVO, MSG_CONTROL, MSG_STATUS,
)
from ..frame import Frame


class Servo(Module):
    def __init__(self, port, servo_type=180):
        super().__init__()
        self._port = port
        self._servo_type = servo_type
        self._pin = CH_SERVO_PINS.get(port, CH_SERVO_PINS[1])
        self._id = port
        self._pwm = None
        self._begun = False
        self._min_angle = 0
        self._max_angle = 180
        self._last_angle = 90
        self._last_us = 1500

    def module_type(self):
        return MOD_SERVO

    def id(self):
        return self._id

    def begin(self):
        if self._begun:
            return
        self._pwm = PWM(Pin(self._pin), freq=CH_SERVO_FREQ)
        self._begun = True
        self.write_microseconds(1500)       # center

    def write_microseconds(self, us):
        if not self._begun:
            self.begin()
        if us < CH_SERVO_MIN_US:
            us = CH_SERVO_MIN_US
        elif us > CH_SERVO_MAX_US:
            us = CH_SERVO_MAX_US
        self._last_us = us
        self._pwm.duty_ns(int(us) * 1000)

    def write(self, angle):
        if angle > self._max_angle:
            angle = self._max_angle
        elif angle < self._min_angle:
            angle = self._min_angle
        self._last_angle = angle
        us = CH_SERVO_MIN_US + (angle * (CH_SERVO_MAX_US - CH_SERVO_MIN_US)) // self._servo_type
        self.write_microseconds(us)

    # ---- Frame protocol -----------------------------------------------
    def handle_frame(self, f, hub):
        if f.module_type != MOD_SERVO or f.module_id != self._id:
            return
        if f.rtr:
            s = Frame(MOD_SERVO, self._id, MSG_STATUS, 0, 3)
            s.data[0] = self._last_angle & 0xFF
            s.data[1] = (self._last_us >> 8) & 0xFF
            s.data[2] = self._last_us & 0xFF
            hub.send_frame(s)
            return
        if f.msg_id == MSG_CONTROL and f.dlc >= 1:
            self.write(f.data[0])
