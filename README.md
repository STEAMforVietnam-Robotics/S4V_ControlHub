# ControlHub - Arduino Library

Thư viện Arduino cho mạch **Control Hub** (ESP32-S3 + DRV8874 × 4 qua PCA9685 + 4 servo + 5 cổng đa năng + 2 bus RS485 cho Smart Module) dùng cho các dự án STEAM.

Kiến trúc lấy cảm hứng từ **Dabble**: một file include duy nhất, mỗi module (Motor, Servo, Distance, Contact, IMU, Power…) là một class riêng, và host (PC hoặc điện thoại) có thể điều khiển đồng thời qua **USB Serial + BLE** bằng cùng một khung tin nhị phân định nghĩa trong `RVRS485060726162210.pdf`.

---

## 1. Tính năng chính

| | |
|---|---|
| **API phân cấp** | Level 1 (`Robot.*`) - Level 2 (`motor.rotate()`, `ultrasonic.get()`) |
| **Điều khiển đồng thời** | USB Serial + BLE (Nordic UART) chạy song song, cùng nhận/gửi khung tin |
| **Giao tiếp Smart Module** | 2 bus RS485 half-duplex (DE/RE gộp 1 chân), khung tin đúng chuẩn PDF (CRC-8/SMBUS) |
| **Hardware trực tiếp** | DRV8874 × 4 qua PCA9685, servo qua LEDC |

---

## 2. Cấu trúc thư mục

```
ControlHub/
├── library.properties
├── keywords.txt
├── README.md
├── src/
│   ├── ControlHub.h                # Include chính, khai báo class ControlHubClass
│   ├── ControlHub.cpp
│   ├── ControlHubConfig.h          # Pin mapping và cấu hình biên dịch
│   ├── protocol/
│   │   ├── CRC8.{h,cpp}            # CRC-8/SMBUS đúng theo PDF
│   │   └── Frame.{h,cpp}           # Struct Frame + FrameParser (state machine)
│   ├── transport/
│   │   ├── Transport.h             # Interface trừu tượng
│   │   ├── UsbTransport.{h,cpp}    # Serial (USB CDC)
│   │   ├── BleTransport.{h,cpp}    # BLE Nordic UART Service
│   │   └── Rs485Transport.{h,cpp}  # Half-duplex RS485 với DE/RE
│   ├── core/
│   │   └── CommHub.{h,cpp}         # Poll transport + dispatch frame theo ModuleType
│   ├── modules/
│   │   ├── Module.h                # Base class (moduleType/id/handleFrame)
│   │   ├── Motor.{h,cpp}           # DC motor (DRV8874 + PCA9685)
│   │   ├── Servo.{h,cpp}           # LEDC 16-bit
│   │   ├── Ultrasonic.{h,cpp}      # HC-SR04 style (MF4P trig/echo)
│   │   └── Contact.{h,cpp}         # Bumper switch
│   ├── robot/
│   │   └── Robot.{h,cpp}           # Level-1 API (go_forward, turn_left, grasp…)
│   └── system/
│       └── System.{h,cpp}          # system.wait_sec / wait_millisec / reset
└── examples/
    ├── L1_HelloRobot/              # Robot chạy hình vuông
    ├── L1_ObstacleAvoid/           # Robot tránh vật cản + gắp
    ├── L2_MotorControl/            # Motor + Ultrasonic API
    ├── USB_BLE_Bridge/             # Nhận lệnh đồng thời USB + BLE
    └── SmartModule_RS485/          # Master RS485 nói chuyện với Smart Module
```

---

## 3. Kiến trúc

```
┌──────────────────────── User Sketch ─────────────────────────┐
│  Robot.go_forward(60)      motor.rotate(60)     dist.get()   │
└────────────┬─────────────────┬──────────────────┬────────────┘
             │ Level 1         │ Level 2/3        │ Sensor L2/L3
             ▼                 ▼                  ▼
┌────────── RobotClass ─┐  ┌─── Motor ───┐  ┌─ Ultrasonic ─┐
│  drive/gripper/vision │  │  DRV8874 +  │  │  HC-SR04     │
└──────┬────────────────┘  │  PCA9685    │  └──────┬───────┘
       │                   └──────┬──────┘         │
       │                          │                │
       │           ┌─── frame protocol ────────────┤
       ▼           ▼                               ▼
┌───────────────────────── CommHub ─────────────────────────────┐
│ dispatch(frame) → per-module callbacks (RS485 x2 độc lập)     │
└──┬────────────────────────┬────────────────────────┬──────────┘
   │                        │                        │
   ▼                        ▼                        ▼
UsbTransport         BleTransport (NUS)        Rs485Transport
(Serial CDC)         Nordic UART Service       DE/RE half-duplex
   ▲                        ▲                        ▲
   │                        │                        │
PC via USB           Phone/Tablet BLE          Smart Modules
                                                (ToF, IMU, Motor, …)
```

Điểm mấu chốt: cả USB, BLE, RS485 đều dùng chung một cấu trúc `Frame` và chung một `FrameParser`. `CommHub::update()` poll ba stream, phân giải khung tin và gọi handler tương ứng.

---

## 4. Định dạng khung tin

Đúng theo PDF `RVRS485060726162210.pdf`:

| Header | ModuleType | ModuleID | MsgID | RTR | DLC | Data (0..8) | CRC8 |
|--------|-----------|---------|-------|-----|-----|-------------|------|
| 0x55   | 1 B       | 1 B     | 1 B   | 1 B | 1 B | up to 8 B   | 1 B  |

