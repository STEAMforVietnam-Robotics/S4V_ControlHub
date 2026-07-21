"""commhub.py -- Owns the three transports, runs a FrameParser on each stream,
and dispatches valid frames to per-module handlers or a global callback.

Ported from src/core/CommHub.{h,cpp}.
"""

import sys
import time

try:
    import uselect as select
except ImportError:            # CPython fallback (for host-side tests)
    import select

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

try:
    import micropython
except ImportError:
    micropython = None

_MAX_MODULE_HANDLERS = 16


def _restore_kbd_intr():
    if micropython is not None:
        try:
            micropython.kbd_intr(3)      # restore default Ctrl-C char
        except Exception:
            pass


class _ConsoleWatcher:
    """Watches sys.stdin for a raw Ctrl-C byte once kbd_intr() is disabled.

    Only needed when RS485_1 disables kbd_intr (see begin_rs485) but nothing
    else is already reading stdin -- if UsbTransport is active it owns stdin
    for the binary protocol and does its own Ctrl-C detection instead (see
    CommHub._drain), so this watcher must stay off to avoid stealing bytes
    from it.
    """

    def __init__(self):
        self._poll = None
        try:
            self._in = sys.stdin.buffer
        except AttributeError:
            self._in = sys.stdin

    def begin(self):
        if self._poll is not None:
            return
        self._poll = select.poll()
        self._poll.register(self._in, select.POLLIN)

    def end(self):
        if self._poll is None:
            return
        try:
            self._poll.unregister(self._in)
        except Exception:
            pass
        self._poll = None

    def check(self):
        if self._poll is None:
            return False
        while self._poll.poll(0):
            b = self._in.read(1)
            if not b:
                break
            if b[0] == 0x03:
                return True
        return False


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

        self._console = _ConsoleWatcher()
        self._console_active = False

    # ---- lifecycle ----------------------------------------------------
    def begin_usb(self, baud=CH_USB_BAUD):
        self.usb.begin(baud)
        # UsbTransport now owns stdin for the binary protocol (and its own
        # Ctrl-C detection in _drain) -- stop our stand-in watcher so the two
        # don't race for the same bytes if begin_rs485() ran first.
        if self._console_active:
            self._console.end()
            self._console_active = False

    def begin_ble(self, name=CH_BLE_DEFAULT_NAME):
        self.ble.begin(name)

    def begin_rs485(self, baud=CH_RS485_BAUD):
        self.rs485.begin(baud)
        # RS485_1 shares GPIO43/44 with the board's UART0 console pins (see
        # config.py). Every Smart ToF reply carries a literal 0x03 byte
        # (MOD_TOF, the byte right after the frame sync) which the
        # firmware's own Ctrl-C detector on that console would otherwise
        # misread as a keypress and abort the program on every single ToF
        # read. Disable it here; _drain()/_request() do their own
        # frame-boundary aware Ctrl-C detection instead.
        if micropython is not None:
            try:
                micropython.kbd_intr(-1)
            except Exception:
                pass
        # If USB isn't active, nothing else reads stdin -- start our own
        # watcher so Ctrl-C typed at the REPL can still be recognised (it no
        # longer raises on its own now that kbd_intr is disabled).
        if not self.usb.connected():
            self._console.begin()
            self._console_active = True

    def begin_rs485_2(self, baud=CH_RS485_2_BAUD):
        self.rs485_2.begin(baud)

    # ---- poll & dispatch ----------------------------------------------
    def _drain(self, transport, parser, src):
        transport.poll()
        while transport.available() > 0:
            b = transport.read()
            if b < 0:
                break
            # The USB transport disables the firmware's own Ctrl-C handling
            # (see UsbTransport.begin) so a raw byte 0x03 does not corrupt
            # binary frames. A 0x55 sync byte can never be mistaken for a
            # 0x03, so a lone 0x03 while the parser is idle (between frames)
            # is unambiguously a Ctrl-C from the host, not payload data --
            # honour it as a stop request. RS485 does NOT get this check:
            # real keystrokes never reach that bus (they arrive over the
            # console/stdin, watched separately by _ConsoleWatcher), so
            # treating a stray 0x03 there as Ctrl-C only risks false
            # positives from ordinary bus noise/glitches.
            if src == "USB" and b == 0x03 and parser.is_idle():
                transport.end()
                _restore_kbd_intr()
                raise KeyboardInterrupt
            f = parser.feed(b)
            if f is not None:
                self._dispatch(f, src)

    def update(self):
        if self._console_active and self._console.check():
            _restore_kbd_intr()
            raise KeyboardInterrupt
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

    def _request(self, transport, mod_type, mod_id, timeout_ms, src):
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
                    self._dispatch(f, src)
        return None

    def request_rs485(self, mod_type, mod_id, timeout_ms=50):
        return self._request(self.rs485, mod_type, mod_id, timeout_ms, "RS485")

    def request_rs485_2(self, mod_type, mod_id, timeout_ms=50):
        return self._request(self.rs485_2, mod_type, mod_id, timeout_ms, "RS485_2")

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
