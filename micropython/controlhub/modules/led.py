"""led.py -- WS2812 / NeoPixel strip on an MF port.

Ported from src/modules/Led.cpp using the built-in `neopixel` module. Software
brightness scaling is applied on show() (NeoPixel has no hardware brightness).
"""

from machine import Pin
import neopixel

from .module import Module
from ..config import CH_PORT_PINS, MOD_LED, MSG_CONTROL, MSG_STATUS
from ..frame import Frame


class Led(Module):
    def __init__(self, port, count=8, id=1):
        super().__init__()
        self._port = port
        self._count = count
        self._id = id
        self._pin_num = CH_PORT_PINS.get(port, (None, None))[0]
        self._np = None
        self._begun = False
        self._brightness = 64               # 0..255
        self._buf = [(0, 0, 0)] * count      # logical (unscaled) colors
        self._fault = 0

    def module_type(self):
        return MOD_LED

    def id(self):
        return self._id

    def begin(self):
        if self._begun:
            return
        if self._pin_num is None:
            self._fault = 1
            return
        self._np = neopixel.NeoPixel(Pin(self._pin_num), self._count)
        self._begun = True
        self.clear()

    def _apply(self):
        if not self._begun:
            return
        b = self._brightness
        for i, (r, g, bl) in enumerate(self._buf):
            self._np[i] = (r * b // 255, g * b // 255, bl * b // 255)

    def fill(self, r, g, b):
        if not self._begun:
            self.begin()
        self._buf = [(r, g, b)] * self._count

    def set_color(self, r, g, b):
        self.fill(r, g, b)
        self.show()

    def set_pixel(self, i, r, g, b):
        if not self._begun:
            self.begin()
        if i >= self._count:
            return
        self._buf[i] = (r, g, b)
        self.show()

    def set_brightness(self, b):
        self._brightness = b
        if not self._begun:
            self.begin()
        self.show()

    def clear(self):
        if not self._begun:
            self.begin()
        self._buf = [(0, 0, 0)] * self._count
        self.show()

    def show(self):
        if not self._begun:
            self.begin()
        self._apply()
        self._np.write()

    def make_status_frame(self):
        f = Frame(MOD_LED, self._id, MSG_STATUS, 0, 4)
        f.data[0] = (self._count >> 8) & 0xFF
        f.data[1] = self._count & 0xFF
        f.data[2] = self._brightness
        f.data[3] = self._fault
        return f

    def handle_frame(self, f, hub):
        if f.module_type != MOD_LED or f.module_id != self._id:
            return
        if f.rtr:
            hub.send_frame(self.make_status_frame())
            return
        if f.msg_id == MSG_CONTROL and f.dlc >= 4:
            index = f.data[0]
            r, g, b = f.data[1], f.data[2], f.data[3]
            if index == 0xFF:
                self.set_color(r, g, b)
            else:
                self.set_pixel(index, r, g, b)
            if f.dlc >= 5:
                self.set_brightness(f.data[4])