- **CRC**: CRC-8/SMBUS, polynomial `0x07`, init `0x00` — bảng tra và hàm giống hệt reference trong PDF, đặt trong `src/protocol/CRC8.cpp`.
- **Module Type** đã hỗ trợ: `MOD_POWER (0x01)`, `MOD_IMU (0x02)`, `MOD_TOF (0x03)`, `MOD_BUMPER (0x04)`, `MOD_MOTOR (0x05)`, cộng thêm hai mở rộng cục bộ `MOD_SERVO (0x06)` và `MOD_SYSTEM (0x00)`.
- **Msg ID cho Motor**: `0x01` control (duty + mode), `0x02` config (đổi ID), `0x11` status.

Ví dụ khung `Request ToF sensor status`:
```
55 03 00 11 01 00 1F
```
Ví dụ khung `Motor command duty 0x0064`:
```
55 03 00 11 00 01 64 18
```
(Đây chính là hai ví dụ trong mục **II. Example** của PDF; thư viện tạo và kiểm tra đúng byte này.)

---

## 5. API chính

### Level 1 - Robot (Mission)

```cpp
Robot.go_forward(50);           // %
Robot.go_backward(50);
Robot.turn_left(50);
Robot.turn_right(50);
Robot.stop();
Robot.grasp();                  // đóng servo gripper
Robot.release();                // mở servo gripper
Robot.wait(1.5);                // giây
bool obs   = Robot.is_obstacle_detected();
bool touch = Robot.is_obstacle_touched();
```

### Level 2 - Action

```cpp
Motor      m(M1);
Ultrasonic d(P1);       // HC-SR04: trig=SIG1, echo=SIG2

m.rotate(60);                    // -100..100 %
m.stop();

uint16_t mm  = d.get();          // khoảng cách mm (0 nếu timeout)
uint8_t  err = d.status();
```

> **Lưu ý:** Mạch không có encoder nên điều khiển vòng kín (PID tốc độ / vị trí)
> không khả dụng. Motor chạy ở chế độ open-loop (`MODE_DUTY`); các frame
> `MODE_SPEED` / `MODE_POSITION` được chấp nhận nhưng chỉ `MODE_DUTY` làm động cơ quay.

### System

```cpp
SysCtrl.wait_sec(1.0);
SysCtrl.wait_millisec(250);
SysCtrl.reset();                 // ESP.restart()
```

---

## 6. Kết nối đồng thời USB + BLE

```cpp
void setup() {
    ControlHub.begin();              // USB + BLE + RS485
    // hoặc điều khiển từng cái:
    // ControlHub.begin(/*USB*/true, /*BLE*/true, /*RS485*/false, "MyRobot");
    ControlHub.onFrame([](const Frame &f, const char *src) {
        Serial.printf("frame from %s\n", src);
    });
}
void loop() { ControlHub.update(); }
```

- **USB Serial** (`Serial`) mặc định 115200 bps.
- **BLE** dùng Nordic UART Service (UUIDs cùng chuẩn với nRF Connect, Adafruit Bluefruit LE Connect, DumbLE, v.v.).
- Cả hai stream chạy song song. Frame gửi từ `sendFrame()` được đẩy ra **cả** USB lẫn BLE khi transport nào đang connected.

---

## 7. Cấu hình pin

Toàn bộ pin mapping nằm trong `ControlHubConfig.h`, mặc định khớp với `ControlHub.md`:

| Chức năng | GPIO / kênh |
|---|---|
| I²C (PCA9685) | SDA=47, SCL=21 |
| M1–M4 duty | PCA9685 ch 0-1, 2-3, 15-14, 13-12 |
| Servos SV1–SV4 | GPIO 11 / 12 / 38 / 39 |
| MF4P_1 | GPIO 16, 17 |
| MF4P_2 | GPIO 18, 8 |
| MF3P_1..3 | GPIO 3, 9, 10 |
| RS485_1 | Serial0 (UART0, TX=GPIO43/RX=GPIO44), DE/RE=GPIO2 |
| RS485_2 | Serial1 (TX=GPIO4/RX=GPIO5), DE/RE=GPIO6 |
| WS2812 status LED | GPIO40 |

Nếu schematic thay đổi, chỉ cần chỉnh `ControlHubConfig.h` — phần còn lại của thư viện không cần đụng.

---

## 8. Phụ thuộc

- Arduino core cho ESP32 (>= 2.0.x)
- [Adafruit PWM Servo Driver Library](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library) (cho PCA9685)
- BLE nằm sẵn trong ESP32 core (không cần cài thêm)

Nếu muốn build không dùng BLE (giảm flash), define `CH_NO_BLE` trước khi include:
```cpp
#define CH_NO_BLE
#include <ControlHub.h>
```

---

## 9. Roadmap

- [ ] IMU on-board (đọc trực tiếp qua I²C nếu có chip trên board)
- [ ] Power Management on-board (INA219/INA226)
- [ ] Truyền qua WiFi UDP như một transport thứ tư
- [ ] Bộ scan tự động các Smart Module trên RS485
- [ ] Tích hợp với block-based IDE (Blockly) qua các API Level-1

---

## 10. Lịch sử thay đổi

| Version | Ngày | Nội dung |
|---|---|---|
| 1.0.0 | 2026-07-07 | Bản đầu tiên: L1/L2/L3 API, giao thức khung nhị phân, USB + BLE + RS485 song song |
