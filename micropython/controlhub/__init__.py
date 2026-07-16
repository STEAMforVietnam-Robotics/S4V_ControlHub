"""ControlHub -- MicroPython library for the Control Hub board (ESP32-S3).

Port of the Arduino/C++ ControlHub library. Typical sketch:

    from controlhub import control_hub as hub, robot, Motor, Servo, Ultrasonic, \
        Contact, M1, M2, S1, PORT_MF4P_1

    left, right = Motor(M1), Motor(M2)
    gripper = Servo(S1)
    dist = Ultrasonic(PORT_MF4P_1)

    hub.begin()                       # USB + BLE + RS485, brings up all modules
    robot.set_drive_train(left, right)
    robot.set_gripper(gripper)
    robot.set_distance(dist, 250)

    while True:
        hub.update()
        if robot.is_obstacle_detected():
            robot.stop()
        else:
            robot.go_forward(60)

The hardware-facing pieces (transports, HAL, modules) import `machine` and are
only available on MicroPython. The pure-Python protocol layer (frame, crc8,
config) imports cleanly under CPython too, so it can be unit-tested off-board.
"""

# ---- Pure-Python protocol layer (always importable) -----------------------
from .config import *          # noqa: F401,F403  -- pin map + all constants
from .frame import Frame, FrameParser
from .crc8 import crc8_smbus

# ---- Hardware layer (MicroPython only) ------------------------------------
try:
    from .controlhub import ControlHub, control_hub
    from .robot import Robot, robot
    from .system import System, sys_ctrl
    from .modules.motor import Motor
    from .modules.servo import Servo
    from .modules.ultrasonic import Ultrasonic
    from .modules.contact import Contact
    from .modules.smart_tof import SmartToF
    from .modules.smart_bumper import SmartBumper
    from .modules.led import Led
    from .modules.gamepad import Gamepad
    from .modules.phone_sensor import PhoneSensor
except ImportError as e:
    # On CPython (no `machine`) this is expected: only the protocol layer is
    # exposed. On the board, though, a missing name inside our own modules would
    # also land here and silently hide the class exports - so surface anything
    # that is NOT just the absent `machine` module.
    msg = str(e)
    if "machine" not in msg:
        raise
