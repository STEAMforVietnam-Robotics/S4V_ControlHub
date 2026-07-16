"""smart_bumper.py -- Smart Bumper / contact sensor on the RS485 bus.

Ported from src/modules/SmartBumper.cpp. get() does a blocking RS485 round-trip
and returns True if pressed (False on timeout / fault).
"""

from .module import Module, get_hub
from ..config import MOD_BUMPER, MSG_STATUS, RS485_1, RS485_2
from ..frame import Frame


class SmartBumper(Module):
    def __init__(self, id=0, bus=RS485_1):
        super().__init__()
        self._id = id
        self._bus = bus
        self._last_state = False
        self._fault = 0

    def module_type(self):
        return MOD_BUMPER

    def id(self):
        return self._id

    def begin(self):
        pass                                # RS485 bus started by hub.begin()

    def get(self, timeout_ms=50):
        hub = get_hub()
        if self._bus == RS485_2:
            reply = hub.comm.request_rs485_2(MOD_BUMPER, self._id, timeout_ms)
        else:
            reply = hub.comm.request_rs485(MOD_BUMPER, self._id, timeout_ms)
        if reply is None:
            self._fault = 1                 # timeout - no reply
            self._last_state = False
            return False
        self._last_state = reply.data[0] != 0
        self._fault = reply.data[7]
        return self._last_state

    def status(self):
        return self._fault

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
            self.get()                      # round-trip to the Smart Module
            hub.send_frame(self.make_status_frame())
