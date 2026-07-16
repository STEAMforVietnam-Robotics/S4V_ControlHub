"""controlhub.py -- Top-level facade. Ported from src/ControlHub.{h,cpp}.

Brings up I2C + PCA9685, the three transports, routes incoming frames to the
matching on-board module, and drives the on-board WS2812 status LED.
"""

from machine import Pin, I2C
import neopixel

from .config import (
    CH_I2C_SDA, CH_I2C_SCL, CH_I2C_FREQ, CH_PCA9685_ADDR, CH_PCA9685_FREQ,
    CH_USB_BAUD, CH_BLE_DEFAULT_NAME, CH_RS485_BAUD, CH_RS485_2_BAUD,
    CH_LED_STATUS_PIN, CH_LED_STATUS_NUM,
    MOD_MOTOR, MOD_SERVO, MOD_ULTRASONIC, MOD_BUMPER, MOD_TOF,
    MOD_POWER, MOD_IMU, MOD_LED, MOD_GAMEPAD, MOD_PHONE_SENSOR,
)
from .core.commhub import CommHub
from .hal.pca9685 import PCA9685
from . import modules as _modpkg
from .modules import module as _modmod


_ROUTED_TYPES = (MOD_MOTOR, MOD_SERVO, MOD_ULTRASONIC, MOD_BUMPER, MOD_TOF,
                 MOD_POWER, MOD_IMU, MOD_LED, MOD_GAMEPAD, MOD_PHONE_SENSOR)


class ControlHub:
    def __init__(self):
        self.comm = CommHub()
        self._pca = None
        self._pca_done = False
        self._status = None
        self._status_brightness = 64
        self._status_begun = False
        self._blink_on = False
        _modmod.set_hub(self)

    # ---- PCA / I2C ----------------------------------------------------
    def _init_pca(self):
        # Bring up the motor driver's PCA9685 lazily. This is best-effort: an
        # RS485-only or sensor-only sketch (e.g. Smart Module scan) must still
        # come up even when no PCA9685 is on the I2C bus. Motor use without a
        # working PCA9685 is what fails later, not begin().
        if self._pca_done:
            return
        self._pca_done = True
        try:
            i2c = I2C(0, sda=Pin(CH_I2C_SDA), scl=Pin(CH_I2C_SCL), freq=CH_I2C_FREQ)
            self._pca = PCA9685(i2c, CH_PCA9685_ADDR, CH_PCA9685_FREQ)
            _modmod.set_pca(self._pca)
        except Exception as e:
            self._pca = None
            print("ControlHub: PCA9685 not initialised (%s) - motors disabled" % e)

    # ---- lifecycle ----------------------------------------------------
    def begin(self, usb=True, ble=True, rs485=True, ble_name=CH_BLE_DEFAULT_NAME):
        self._init_pca()
        if usb:
            self.comm.begin_usb(CH_USB_BAUD)
        if ble:
            self.comm.begin_ble(ble_name)
        if rs485:
            self.comm.begin_rs485(CH_RS485_BAUD)
            self.comm.begin_rs485_2(CH_RS485_2_BAUD)
        self._install_default_handlers()
        self._begin_modules()

    def begin_usb(self, baud=CH_USB_BAUD):
        self._init_pca()
        self.comm.begin_usb(baud)
        self._install_default_handlers()
        self._begin_modules()

    def begin_ble(self, name=CH_BLE_DEFAULT_NAME):
        self._init_pca()
        self.comm.begin_ble(name)
        self._install_default_handlers()
        self._begin_modules()

    def begin_rs485(self, baud=CH_RS485_BAUD):
        self._init_pca()
        self.comm.begin_rs485(baud)
        self._install_default_handlers()

    def begin_rs485_2(self, baud=CH_RS485_2_BAUD):
        self._init_pca()
        self.comm.begin_rs485_2(baud)
        self._install_default_handlers()

    def _begin_modules(self):
        for m in _modmod.registry():
            try:
                m.begin()
            except Exception:
                pass

    def update(self):
        self.comm.update()

    # ---- outgoing (facade for modules & user) -------------------------
    def send_frame(self, f):
        self.comm.send_frame(f)

    def send_rs485(self, f):
        self.comm.send_rs485(f)

    def send_rs485_2(self, f):
        self.comm.send_rs485_2(f)

    def on_frame(self, cb):
        self.comm.on_frame(cb)

    # ---- routing ------------------------------------------------------
    @staticmethod
    def _from_host(src):
        return src != "RS485" and src != "RS485_2"

    def _route(self, f, src):
        for m in _modmod.registry():
            if m.module_type() == f.module_type and m.id() == f.module_id:
                m.handle_frame(f, self)
                return
        if not self._from_host(src):
            self.comm.send_frame(f)          # relay smart-module reply -> host

    def _install_default_handlers(self):
        for t in _ROUTED_TYPES:
            self.comm.on_module(t, self._route)

    # ---- on-board WS2812 status LED -----------------------------------
    def _init_status_led(self):
        if self._status_begun:
            return
        self._status = neopixel.NeoPixel(Pin(CH_LED_STATUS_PIN), CH_LED_STATUS_NUM)
        self._status_begun = True
        self._status_fill(0, 0, 0)

    def _status_fill(self, r, g, b):
        n = CH_LED_STATUS_NUM
        br = self._status_brightness
        col = (r * br // 255, g * br // 255, b * br // 255)
        for i in range(n):
            self._status[i] = col
        self._status.write()

    def status_color(self, r, g, b):
        self._init_status_led()
        self._status_fill(r, g, b)

    def status_pixel(self, i, r, g, b):
        self._init_status_led()
        if i >= CH_LED_STATUS_NUM:
            return
        br = self._status_brightness
        self._status[i] = (r * br // 255, g * br // 255, b * br // 255)
        self._status.write()

    def status_brightness(self, b):
        self._status_brightness = b
        self._init_status_led()

    def status_blink(self, r, g, b):
        self._init_status_led()
        self._blink_on = not self._blink_on
        if self._blink_on:
            self._status_fill(r, g, b)
        else:
            self._status_fill(0, 0, 0)

    def status_off(self):
        self._init_status_led()
        self._status_fill(0, 0, 0)

    def status_ok(self):
        self.status_color(0, 255, 0)

    def status_error(self):
        self.status_color(255, 0, 0)

    def status_warning(self):
        self.status_color(255, 128, 0)

    def status_busy(self):
        self.status_color(0, 0, 255)


# Global singleton the sketches interact with.
control_hub = ControlHub()
