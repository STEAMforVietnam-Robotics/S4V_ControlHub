"""gamepad.py -- BLE gamepad receiver. Ported from src/modules/Gamepad.cpp.

Decodes MOD_GAMEPAD frames (buttons + joysticks) pushed by a phone app over the
on-board NUS. Does not touch the BLE stack itself.

Wire format (MOD_GAMEPAD, MSG_CONTROL, dlc = 6):
    data[0] = buttons hi (bits 8..13)
    data[1] = buttons lo (bits 0..7)
    data[2..5] = leftX, leftY, rightX, rightY  (128 = centre)
"""

import time

from .module import Module
from ..config import (
    MOD_GAMEPAD, MSG_CONTROL, MSG_STATUS, CH_GAMEPAD_TIMEOUT_MS,
    GP_A, GP_B, GP_X, GP_Y, GP_UP, GP_DOWN, GP_LEFT, GP_RIGHT,
    GP_L1, GP_R1, GP_R2, GP_L2, GP_SELECT, GP_START,
)
from ..frame import Frame


def _axis(raw):
    v = (raw - 128) * 100 // 127
    if v > 100:
        v = 100
    elif v < -100:
        v = -100
    if -8 < v < 8:                 # ~8% deadzone
        v = 0
    return v


class Gamepad(Module):
    def __init__(self, id=1):
        super().__init__()
        self._id = id
        self._buttons = 0
        self._prev_buttons = 0
        self._lx = self._ly = self._rx = self._ry = 128
        self._last_ms = 0
        self._ever_rx = False
        self._cb = None

    def module_type(self):
        return MOD_GAMEPAD

    def id(self):
        return self._id

    def begin(self):
        pass

    # ---- Buttons ------------------------------------------------------
    def pressed(self, btn):
        return bool(self._buttons & (1 << btn))

    def just_pressed(self, btn):
        return bool(self._buttons & (1 << btn)) and not (self._prev_buttons & (1 << btn))

    def just_released(self, btn):
        return not (self._buttons & (1 << btn)) and bool(self._prev_buttons & (1 << btn))

    def a(self): return self.pressed(GP_A)
    def b(self): return self.pressed(GP_B)
    def x(self): return self.pressed(GP_X)
    def y(self): return self.pressed(GP_Y)
    def l1(self): return self.pressed(GP_L1)
    def r1(self): return self.pressed(GP_R1)
    def l2(self): return self.pressed(GP_L2)
    def r2(self): return self.pressed(GP_R2)
    def start(self): return self.pressed(GP_START)
    def select(self): return self.pressed(GP_SELECT)
    def up(self): return self.pressed(GP_UP)
    def down(self): return self.pressed(GP_DOWN)
    def left(self): return self.pressed(GP_LEFT)
    def right(self): return self.pressed(GP_RIGHT)

    # ---- Joysticks (-100..100) ----------------------------------------
    def left_x(self): return _axis(self._lx)
    def left_y(self): return _axis(self._ly)
    def right_x(self): return _axis(self._rx)
    def right_y(self): return _axis(self._ry)

    def buttons(self):
        return self._buttons

    def connected(self):
        return self._ever_rx and time.ticks_diff(time.ticks_ms(), self._last_ms) < CH_GAMEPAD_TIMEOUT_MS

    def on_update(self, cb):
        self._cb = cb

    def make_status_frame(self):
        f = Frame(MOD_GAMEPAD, self._id, MSG_STATUS, 0, 6)
        f.data[0] = (self._buttons >> 8) & 0xFF
        f.data[1] = self._buttons & 0xFF
        f.data[2] = self._lx
        f.data[3] = self._ly
        f.data[4] = self._rx
        f.data[5] = self._ry
        return f

    def handle_frame(self, f, hub):
        if f.module_type != MOD_GAMEPAD or f.module_id != self._id:
            return
        if f.rtr:
            hub.send_frame(self.make_status_frame())
            return
        if f.msg_id == MSG_CONTROL and f.dlc >= 6:
            self._prev_buttons = self._buttons
            self._buttons = (f.data[0] << 8) | f.data[1]
            self._lx, self._ly, self._rx, self._ry = f.data[2], f.data[3], f.data[4], f.data[5]
            self._last_ms = time.ticks_ms()
            self._ever_rx = True
            if self._cb:
                self._cb(self)
