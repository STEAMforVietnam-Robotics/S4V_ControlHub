"""robot.py -- LEVEL 1 "Mission" API. Ported from src/robot/Robot.cpp.

Glues a pair of drive motors, a gripper servo, a distance sensor and a contact
sensor into simple verbs (go_forward, turn_left, is_obstacle_detected, ...).
"""

import time


class Robot:
    def __init__(self):
        self._left = None
        self._right = None
        self._gripper = None
        self._open_ang = 20
        self._closed_ang = 160
        self._dist = None
        self._dist_threshold = 200
        self._contact = None

    def begin(self):
        pass                                # motors begin() themselves

    # ---- configuration hooks ------------------------------------------
    def set_drive_train(self, left, right):
        self._left = left
        self._right = right

    def invert_right(self, on=True):
        # kept for parity with the Arduino example; flips right motor sense
        if self._right is not None:
            self._right._reverse = bool(on)

    def set_gripper(self, servo, open_angle=20, closed_angle=160):
        self._gripper = servo
        self._open_ang = open_angle
        self._closed_ang = closed_angle

    def set_distance(self, sensor, threshold_mm=200):
        self._dist = sensor
        self._dist_threshold = threshold_mm

    def set_contact(self, sensor):
        self._contact = sensor

    # ---- movement primitives ------------------------------------------
    def _drive_raw(self, left, right):
        if self._left is None or self._right is None:
            return
        self._left.rotate(left)
        self._right.rotate(right)

    def wait(self, seconds):
        time.sleep_ms(int(seconds * 1000))

    def stop(self):
        self._drive_raw(0, 0)

    def go_forward(self, speed=50):
        self._drive_raw(speed, speed)

    def go_backward(self, speed=50):
        self._drive_raw(-speed, -speed)

    def turn_left(self, speed=50):
        self._drive_raw(-speed, speed)

    def turn_right(self, speed=50):
        self._drive_raw(speed, -speed)

    # ---- gripper ------------------------------------------------------
    def grasp(self):
        if self._gripper:
            self._gripper.write(self._closed_ang)

    def release(self):
        if self._gripper:
            self._gripper.write(self._open_ang)

    # ---- sensors ------------------------------------------------------
    def is_obstacle_detected(self):
        if self._dist is None:
            return False
        d = self._dist.get()
        return d > 0 and d < self._dist_threshold

    def is_obstacle_touched(self):
        if self._contact is None:
            return False
        return self._contact.get()


# Global singleton (users write "robot.go_forward(50)").
robot = Robot()
