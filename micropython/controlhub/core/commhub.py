"""commhub.py -- Owns the three transports, runs a FrameParser on each stream,
and dispatches valid frames to per-module handlers or a global callback.

Ported from src/core/CommHub.{h,cpp}.
"""

import time

from ..frame import Frame, FrameParser
from ..config import MSG_STATUS
from ..transport.usb import UsbTransport
from ..transport.ble import BleTransport
from ..transport.rs485 import Rs485Transport
from ..config import (
    CH_RS485_UART, CH_RS485_TX, CH_RS485_RX, CH_RS485_DE_RE, CH_RS485_BAUD,
    CH_RS485_2_UART, CH_RS485_2_TX, CH_RS485_2_RX, CH_RS485_2_DE_RE, CH_RS485_2_BAUD,
    CH_USB_BAUD, CH_BLE_DEFAULT_NAME,
)

_MAX_MODULE_HANDLERS = 16


class CommHub:
    def __init__(self):
        self.usb = UsbTransport()
        self.ble = BleTransport()
        self.rs485 = Rs485Transport(CH_RS485_UART, CH_RS485_TX, CH_RS485_RX,
                                    CH_RS485_DE_RE, CH_RS485_BAUD)
        self.rs485_2 = Rs485Transport(CH_RS485_2_UART, CH_RS485_2_TX, CH_RS485_2_RX,
                                      CH_RS485_2_DE_RE, CH_RS485_2_BAUD)

        self._usb_parser = FrameParser()
        self._ble_parser = FrameParser()
        self._rs485_parser = FrameParser()
        self._rs485_2_parser = FrameParser()

        self._global_cb = None
        self._module_cb = [None] * _MAX_MODULE_HANDLERS

    # ---- lifecycle ----------------------------------------------------
    def begin_usb(self, baud=CH_USB_BAUD):
        self.usb.begin(baud)

    def begin_ble(self, name=CH_BLE_DEFAULT_NAME):
        self.ble.begin(name)

    def begin_rs485(self, baud=CH_RS485_BAUD):
        self.rs485.begin(baud)

    def begin_rs485_2(self, baud=CH_RS485_2_BAUD):
        self.rs485_2.begin(baud)

    # ---- poll & dispatch ----------------------------------------------
    def _drain(self, transport, parser, src):
        transport.poll()
        while transport.available() > 0:
            b = transport.read()
            if b < 0:
                break
            f = parser.feed(b)
            if f is not None:
                self._dispatch(f, src)

    def update(self):
        self._drain(self.usb, self._usb_parser, "USB")
        self._drain(self.ble, self._ble_parser, "BLE")
        self._drain(self.rs485, self._rs485_parser, "RS485")
        self._drain(self.rs485_2, self._rs485_2_parser, "RS485_2")

    # ---- outgoing -----------------------------------------------------
    def send_frame(self, f):
        buf = f.serialize()
        if self.usb.connected():
            self.usb.write(buf)
        if self.ble.connected():
            self.ble.write(buf)

    def send_rs485(self, f):
        self.rs485.write(f.serialize())

    def send_rs485_2(self, f):
        self.rs485_2.write(f.serialize())

    def _request(self, transport, mod_type, mod_id, timeout_ms):
        transport.write(Frame.make_status_request(mod_type, mod_id).serialize())
        parser = FrameParser()
        deadline = time.ticks_add(time.ticks_ms(), timeout_ms)
        while time.ticks_diff(deadline, time.ticks_ms()) > 0:
            while transport.available() > 0:
                b = transport.read()
                if b < 0:
                    break
                f = parser.feed(b)
                if f is not None:
                    if (f.module_type == mod_type and f.module_id == mod_id and
                            f.msg_id == MSG_STATUS):
                        return f
                    self._dispatch(f, "RS485")
        return None

    def request_rs485(self, mod_type, mod_id, timeout_ms=50):
        return self._request(self.rs485, mod_type, mod_id, timeout_ms)

    def request_rs485_2(self, mod_type, mod_id, timeout_ms=50):
        return self._request(self.rs485_2, mod_type, mod_id, timeout_ms)

    # ---- routing ------------------------------------------------------
    def on_frame(self, cb):
        """Global catch-all handler cb(frame, source)."""
        self._global_cb = cb

    def on_module(self, module_type, cb):
        """Per-module handler; overrides global for that module type."""
        if module_type < _MAX_MODULE_HANDLERS:
            self._module_cb[module_type] = cb

    def _dispatch(self, f, src):
        if f.module_type < _MAX_MODULE_HANDLERS and self._module_cb[f.module_type]:
            self._module_cb[f.module_type](f, src)
        elif self._global_cb:
            self._global_cb(f, src)
