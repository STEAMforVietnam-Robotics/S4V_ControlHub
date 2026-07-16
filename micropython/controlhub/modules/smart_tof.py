"""smart_tof.py -- Smart ToF distance sensor on the RS485 Smart Module bus.

Ported from src/modules/SmartToF.cpp. Not wired directly to the hub: get() does
a blocking request/reply round-trip over RS485 and returns distance in mm
(0 on timeout / fault). Same wire layout as the on-board Ultrasonic.
"""

from .module import Module, get_hub
from ..config import MOD_TOF, MSG_STATUS, RS485_1, RS485_2
from ..frame import Frame


class SmartToF(Module):
    def __init__(self, id=0, bus=RS485_1):
        super().__init__()
        self._id = id
        self._bus = bus
        self._last_mm = 0
        self._fault = 0

    def module_type(self):
        return MOD_TOF

    def id(self):
        return self._id

    def begin(self):
        pass                                # RS485 bus started by hub.begin()

    def get(self, timeout_ms=50):
        hub = get_hub()
        if self._bus == RS485_2:
            reply = hub.comm.request_rs485_2(MOD_TOF, self._id, timeout_ms)
        else:
            reply = hub.comm.request_rs485(MOD_TOF, self._id, timeout_ms)
        if reply is None:
            self._fault = 1                 # timeout - no reply
            self._last_mm = 0
            return 0
        self._last_mm = (reply.data[0] << 8) | reply.data[1]
        self._fault = reply.data[7]
        return self._last_mm

    def status(self):
        return self._fault

    def make_status_frame(self):
        f = Frame(MOD_TOF, self._id, MSG_STATUS, 0, 8)
        f.data[0] = (self._last_mm >> 8) & 0xFF
        f.data[1] = self._last_mm & 0xFF
        f.data[7] = self._fault
        return f

    def handle_frame(self, f, hub):
        if f.module_type != MOD_TOF or f.module_id != self._id:
            return
        if f.rtr:
            self.get()                      # round-trip to the Smart Module
            hub.send_frame(self.make_status_frame())
