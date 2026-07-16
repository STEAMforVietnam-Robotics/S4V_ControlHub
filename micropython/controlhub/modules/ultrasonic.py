"""ultrasonic.py -- HC-SR04 style sensor on an MF4P port (SIG1=trig, SIG2=echo).

Ported from src/modules/Ultrasonic.cpp; pulseIn -> machine.time_pulse_us.
"""

from machine import Pin, time_pulse_us
import time

from .module import Module
from ..config import (
    CH_PORT_PINS, PORT_MF4P_1, PORT_MF4P_2,
    MOD_ULTRASONIC, MSG_STATUS,
)
from ..frame import Frame


class Ultrasonic(Module):
    def __init__(self, port, id=1):
        super().__init__()
        self._port = port
        self._id = id
        self._trig = None
        self._echo = None
        self._begun = False
        self._last_mm = 0
        self._fault = 0

    def module_type(self):
        return MOD_ULTRASONIC

    def id(self):
        return self._id

    def begin(self):
        if self._begun:
            return
        if self._port not in (PORT_MF4P_1, PORT_MF4P_2):
            self._begun = False              # needs a 4-pin port
            return
        trig_pin, echo_pin = CH_PORT_PINS[self._port]
        self._trig = Pin(trig_pin, Pin.OUT)
        self._echo = Pin(echo_pin, Pin.IN)
        self._trig.value(0)
        self._begun = True

    def get(self):
        if not self._begun:
            return 0
        self._trig.value(0)
        time.sleep_us(2)
        self._trig.value(1)
        time.sleep_us(10)
        self._trig.value(0)

        dur = time_pulse_us(self._echo, 1, 30000)   # 30 ms timeout (~5 m)
        if dur < 0:
            self._fault = 1
            self._last_mm = 0
            return 0
        self._fault = 0
        mm = (dur * 343) // 2000                     # us * 0.343 mm/us / 2
        self._last_mm = mm
        return mm

    def status(self):
        return self._fault

    def make_status_frame(self):
        f = Frame(MOD_ULTRASONIC, self._id, MSG_STATUS, 0, 8)
        f.data[0] = (self._last_mm >> 8) & 0xFF
        f.data[1] = self._last_mm & 0xFF
        f.data[7] = self._fault
        return f

    def handle_frame(self, f, hub):
        if f.module_type != MOD_ULTRASONIC or f.module_id != self._id:
            return
        if f.rtr:
            self.get()
            hub.send_frame(self.make_status_frame())
