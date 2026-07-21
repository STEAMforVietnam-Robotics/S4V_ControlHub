"""frame.py -- Wire-format frame used on RS485, USB Serial and BLE.

    +--------+-----------+-----------+--------+-----+-----+----------+------+
    | Header | ModuleType| ModuleID  | MsgID  | RTR | DLC | Data...  | CRC8 |
    |  0x55  |   1 B     |   1 B     |  1 B   | 1 B | 1 B | 0..8 B   | 1 B  |
    +--------+-----------+-----------+--------+-----+-----+----------+------+

Ported from src/protocol/Frame.{h,cpp}. Layout, semantics and the CRC-8
algorithm match RVRS485060726162210.pdf and the C++ library exactly.
"""

from .config import (
    CH_FRAME_HEADER, CH_FRAME_MAX_DATA,
    MOD_MOTOR, MSG_CONTROL, MSG_CONFIG, MSG_STATUS, MODE_DUTY,
)
from .crc8 import crc8_smbus


def _u8(v):
    return v & 0xFF


class Frame:
    __slots__ = ("module_type", "module_id", "msg_id", "rtr", "dlc", "data", "crc")

    def __init__(self, module_type=0, module_id=0, msg_id=0, rtr=0, dlc=0, data=None):
        self.module_type = module_type
        self.module_id = module_id
        self.msg_id = msg_id
        self.rtr = rtr
        self.dlc = dlc
        # Always 8 bytes of backing storage; dlc says how many are valid.
        self.data = bytearray(CH_FRAME_MAX_DATA)
        if data:
            for i in range(min(len(data), CH_FRAME_MAX_DATA)):
                self.data[i] = data[i] & 0xFF
        self.crc = 0

    # ---- CRC / serialization ------------------------------------------
    def compute_crc(self):
        """CRC over Header .. last data byte (total = 6 + dlc)."""
        n = self.dlc if self.dlc <= CH_FRAME_MAX_DATA else CH_FRAME_MAX_DATA
        buf = bytearray(6 + n)
        buf[0] = CH_FRAME_HEADER
        buf[1] = _u8(self.module_type)
        buf[2] = _u8(self.module_id)
        buf[3] = _u8(self.msg_id)
        buf[4] = _u8(self.rtr)
        buf[5] = _u8(self.dlc)
        for i in range(n):
            buf[6 + i] = self.data[i]
        return crc8_smbus(buf)

    def serialize(self):
        """Return the on-wire bytes (7 .. 15 bytes), CRC appended."""
        n = self.dlc if self.dlc <= CH_FRAME_MAX_DATA else CH_FRAME_MAX_DATA
        out = bytearray(7 + n)
        out[0] = CH_FRAME_HEADER
        out[1] = _u8(self.module_type)
        out[2] = _u8(self.module_id)
        out[3] = _u8(self.msg_id)
        out[4] = _u8(self.rtr)
        out[5] = _u8(self.dlc)
        for i in range(n):
            out[6 + i] = self.data[i]
        self.crc = self.compute_crc()
        out[6 + n] = self.crc
        return bytes(out)

    def __repr__(self):
        return "Frame(type=0x%02X id=%d msg=0x%02X rtr=%d dlc=%d data=%s)" % (
            self.module_type, self.module_id, self.msg_id, self.rtr,
            self.dlc, bytes(self.data[:self.dlc]).hex())

    # ---- Factory helpers ----------------------------------------------
    @staticmethod
    def make_motor_control(motor_id, duty_x100, mode=MODE_DUTY):
        d = duty_x100 & 0xFFFF
        return Frame(MOD_MOTOR, motor_id, MSG_CONTROL, 0, 3,
                     [(d >> 8) & 0xFF, d & 0xFF, mode & 0xFF])

    @staticmethod
    def make_motor_config(current_id, new_id):
        return Frame(MOD_MOTOR, current_id, MSG_CONFIG, 0, 1, [new_id & 0xFF])

    @staticmethod
    def make_status_request(module_type, module_id, msg_id=MSG_STATUS):
        f = Frame(module_type, module_id, msg_id, 1, 0)
        return f


# ---- Stream parser -------------------------------------------------------
_S_HEADER, _S_MODTYPE, _S_MODID, _S_MSGID, _S_RTR, _S_DLC, _S_DATA, _S_CRC = range(8)


class FrameParser:
    """Byte-at-a-time parser for UART / BLE / USB streams.

    feed(byte) returns a Frame when a complete, CRC-valid frame arrives, else
    None. Malformed frames are dropped and the parser resyncs on 0x55.
    """

    def __init__(self):
        self._state = _S_HEADER
        self._idx = 0
        self._work = Frame()

    def reset(self):
        self._state = _S_HEADER
        self._idx = 0

    def is_idle(self):
        """True between frames (waiting for the 0x55 sync byte).

        A byte value can never legitimately be mid-frame data while idle, so
        callers can use this to safely recognise out-of-band control bytes
        (e.g. Ctrl-C) without risking a collision with real payload bytes.
        """
        return self._state == _S_HEADER

    def feed(self, b):
        b &= 0xFF
        st = self._state
        if st == _S_HEADER:
            if b == CH_FRAME_HEADER:
                self._work = Frame()
                self._state = _S_MODTYPE
        elif st == _S_MODTYPE:
            self._work.module_type = b
            self._state = _S_MODID
        elif st == _S_MODID:
            self._work.module_id = b
            self._state = _S_MSGID
        elif st == _S_MSGID:
            self._work.msg_id = b
            self._state = _S_RTR
        elif st == _S_RTR:
            self._work.rtr = b
            self._state = _S_DLC
        elif st == _S_DLC:
            if b > CH_FRAME_MAX_DATA:
                self.reset()            # bogus length -> resync
                return None
            self._work.dlc = b
            self._idx = 0
            self._state = _S_CRC if b == 0 else _S_DATA
        elif st == _S_DATA:
            self._work.data[self._idx] = b
            self._idx += 1
            if self._idx >= self._work.dlc:
                self._state = _S_CRC
        elif st == _S_CRC:
            self._work.crc = b
            expect = self._work.compute_crc()
            frame = self._work
            self.reset()
            if expect == b:
                return frame
        return None
