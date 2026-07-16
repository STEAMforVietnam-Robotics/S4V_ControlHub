"""test_frame.py -- Off-board unit tests for the pure-Python protocol layer.

Runs under CPython (no hardware needed):

    python micropython/tests/test_frame.py

Verifies the two worked examples from the README / PDF and a parser round-trip.
"""

import os
import sys

# Make `controlhub` importable when run from anywhere.
_HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(_HERE))   # .../micropython

from controlhub.crc8 import crc8_smbus                       # noqa: E402
from controlhub.frame import Frame, FrameParser              # noqa: E402
from controlhub.config import (                              # noqa: E402
    MOD_TOF, MSG_STATUS, MSG_CONTROL, MODE_DUTY,
)

_fail = 0


def check(name, got, want):
    global _fail
    ok = got == want
    if not ok:
        _fail += 1
    print("[%s] %s: got=%s want=%s" % ("PASS" if ok else "FAIL", name, got, want))


# 1. CRC of the two PDF examples.
#    "Request ToF status":   55 03 00 11 01 00 -> CRC 1F
ex1 = bytes([0x55, 0x03, 0x00, 0x11, 0x01, 0x00])
check("crc ToF request", crc8_smbus(ex1), 0x1F)
#    "Motor duty 0x0064":    55 05 00 01 00 01 64 -> ... but README byte layout:
#    55 03 00 11 00 01 64 -> CRC 18  (matches the PDF Example II verbatim)
ex2 = bytes([0x55, 0x03, 0x00, 0x11, 0x00, 0x01, 0x64])
check("crc motor duty", crc8_smbus(ex2), 0x18)

# 2. Frame.serialize() reproduces the exact wire bytes.
f1 = Frame.make_status_request(MOD_TOF, 0x00, MSG_STATUS)
check("serialize ToF request", f1.serialize().hex(), "5503001101001f")

f2 = Frame(MOD_TOF, 0x00, MSG_STATUS, 0, 1, [0x64])
check("serialize motor duty", f2.serialize().hex(), "550300110001 6418".replace(" ", ""))

# 3. Parser round-trip: feed the serialized bytes back and recover the Frame.
parser = FrameParser()
recovered = None
for b in f2.serialize():
    r = parser.feed(b)
    if r is not None:
        recovered = r
check("roundtrip module_type", recovered.module_type, MOD_TOF)
check("roundtrip dlc", recovered.dlc, 1)
check("roundtrip data0", recovered.data[0], 0x64)

# 4. A frame with a corrupted CRC must be rejected.
bad = bytearray(f2.serialize())
bad[-1] ^= 0xFF
parser2 = FrameParser()
got_any = False
for b in bad:
    if parser2.feed(b) is not None:
        got_any = True
check("reject bad crc", got_any, False)

# 5. make_motor_control encodes signed duty x100 big-endian.
mc = Frame.make_motor_control(1, 100 * 100, MODE_DUTY)   # 100.00% -> 10000 = 0x2710
check("motor control hi", mc.data[0], 0x27)
check("motor control lo", mc.data[1], 0x10)
check("motor control mode", mc.data[2], MODE_DUTY)

print()
if _fail:
    print("%d test(s) FAILED" % _fail)
    sys.exit(1)
print("All tests passed.")
