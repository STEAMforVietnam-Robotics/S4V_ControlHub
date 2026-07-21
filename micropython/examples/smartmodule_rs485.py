"""smartmodule_rs485.py -- Control Hub làm RS485 master nói chuyện Smart Module.

Port từ SmartModule_RS485.ino. Ví dụ này:
    1. Mỗi 200 ms đọc Smart ToF (ID=0x00) qua SmartToF.get() và in khoảng cách.
    2. Mỗi 200 ms đọc Smart Bumper (ID=0x00) qua SmartBumper.get().
    3. Mỗi 500 ms gửi lệnh duty tới Smart Motor Module ID 0x00.

Đổi RS485_2 nếu module cắm ở bus thứ hai: SmartToF(0x00, RS485_2)
"""

import time

from controlhub import (
    control_hub as hub, Frame,
    SmartToF, SmartBumper,
    RS485_1, MODE_DUTY,
)

# Smart ToF + Smart Bumper, ID 0x00, trên bus RS485_1.
tof = SmartToF(0x03, RS485_1)
bumper = SmartBumper(0x04, RS485_1)

# Chỉ cần RS485 cho ví dụ này (tắt BLE cho nhẹ).
hub.begin(usb=False, ble=False, rs485=True)

next_tof = 0
next_bumper = 0
next_motor = 0

# Thonny/micropico ngắt chương trình cũ bằng Ctrl-C rồi mới soft-reboot + nạp
# script này; đôi khi Ctrl-C đó tới trễ và rơi ngay vào những dòng đầu của
# script mới. Bỏ qua KeyboardInterrupt trong khoảng grace period này, chỉ
# thoát thật khi Ctrl-C đến sau khi vòng lặp đã chạy ổn định.
_START_MS = time.ticks_ms()
_STARTUP_GRACE_MS = 1000

while True:
    try:
        hub.update()
        now = time.ticks_ms()

        if time.ticks_diff(now, next_tof) >= 0:
            next_tof = time.ticks_add(now, 200)
            mm = tof.get()                      # blocking round-trip qua RS485
            if tof.status() == 0:
                print("Smart ToF:", mm, "mm")
            else:
                print("Smart ToF: timeout")

        if time.ticks_diff(now, next_bumper) >= 0:
            next_bumper = time.ticks_add(now, 200)
            pressed = bumper.get()              # blocking round-trip qua RS485
            if bumper.status() == 0:
                print("Smart Bumper:", "PRESSED" if pressed else "released")
            else:
                print("Smart Bumper: timeout")

        if time.ticks_diff(now, next_motor) >= 0:
            next_motor = time.ticks_add(now, 500)
            # Lệnh Smart Motor #0 chạy 60% chiều thuận, MODE_DUTY (duty x100 = 6000).
            cmd = Frame.make_motor_control(0x00, 6000, MODE_DUTY)
            hub.send_rs485(cmd)
    except KeyboardInterrupt:
        if time.ticks_diff(time.ticks_ms(), _START_MS) < _STARTUP_GRACE_MS:
            continue                     # Ctrl-C sót lại từ thao tác Run của IDE
        print("Exiting...")
        break
