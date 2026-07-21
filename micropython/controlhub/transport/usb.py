"""usb.py -- Binary host transport over the native USB-CDC (REPL) channel.

On the ESP32-S3 the native USB-CDC is also the MicroPython REPL. To exchange
binary frames on it we disable the REPL's Ctrl-C interrupt (micropython.kbd_intr)
and read raw bytes non-blocking from sys.stdin.buffer via a poller. Writes go
to sys.stdout.buffer.

NOTE: while the USB transport is running the REPL will not react to Ctrl-C;
reset the board (EN button) to get the REPL back.
"""

import sys

try:
    import uselect as select
except ImportError:            # CPython fallback (for host-side tests)
    import select

try:
    import micropython
except ImportError:
    micropython = None


class UsbTransport:
    def __init__(self):
        self._begun = False
        self._poll = None
        self._rx = bytearray()
        self._pos = 0
        try:
            self._in = sys.stdin.buffer
            self._out = sys.stdout.buffer
        except AttributeError:     # some ports expose stdin/stdout directly
            self._in = sys.stdin
            self._out = sys.stdout

    def begin(self, baud=None):
        if self._begun:
            return
        # Stop Ctrl-C from raising KeyboardInterrupt inside binary data.
        if micropython is not None:
            try:
                micropython.kbd_intr(-1)
            except Exception:
                pass
        self._poll = select.poll()
        self._poll.register(self._in, select.POLLIN)
        self._begun = True

    def end(self):
        """Give the native USB-CDC channel back to the REPL.

        Called once a Ctrl-C stop request has been recognised (see
        CommHub._drain / FrameParser.is_idle) so Ctrl-C and file access work
        normally again without needing a physical reset.
        """
        if not self._begun:
            return
        self._begun = False
        if micropython is not None:
            try:
                micropython.kbd_intr(3)   # restore default interrupt char
            except Exception:
                pass

    def connected(self):
        # USB-CDC is effectively always "connected" once begin() ran.
        return self._begun

    def poll(self):
        if not self._begun:
            return
        # Drain everything currently available into the local buffer.
        while self._poll.poll(0):
            chunk = self._in.read(1)
            if not chunk:
                break
            self._rx.extend(chunk)

    def available(self):
        return len(self._rx) - self._pos

    def read(self):
        # bytearray has no item deletion in MicroPython, so consume via an
        # index and compact once the buffer has been fully drained.
        if self._pos >= len(self._rx):
            return -1
        b = self._rx[self._pos]
        self._pos += 1
        if self._pos >= len(self._rx):
            self._rx = bytearray()
            self._pos = 0
        return b

    def write(self, data):
        if not self._begun:
            return
        try:
            self._out.write(data)
        except Exception:
            pass
