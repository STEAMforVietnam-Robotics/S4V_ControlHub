"""gamepad_test.py -- Nhận frame gamepad qua BLE và in trạng thái nút + joystick.

Port từ Gamepad_Serial.ino. Dùng để kiểm tra webapp/ (panel Gamepad) hoặc bất kỳ
app nào gửi MOD_GAMEPAD frame vào Control Hub.

Cách dùng:
    1. Nạp `controlhub/` + chạy file này. Mở terminal USB để xem log.
    2. Kết nối tới "ControlHub" bằng webapp/ (panel Gamepad) hoặc nRF Connect.
    3. Bấm nút / kéo joystick -> trạng thái in ra USB.

Trục joystick 0..255 (128 = giữa); API quy về -100..100 với deadzone ~8%.
"""

import time

from controlhub import (
    control_hub as hub, Gamepad,
    GP_A, GP_B, GP_X, GP_Y, GP_UP, GP_DOWN, GP_LEFT, GP_RIGHT,
    GP_L1, GP_R1, GP_L2, GP_R2, GP_SELECT, GP_START,
)

gp = Gamepad(1)                 # wire id = 1 (gamepad đầu tiên)

_BTNS = (
    ("A", GP_A), ("B", GP_B), ("X", GP_X), ("Y", GP_Y),
    ("UP", GP_UP), ("DOWN", GP_DOWN), ("LEFT", GP_LEFT), ("RIGHT", GP_RIGHT),
    ("L1", GP_L1), ("R1", GP_R1), ("L2", GP_L2), ("R2", GP_R2),
    ("SELECT", GP_SELECT), ("START", GP_START),
)


def _buttons_str():
    held = [name for name, bit in _BTNS if gp.pressed(bit)]
    return "btn=[%s]" % (" ".join(held) if held else "-")


def on_gamepad(g):
    print("%s  L(%d,%d)  R(%d,%d)  mask=0x%X" % (
        _buttons_str(),
        g.left_x(), g.left_y(), g.right_x(), g.right_y(),
        g.buttons()))
    # Ví dụ bắt sự kiện nhấn (rising edge):
    if g.just_pressed(GP_A):
        print(">> A pressed")
    if g.just_pressed(GP_START):
        print(">> START pressed")


hub.begin()                     # USB + BLE + RS485; BLE quảng bá "ControlHub"
gp.on_update(on_gamepad)
print("gamepad_test: cho ket noi va gui frame gamepad...")

was_connected = False
while True:
    hub.update()                # giữ parser BLE chạy -> gọi on_gamepad khi có frame

    now = gp.connected()
    if now != was_connected:
        print("[gamepad CONNECTED]" if now else "[gamepad TIMEOUT]")
        was_connected = now

    time.sleep_ms(5)
