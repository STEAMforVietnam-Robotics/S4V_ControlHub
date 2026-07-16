"""drv8874.py -- DRV8874 H-bridge driven by two PCA9685 channels.

Ported from src/hal/DRV8874.cpp. Two channels per motor (IN1 = forward,
IN2 = reverse); direction is chosen by which channel gets the duty.
"""

from ..config import STOP_COAST, STOP_BRAKE


class DRV8874:
    def __init__(self, pca, in1_ch, in2_ch):
        self._pca = pca
        self._in1 = in1_ch
        self._in2 = in2_ch
        self.stop(STOP_COAST)      # start coasting so the motor doesn't twitch

    def drive(self, pct):
        """pct in -100..100. Positive = forward via IN1, negative = IN2."""
        if pct > 100.0:
            pct = 100.0
        elif pct < -100.0:
            pct = -100.0

        duty = int(abs(pct) * 40.95)      # magnitude -> 12-bit duty (0..4095)
        if pct > 0.0:
            self._pca.set_off(self._in1, duty)
            self._pca.set_off(self._in2, 0)
        elif pct < 0.0:
            self._pca.set_off(self._in1, 0)
            self._pca.set_off(self._in2, duty)
        else:
            self.stop(STOP_COAST)

    def stop(self, mode=STOP_COAST):
        if mode == STOP_BRAKE:
            self._pca.set_off(self._in1, 4095)
            self._pca.set_off(self._in2, 4095)
        else:  # STOP_COAST
            self._pca.set_off(self._in1, 0)
            self._pca.set_off(self._in2, 0)
