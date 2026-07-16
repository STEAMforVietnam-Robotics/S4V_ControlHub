"""ble_selftest.py -- Kiem tra BLE toi gian, DOC LAP voi thu vien ControlHub.

Muc dich: xac nhan firmware co BLE va dien thoai/nRF Connect THAY duoc thiet bi.
Neu file nay quang ba OK ma vi du gamepad khong thay -> loi nam o lop ble.py.

Dung module `bluetooth` truc tiep, dung service Nordic UART (NUS) giong thu vien:
  - Quang ba ten "CH-Test"
  - In khi co central ket noi / ngat
  - In moi byte central ghi vao RX characteristic

Chay:
    mpremote connect COM5 run examples/ble_selftest.py
Roi mo nRF Connect (hoac webapp/) va tim "CH-Test".
"""

import bluetooth
import struct
import time


# ---- Nordic UART Service UUIDs (giong config.py) --------------------------
_NUS_SERVICE = bluetooth.UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
_NUS_TX = bluetooth.UUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")   # hub -> host (notify)
_NUS_RX = bluetooth.UUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")   # host -> hub (write)

_FLAG_WRITE = 0x0008
_FLAG_WRITE_NR = 0x0004
_FLAG_NOTIFY = 0x0010

_IRQ_CONNECT = 1
_IRQ_DISCONNECT = 2
_IRQ_WRITE = 3

_NAME = "CH-Test"


def _add(p, t, v):
    p.extend(struct.pack("BB", len(v) + 1, t))
    p.extend(v)


def _adv_payload():
    # flags + UUID128 (21 B). UUID phai o day thi Web Bluetooth moi loc thay.
    p = bytearray()
    _add(p, 0x01, struct.pack("B", 0x06))
    _add(p, 0x07, bytes(_NUS_SERVICE))     # complete list of 128-bit UUIDs
    return p


def _resp_payload(name):
    # Ten nam o scan response de khong vuot 31 byte.
    p = bytearray()
    _add(p, 0x09, name.encode())
    return p


ble = bluetooth.BLE()
ble.active(True)
print("BLE active:", ble.active())
try:
    print("MAC:", bytes(ble.config("mac")[1]).hex(":"))
except Exception as e:
    print("khong doc duoc MAC:", e)

# Dat ten GAP (mot so app hien theo ten nay).
try:
    ble.config(gap_name=_NAME)
except Exception:
    pass

_rx_handle = None
_tx_handle = None


def irq(event, data):
    global _conn
    if event == _IRQ_CONNECT:
        conn, _, _ = data
        _conn = conn
        print(">> CONNECTED, handle =", conn)
    elif event == _IRQ_DISCONNECT:
        print(">> DISCONNECTED - quang ba lai")
        _conn = None            # phai xoa, neu khong notify se loi -128
        advertise()
    elif event == _IRQ_WRITE:
        conn, attr = data
        if attr == _rx_handle:
            buf = ble.gatts_read(_rx_handle)
            print(">> RX:", bytes(buf).hex(), buf)


ble.irq(irq)

((_tx_handle, _rx_handle),) = ble.gatts_register_services(
    ((_NUS_SERVICE, (
        (_NUS_TX, _FLAG_NOTIFY),
        (_NUS_RX, _FLAG_WRITE | _FLAG_WRITE_NR),
    )),)
)
print("Da dang ky NUS: tx_handle =", _tx_handle, " rx_handle =", _rx_handle)

_conn = None
_payload = _adv_payload()
_resp = _resp_payload(_NAME)
print("Adv payload :", bytes(_payload).hex(), "(%d byte)" % len(_payload))
print("Resp payload:", bytes(_resp).hex(), "(%d byte)" % len(_resp))


def advertise():
    # interval 100 ms, connectable undirected.
    ble.gap_advertise(100000, adv_data=_payload, resp_data=_resp, connectable=True)
    print("Dang quang ba '%s' ..." % _NAME)


advertise()

# Vong lap: moi 2s neu co ket noi thi gui 1 goi notify de test chieu hub->host.
n = 0
while True:
    if _conn is not None:
        try:
            ble.gatts_notify(_conn, _tx_handle, b"ping %d\n" % n)
            n += 1
        except Exception as e:
            print("notify loi:", e)
    time.sleep(2)
