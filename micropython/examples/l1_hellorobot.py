"""l1_hellorobot.py -- Simplest Control Hub sketch: drive in a square using the
Level-1 Robot API. No sensors, no host control.

Wiring:
    Left  motor -> M1
    Right motor -> M2

Copy the `controlhub/` package to the board, then run this file (main.py or via
`mpremote run`).
"""

from controlhub import control_hub as hub, robot, Motor, M1, M2

left = Motor(M1)
right = Motor(M2)

hub.begin()                       # USB + BLE + RS485; brings up all modules
robot.set_drive_train(left, right)
robot.invert_right(True)          # right motor mirror-mounted

while True:
    hub.update()                  # keep host protocol alive

    robot.go_forward(60); robot.wait(1.0)
    robot.turn_right(50); robot.wait(0.5)
    robot.stop();         robot.wait(0.3)
