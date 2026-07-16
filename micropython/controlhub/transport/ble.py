"""ble.py -- BLE Nordic UART Service peripheral transport.

Ported from src/transport/BleTransport.cpp using MicroPython's `bluetooth`
module (IRQ-based). Bytes written by the central to the RX characteristic are
pushed into a ring buffer that update()/poll() drains; write() notifies on the
TX characteristic in MTU-sized chunks.

If the firmware has no BLE support, the transport degrades gracefully (like the
C++ CH_NO_BLE build) and simply reports never-connected.
"""

import struct

from ..config import (
    CH_BLE_DEFAULT_NAME,
    CH_BLE_NUS_SERVICE_UUID, CH_BLE_NUS_RX_CHAR_UUID, CH_BLE_NUS_TX_CHAR_UUID,
)

try:
    import bluetooth
    _HAS_BLE = True
except ImportError:
    bluetooth = None
    _HAS_BLE = False

# IRQ event codes
_IRQ_CENTRAL_CONNECT = 1
_IRQ_CENTRAL_DISCONNECT = 2
_IRQ_GATTS_WRITE = 3

_FLAG_WRITE = 0x0008
_FLAG_WRITE_NO_RESPONSE = 0x0004
_FLAG_NOTIFY = 0x0010

_ADV_TYPE_FLAGS = 0x01
_ADV_TYPE_NAME = 0x09
_ADV_TYPE_UUID128_COMPLETE = 0x07

_RX_BUF_SIZE = 256


def _append(payload, adv_type, value):
    payload.extend(struct.pack("BB", len(value) + 1, adv_type))
    payload.extend(value)


def _adv_payload(service_uuid):
    # Advertising packet: flags (3 B) + complete 128-bit service UUID (18 B) = 21 B.
    # The NUS UUID MUST be here: Web Bluetooth (webapp/js/ble.js) scans with
    # `filters: [{ services: [NUS_SERVICE] }]` and only matches devices that
    # advertise that UUID. nRF Connect lists everything, which is why it found
    # the board even when the UUID was missing.
    payload = bytearray()
    _append(payload, _ADV_TYPE_FLAGS, struct.pack("B", 0x06))
    _append(payload, _ADV_TYPE_UUID128_COMPLETE, bytes(service_uuid))
    return payload


def _resp_payload(name):
    # Scan response: the name goes here so the 31-byte advertising limit is not
    # exceeded. Central still resolves device.name from the scan response.
    payload = bytearray()
    _append(payload, _ADV_TYPE_NAME, name.encode())
    return payload


class BleTransport:
    def __init__(self):
        self._started = False
        self._connected = False
        self._conn_handle = None
        self._ble = None
        self._tx_handle = None
        self._rx = bytearray(_RX_BUF_SIZE)
        self._head = 0
        self._tail = 0

    def begin(self, name=CH_BLE_DEFAULT_NAME):
        if self._started or not _HAS_BLE:
            return
        self._ble = bluetooth.BLE()
        self._ble.active(True)
        self._ble.irq(self._irq)

        svc_uuid = bluetooth.UUID(CH_BLE_NUS_SERVICE_UUID)
        tx = (bluetooth.UUID(CH_BLE_NUS_TX_CHAR_UUID), _FLAG_NOTIFY)
        rx = (bluetooth.UUID(CH_BLE_NUS_RX_CHAR_UUID),
              _FLAG_WRITE | _FLAG_WRITE_NO_RESPONSE)
        ((self._tx_handle, self._rx_handle),) = self._ble.gatts_register_services(
            ((svc_uuid, (tx, rx)),))

        self._name = name
        self._payload = _adv_payload(svc_uuid)
        self._resp = _resp_payload(name)
        self._advertise()
        self._started = True

    def _advertise(self):
        # 100 ms interval, connectable. The service UUID rides in adv_data and
        # the name in the scan response - see _adv_payload().
        self._ble.gap_advertise(100000, adv_data=self._payload,
                                resp_data=self._resp, connectable=True)

    # ---- IRQ ----------------------------------------------------------
    def _irq(self, event, data):
        # An exception escaping an IRQ leaves the BLE stack in a bad state and
        # usually shows up as an instant disconnect, so trap and report instead.
        try:
            if event == _IRQ_CENTRAL_CONNECT:
                conn_handle, _, _ = data
                self._conn_handle = conn_handle
                self._connected = True
            elif event == _IRQ_CENTRAL_DISCONNECT:
                self._connected = False
                self._conn_handle = None
                self._advertise()           # re-advertise for the next client
            elif event == _IRQ_GATTS_WRITE:
                conn_handle, value_handle = data
                if value_handle == self._rx_handle:
                    buf = self._ble.gatts_read(self._rx_handle)
                    self._push(buf)
        except Exception as e:
            print("BLE irq loi (event=%d): %r" % (event, e))

    def _push(self, buf):
        for b in buf:
            nxt = (self._head + 1) % _RX_BUF_SIZE
            if nxt == self._tail:
                return                      # drop on overflow
            self._rx[self._head] = b
            self._head = nxt

    # ---- Transport interface ------------------------------------------
    def connected(self):
        return self._started and self._connected

    def poll(self):
        pass                                # IRQ fills the buffer already

    def available(self):
        if not self._started:
            return 0
        return (self._head - self._tail + _RX_BUF_SIZE) % _RX_BUF_SIZE

    def read(self):
        if not self._started or self._head == self._tail:
            return -1
        b = self._rx[self._tail]
        self._tail = (self._tail + 1) % _RX_BUF_SIZE
        return b

    def write(self, data):
        if not self.connected() or self._tx_handle is None:
            return 0
        CHUNK = 180
        n = len(data)
        sent = 0
        while sent < n:
            end = sent + CHUNK
            if end > n:
                end = n
            try:
                self._ble.gatts_notify(self._conn_handle, self._tx_handle,
                                       data[sent:end])
            except Exception:
                break
            sent = end
        return sent
