"""l2_motor_control.py -- Level-2 (Action) API: talk to individual motors and
sensors directly, without the Robot abstraction.
"""

import time
from controlhub import control_hub as hub, Motor, Ultrasonic, M1, M2, P1

m0 = Motor(M1)
m1 = Motor(M2)
dist = Ultrasonic(P1)      # HC-SR04 on P1 (trig/echo)

hub.begin()

while True:
    hub.update()

    d = dist.get()
    print("Distance =", d, "mm")

    if 0 < d < 300:
        m0.stop()
        m1.stop()
    else:
        m0.rotate(60)               # percent, -100..100
        m1.rotate(-60)

    time.sleep_ms(50)
