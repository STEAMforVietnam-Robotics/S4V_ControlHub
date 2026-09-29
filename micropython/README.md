# ControlHub - Thư viện MicroPython (ESP32-S3)

Bản port MicroPython của thư viện Arduino **ControlHub** cho mạch Control Hub
(ESP32-S3 + DRV8874 ×4 qua PCA9685 + 4 servo + 5 cổng đa năng + WS2812 + 2 bus
RS485). Giữ **nguyên giao thức khung nhị phân** của bản C++ nên tương thích với
webapp trong `../webapp/` và các Smart Module RS485.

Mô hình lập trình **đồng bộ (blocking)** giống Arduino: `hub.begin()` một lần rồi
gọi `hub.update()` trong vòng lặp `while True`.

---

## 1. Yêu cầu

- Firmware MicroPython cho **ESP32-S3** có sẵn module `machine`, `neopixel`,
  và `bluetooth` (nếu dùng BLE). Bản MicroPython chính thức cho ESP32-S3 đã có đủ.
- Board bật **USB-CDC** (mặc định trên ESP32-S3).

## 2. Nạp thư viện lên board

Copy cả thư mục `controlhub/` vào thư mục gốc filesystem của board. Ví dụ với
[`mpremote`](https://docs.micropython.org/en/latest/reference/mpremote.html):

```bash
mpremote connect COM5 fs cp -r controlhub :
mpremote connect COM5 run examples/l2_motor_control.py
```

Hoặc dùng Thonny: copy thư mục `controlhub/` vào "MicroPython device", mở một file
trong `examples/` rồi bấm Run. Để chạy tự động khi cấp nguồn, đổi tên ví dụ thành
`main.py` và copy lên board.

## 3. Ví dụ nhanh

```python
from controlhub import control_hub as hub, robot, Motor, Ultrasonic, \
    M1, M2, P1

left, right = Motor(M1), Motor(M2)
dist = Ultrasonic(P1)

hub.begin()                       # USB + BLE + RS485, tự khởi động mọi module
robot.set_drive_train(left, right)
robot.set_distance(dist, 250)

while True:
    hub.update()
    if robot.is_obstacle_detected():
        robot.stop()
    else:
        robot.go_forward(60)
```

Các ví dụ khác trong `examples/`:

| File | Nội dung |
|---|---|
| `l1_hellorobot.py` | Chạy hình vuông (Level-1 Robot API) |
| `l1_obstacle_avoid.py` | Tránh vật cản + gắp vật |
| `l2_motor_control.py` | Điều khiển motor + đọc siêu âm trực tiếp |
| `usb_ble_bridge.py` | Nhận khung đồng thời qua USB + BLE |

## 4. Ánh xạ API C++ → Python

| Arduino / C++ | MicroPython |
|---|---|
| `ControlHub.begin()` | `hub.begin()` |
| `ControlHub.begin(usb,ble,rs485,name)` | `hub.begin(usb, ble, rs485, ble_name)` |
| `ControlHub.update()` | `hub.update()` |
| `ControlHub.sendFrame(f)` | `hub.send_frame(f)` |
| `ControlHub.statusOk()` | `hub.status_ok()` |
| `Robot.go_forward(60)` | `robot.go_forward(60)` |
| `Robot.turn_left(50)` | `robot.turn_left(50)` |
| `Robot.grasp()/release()` | `robot.grasp()/robot.release()` |
| `Robot.is_obstacle_detected()` | `robot.is_obstacle_detected()` |
| `Robot.setDriveTrain(&l,&r)` | `robot.set_drive_train(l, r)` |
| `Motor m(M1); m.rotate(60)` | `m = Motor(M1); m.rotate(60)` |
| `Servo s(S1); s.write(90)` | `s = Servo(S1); s.write(90)` |
| `Ultrasonic d(P1); d.get()` | `d = Ultrasonic(P1); d.get()` |
| `Contact b(P5); b.get()` | `b = Contact(P5); b.get()` |
| `SysCtrl.wait_sec(1.0)` | `sys_ctrl.wait_sec(1.0)` |
| `Frame::makeMotorControl(...)` | `Frame.make_motor_control(...)` |

Các module tự đăng ký với hub khi được tạo, và `hub.begin()` sẽ gọi `begin()` cho
tất cả module đã tạo — bạn **không cần** gọi `motor.begin()` thủ công (nhưng vẫn
gọi được nếu tạo module sau `hub.begin()`).

## 5. Định dạng khung tin

Giống hệt bản C++ / PDF: `Header(0x55) | ModuleType | ModuleID | MsgID | RTR |
DLC | Data(0..8) | CRC8`, với CRC-8/SMBUS (poly `0x07`, init `0x00`). Xem
`controlhub/frame.py` và `controlhub/crc8.py`.

## 6. Cấu hình pin

Toàn bộ pin map + hằng số nằm trong `controlhub/config.py` (port từ
`../src/ControlHubConfig.h`). Chỉnh ở đây nếu schematic thay đổi.

| Chức năng | GPIO |
|---|---|
| I²C (PCA9685) | SDA=47, SCL=21 |
| Servo SV1–SV4 | 11 / 12 / 38 / 39 |
| MF4P_1 | 16, 17 |
| MF4P_2 | 18, 8 |
| MF3P_1..3 | 3, 9, 10 |
| RS485_1 | UART1 → TX=43/RX=44, DE/RE=2 (route ra chân, tránh UART0/REPL) |
| RS485_2 | UART2 → TX=4/RX=5, DE/RE=6 |
| Host (USB CDC) | USB gốc — kênh có dây duy nhất tới host, chung với REPL |
| WS2812 status | 40 (12 LED) |

## 7. Lưu ý về USB-CDC (REPL)

Trên ESP32-S3, kênh USB-CDC dùng chung với REPL. Khi bật USB transport
(`hub.begin(usb=True)`), thư viện **tắt Ctrl-C** (`micropython.kbd_intr(-1)`) để
đọc dữ liệu nhị phân không bị gián đoạn. Muốn quay lại REPL, **nhấn nút reset/EN**
trên board. Nếu chỉ dùng BLE/RS485, gọi `hub.begin(usb=False)` để giữ REPL.

Nếu firmware không có BLE, lớp BLE tự vô hiệu hoá nhẹ nhàng (giống cờ `CH_NO_BLE`
bản C++) và phần còn lại vẫn chạy.

## 8. Kiểm thử off-board

Lớp giao thức thuần Python chạy được trên PC (CPython):

```bash
python tests/test_frame.py
```

Xác nhận CRC + serialize khớp 2 ví dụ trong PDF (`...1F`, `...18`) và parser
round-trip đúng.
