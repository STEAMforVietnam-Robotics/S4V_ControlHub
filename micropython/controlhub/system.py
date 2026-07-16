"""system.py -- System-level helpers. Ported from src/system/System.cpp."""

import time
import machine

from .robot import robot


class System:
    def wait_sec(self, seconds):
        time.sleep_ms(int(seconds * 1000))

    def wait_millisec(self, ms):
        time.sleep_ms(int(ms))

    def stop(self):
        robot.stop()

    def reset(self):
        machine.reset()


# Global singleton.
sys_ctrl = System()
