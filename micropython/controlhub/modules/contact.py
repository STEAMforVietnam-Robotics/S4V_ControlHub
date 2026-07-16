"""contact.py -- Bumper / contact switch on any MF port.

Ported from src/modules/Contact.cpp.
"""

from machine import Pin

from .module import Module
from ..config import CH_PORT_PINS, MOD_BUMPER, MSG_STATUS
from ..frame import Frame


class Contact(Module):
    def __init__(self, port, id=1, active_low=True):
        super().__init__()
        self._port = port
        self._id = id
        self._active_low = active_low
        self._pin_num = CH_PORT_PINS.get(port, (None, None))[0]
        self._pin = None
        self._begun = False
        self._last_state = False
        self._fault = 0

    def module_type(self):
        return MOD_BUMPER

    def id(self):
        return self._id

    def begin(self):
        if self._begun:
            return
        if self._pin_num is None:
            self._fault = 1
            return
        pull = Pin.PULL_UP if self._active_low else Pin.PULL_DOWN
        self._pin = Pin(self._pin_num, Pin.IN, pull)
        self._begun = True

    def get(self):
        if not self._begun:
            self.begin()
        if not self._begun:
            return False
        raw = self._pin.value()
        pressed = (raw == 0) if self._active_low else (raw == 1)
        self._last_state = pressed
        return pressed

    def make_status_frame(self):
        f = Frame(MOD_BUMPER, self._id, MSG_STATUS, 0, 8)
        f.data[0] = 1 if self._last_state else 0
        f.data[1] = 0
        f.data[7] = self._fault
        return f

    def handle_frame(self, f, hub):
        if f.module_type != MOD_BUMPER or f.module_id != self._id:
            return
        if f.rtr:
            self.get()
            hub.send_frame(self.make_status_frame())
