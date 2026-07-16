"""pca9685.py -- Minimal PCA9685 16-channel PWM driver over machine.I2C.

Only the features the Control Hub needs: set the output frequency and write a
12-bit on/off pair per channel. Mirrors Adafruit_PWMServoDriver.setPWM().
"""

import math
import time

# Register map
_MODE1 = 0x00
_PRESCALE = 0xFE
_LED0_ON_L = 0x06

_RESTART = 0x80
_SLEEP = 0x10
_ALLCALL = 0x01
_AI = 0x20            # auto-increment


class PCA9685:
    def __init__(self, i2c, address=0x40, freq=1500):
        self.i2c = i2c
        self.address = address
        self._buf1 = bytearray(1)
        self._buf4 = bytearray(4)
        self.reset()
        self.set_freq(freq)

    def _write8(self, reg, value):
        self._buf1[0] = value & 0xFF
        self.i2c.writeto_mem(self.address, reg, self._buf1)

    def _read8(self, reg):
        return self.i2c.readfrom_mem(self.address, reg, 1)[0]

    def reset(self):
        self._write8(_MODE1, _AI | _ALLCALL)
        time.sleep_ms(5)

    def set_freq(self, freq):
        # PCA9685 internal oscillator is 25 MHz, 12-bit (4096) resolution.
        prescale = int(math.floor(25000000.0 / (4096 * freq) + 0.5)) - 1
        if prescale < 3:
            prescale = 3
        if prescale > 255:
            prescale = 255
        old_mode = self._read8(_MODE1)
        self._write8(_MODE1, (old_mode & 0x7F) | _SLEEP)   # sleep to set prescale
        self._write8(_PRESCALE, prescale)
        self._write8(_MODE1, old_mode)
        time.sleep_ms(5)
        self._write8(_MODE1, old_mode | _RESTART | _AI)

    def set_pwm(self, channel, on, off):
        """Set the 12-bit ON and OFF tick for a channel (0..15)."""
        b = self._buf4
        b[0] = on & 0xFF
        b[1] = (on >> 8) & 0x0F
        b[2] = off & 0xFF
        b[3] = (off >> 8) & 0x0F
        self.i2c.writeto_mem(self.address, _LED0_ON_L + 4 * channel, b)

    def set_off(self, channel, value12):
        """Duty helper matching DRV8874.cpp::_write().

        value12 == 0     -> fully off  (setPWM(ch, 0, 4096))
        value12 >= 4095  -> fully on   (setPWM(ch, 4096, 0))
        otherwise        -> setPWM(ch, 0, value12)
        """
        if value12 <= 0:
            self.set_pwm(channel, 0, 4096)
        elif value12 >= 4095:
            self.set_pwm(channel, 4096, 0)
        else:
            self.set_pwm(channel, 0, value12)
