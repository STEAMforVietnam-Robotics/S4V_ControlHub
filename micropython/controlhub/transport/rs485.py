"""rs485.py -- Half-duplex RS485 Smart Module bus.

Ported from src/transport/Rs485Transport.cpp. Uses a machine.UART plus a
combined DE/RE pin: driven high while transmitting, low while receiving.
"""

from machine import UART, Pin
import time


class Rs485Transport:
    def __init__(self, uart_id, tx_pin, rx_pin, de_re_pin, baud=115200):
        self._uart_id = uart_id
        self._tx = tx_pin
        self._rx = rx_pin
        self._de_re_pin = de_re_pin
        self._baud = baud
        self._uart = None
        self._de = None
        self._begun = False

    def begin(self, baud=None):
        if self._begun:
            return
        if baud:
            self._baud = baud
        self._uart = UART(self._uart_id, baudrate=self._baud,
                          tx=self._tx, rx=self._rx,
                          bits=8, parity=None, stop=1, timeout=5)
        self._de = Pin(self._de_re_pin, Pin.OUT)
        self._de.value(0)                 # receive mode by default
        self._begun = True

    def connected(self):
        return self._begun

    def poll(self):
        pass                              # UART buffers internally

    def available(self):
        if not self._begun:
            return 0
        n = self._uart.any()
        return n if n else 0

    def read(self):
        if not self._begun:
            return -1
        b = self._uart.read(1)
        if not b:
            return -1
        return b[0]

    def write(self, data):
        if not self._begun:
            return
        self._de.value(1)                 # enable driver (transmit)
        self._uart.write(data)
        # Wait until the bytes have actually left the shift register before
        # releasing the driver, so we don't cut off the tail of the frame.
        try:
            self._uart.flush()
        except AttributeError:
            # Fallback: ~10 bits per byte at the configured baud rate.
            us = int(len(data) * 10 * 1000000 / self._baud) + 100
            time.sleep_us(us)
        self._de.value(0)                 # back to receive mode
