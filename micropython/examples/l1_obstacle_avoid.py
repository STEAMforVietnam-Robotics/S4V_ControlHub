"""l1_obstacle_avoid.py -- Drive-around-obstacle behaviour, Level-1 Robot API.

Wiring:
    Left  motor  -> M1
    Right motor  -> M2
    HC-SR04      -> MF4P_1 (trig = SIG1, echo = SIG2)
    Bumper       -> MF3P_2
    Gripper      -> SV1
"""

from controlhub import (
    control_hub as hub, robot,
    Motor, Servo, Ultrasonic, Contact,
    M1, M2, S1, P1, P4,
)

left = Motor(M1)
right = Motor(M2)
gripper = Servo(S1)
dist = Ultrasonic(P1)
bumper = Contact(P4)

hub.begin()

robot.set_drive_train(left, right)
robot.invert_right(True)
robot.set_gripper(gripper, open_angle=20, closed_angle=160)
robot.set_distance(dist, threshold_mm=250)
robot.set_contact(bumper)

robot.release()                   # start with gripper open

while True:
    hub.update()

    if robot.is_obstacle_touched():
        robot.go_backward(60); robot.wait(0.5)
        robot.turn_left(60);   robot.wait(0.6)
        continue

    if robot.is_obstacle_detected():
        robot.stop();          robot.wait(0.2)
        robot.grasp();         robot.wait(0.4)
        robot.go_backward(50); robot.wait(0.8)
        robot.release();       robot.wait(0.3)
        robot.turn_right(50);  robot.wait(0.5)
        continue

    robot.go_forward(70)
