# ControlHub — Arduino library for ESP32-S3 Control Hub

Thư viện chính thức cho mạch **Control Hub** (ESP32-S3 + DRV8874 × 6 + PCA9685 + 4 servo + 5 cổng đa năng).

Thiết kế cho STEAM với **3 lớp API** và **một entry point duy nhất `create()`** cho mọi thiết bị plug-and-play.

---

## 1. Mô hình lập trình

```cpp
#include <ControlHub.h>

// 1. Khai báo handle ở scope global (rỗng) — sống suốt chương trình
UltrasonicHandle sonar;
NeoPixelHandle   leds;
ContactHandle    bumper;

void setup() {
    Serial.begin(115200);
    ControlHub.begin();

    // 2. create() TRONG setup(), sau begin() — gán cho handle đã khai ở trên.
    //    Lúc này Serial + phần cứng đã sẵn sàng nên lỗi (sai cổng, trùng cổng…)
    //    in thẳng ra Serial.
    sonar  = create(ULTRASONIC, PORT_MF4P_1);
    leds   = create(NEOPIXEL,   PORT_MF3P_1, 8);
    bumper = create(CONTACT,    PORT_MF3P_2);

    robot.set_primary_distance(sonar);   // L1 dùng sonar làm cảm biến vật cản
    robot.set_primary_contact(bumper);
}

void loop() {
    if (robot.is_obstacle_detected()) {
        leds.fill(255, 0, 0); leds.show();
        robot.stop();
    } else {
        leds.fill(0, 255, 0); leds.show();
        robot.go_forward();
    }
}
```

**Không có `new`, không có pointer, không có `attach()`** — chỉ `create()` và method.

> ⚠️ **Gọi `ControlHub.begin()` trong `setup()` rồi mới `create()` các thiết bị.**
> Khai `XxxHandle` ở global để `loop()` dùng được; gọi `create()` sau `begin()` để
> phần cứng (`pinMode`/RMT/LEDC/I²C) khởi tạo ngay và lỗi cổng được in ra Serial.
>
> *(Vẫn cho phép `create()` ở scope global — khi đó thiết bị chỉ được đăng ký và
> phần cứng khởi tạo bên trong `begin()` — nhưng cách trên rõ ràng và dễ debug hơn.)*

---

## 2. Ba lớp API

| Lớp | Đối tượng | Mục tiêu | Ví dụ |
|---|---|---|---|
| **L1 — EASY** | `robot.*` | Học sinh mới làm quen, verb tự nhiên | `robot.go_forward(60)` |
| **L2 — MEDIUM** | Handle method, `motor.*`, `servo.*`, `Sys.*` | Theo từng thiết bị, có tham số | `sonar.distance_cm()`, `motor.rotate(MOTOR_1, 360, 50)` |
| **L3 — HARD** | Method bậc cao của manager singleton | Tinh chỉnh PID, control mode | `motor.set_P(MOTOR_1, 0.8)` |

---

## 3. Bảng thiết bị (`create()` overloads)

| Tag | Loại thiết bị | Cổng phù hợp | Tham số thêm | Handle class |
|---|---|---|---|---|
| `ULTRASONIC` | HC-SR04 siêu âm | MF4P (cần 2 GPIO: trig + echo) | — | `UltrasonicHandle` |
| `TOF` | VL53L0X Time-of-Flight | MF4P (I²C1/Wire1: SIG1=SDA, SIG2=SCL) | `i2c_addr=0x29` | `ToFHandle` |
| `SHARP_IR` | Sharp GP2Y0A21 IR analog | MF3P (cần ADC) | — | `SharpIRHandle` |
| `CONTACT` | Nút bấm / limit switch | bất kỳ | `active_high=false` | `ContactHandle` |
| `POT` | Potentiometer | MF3P (cần ADC) | — | `PotHandle` |
| `DIGITAL_OUT` | LED đơn, relay, MOSFET | bất kỳ | `initial_state=false` | `DigitalOutHandle` |
| `NEOPIXEL` | WS2812/WS2812B strip | bất kỳ (RMT) | `num_pixels` | `NeoPixelHandle` |
| `BUZZER` | Buzzer thụ động | bất kỳ (PWM) | — | `BuzzerHandle` |

Khi tạo, nếu port không hỗ trợ loại thiết bị đó (capability mismatch) hoặc đã bị thiết bị khác chiếm, handle trả về có `.valid() == false` và mọi method gọi sau đó là no-op an toàn. Log lỗi qua `Serial`.

---

## 4. Bảng năng lực cổng

| Port | GPIO | ADC | PWM | I²C | RMT | Ghi chú |
|---|---|---|---|---|---|---|
| `MF4P_1` | ✅ ×2 | ❌ | ✅ | ✅ | ✅ | GPIO16, 17 |
| `MF4P_2` | ✅ ×2 | ❌ | ✅ | ✅ | ✅ | GPIO18, 8 (GPIO19 dành cho USB native D-) |
| `MF3P_1` | ✅ ×1 | ✅ ADC1 | ✅ | ❌ | ✅ | GPIO9 |
| `MF3P_2` | ✅ ×1 | ✅ ADC1 | ✅ | ❌ | ✅ | GPIO10 |
| `MF3P_3` | ✅ ×1 | ⚠ ADC2 | ✅ | ❌ | ✅ | GPIO11. ADC2 xung đột WiFi. |

### I²C: hai bus tách biệt

| Bus | Dùng cho | Chân |
|---|---|---|
| **I2C0** (`Wire`) | PCA9685 (motor/servo) — bus nội bộ | GPIO13 = SDA, GPIO12 = SCL |
| **I2C1** (`Wire1`) | Cảm biến I²C cắm vào cổng MF4P (vd VL53L0X) | trên cổng host: **SIG1 = SDA, SIG2 = SCL** |

- `Wire1` chỉ dựng được trên **một** cổng MF4P (`MF4P_1` *hoặc* `MF4P_2`) — thiết bị I²C đầu tiên `create()` sẽ "bind" cổng đó; thiết bị I²C tiếp theo phải nằm trên **cùng cổng** và **khác địa chỉ**.
- Thiết bị I²C **không chiếm độc quyền** cổng: nhiều con có thể chia sẻ bus (giới hạn `CH_MAX_I2C_DEVICES`). Cổng đang host bus sẽ từ chối thiết bị thường (Ultrasonic…).
- Hai VL53L0X cùng địa chỉ 0x29 → cần XSHUT để re-address (thêm GPIO).

---

## 5. Cài đặt

### PlatformIO
```ini
lib_deps =
    Adafruit PWM Servo Driver Library
    ESP32Servo
    Adafruit VL53L0X
    Adafruit NeoPixel
    https://github.com/your-org/ControlHub.git
```

### Arduino IDE
1. Tải repo dưới dạng ZIP → *Sketch → Include Library → Add .ZIP Library…*
2. Cài 4 dependencies qua *Library Manager*: **Adafruit PWM Servo Driver Library**, **ESP32Servo**, **Adafruit VL53L0X**, **Adafruit NeoPixel**.

### Cấu hình board & nạp code (ESP32-S3, USB native)

Mạch dùng **USB native của ESP32-S3** (GPIO19 = D-, GPIO20 = D+) để nạp code và debug.
Trong **Tools** chọn:

| Mục | Giá trị |
|---|---|
| Board | ESP32S3 Dev Module (hoặc board tương ứng) |
| USB CDC On Boot | **Enabled** — bắt buộc, nếu không `Serial` sẽ trỏ về UART0 và không thấy log qua USB |
| USB Mode | **Hardware CDC and JTAG** |
| Upload Mode | UART0 / Hardware CDC |

> 💡 Nếu nạp xong board không tự chạy: giữ **BOOT** + chạm **RESET** để vào download mode khi
> nạp, sau khi nạp xong bấm **RESET** để vào chương trình.
>
> Vì USB native chiếm GPIO19/20, cổng `MF4P_2` dùng **GPIO18 + GPIO8** (không phải GPIO19).

---

## 6. Tham chiếu API

### 6.1. Robot (L1)

| API | Mô tả |
|---|---|
| `robot.go_forward(speed=60)` | Đi thẳng, 0..100% |
| `robot.go_backward(speed=60)` | Lùi |
| `robot.turn_left(speed=60)` | Quay trái tại chỗ |
| `robot.turn_right(speed=60)` | Quay phải |
| `robot.stop()` | Dừng 2 bánh |
| `robot.wait(seconds)` | Chờ giây (float) |
| `robot.grasp()` / `robot.release()` | Đóng / mở gripper |
| `robot.is_obstacle_detected()` | `true` nếu primary distance < ngưỡng |
| `robot.is_obstacle_touched()` | `true` nếu primary contact bị nhấn |
| `robot.set_primary_distance(handle)` | Nhận bất kỳ `*Handle` distance |
| `robot.set_primary_contact(handle)` | Nhận `ContactHandle` |
| `robot.set_drivetrain(left, right)` | Đổi cấu hình 2 bánh |
| `robot.set_default_speed(pct)` | Tốc độ mặc định |
| `robot.set_obstacle_threshold_cm(cm)` | Ngưỡng phát hiện |

### 6.2. Handle methods (L2)

| Handle | Methods |
|---|---|
| `UltrasonicHandle`, `SharpIRHandle` | `.distance_cm()` |
| `ToFHandle` | `.distance_cm()`, `.distance_mm()` |
| `ContactHandle` | `.pressed()`, `.value()` |
| `PotHandle` | `.raw()`, `.volts()`, `.percent()`, `.scaled(min, max)` |
| `DigitalOutHandle` | `.on()`, `.off()`, `.toggle()`, `.write(bool)`, `.value()` |
| `NeoPixelHandle` | `.count()`, `.set_color(i, r, g, b)`, `.fill(r, g, b)`, `.clear()`, `.show()`, `.set_brightness(0..255)` |
| `BuzzerHandle` | `.tone(freq_hz, duration_ms=0)`, `.no_tone()` |

Tất cả handle có `.valid()` để kiểm tra khởi tạo thành công.

### 6.3. `motor.*` (L2 + L3)

| API | Mô tả |
|---|---|
| `motor.rotate(id, deg, speed=50)` | Quay chính xác N độ (cần encoder, blocking) |
| `motor.stop(id, STOP_COAST\|STOP_BRAKE)` | Dừng 1 motor |
| `motor.stopAll()` | Dừng tất cả |
| `motor.position(id)` | Vị trí hiện tại (deg) |
| `motor.drive_raw(id, pct)` | Open-loop `-100..+100%` |
| `motor.set_P/I/D(id, val)` | Đặt PID gain |
| `motor.set_controlmode(id, MODE_*)` | OPEN_LOOP / SPEED / POSITION |
| `motor.set_speed(id, rpm)` | Setpoint tốc độ |
| `motor.get_speed(id)` | RPM đo được |
| `motor.set_position(id, deg)` | Setpoint vị trí |
| `motor.reset_encoder(id)` | Reset về 0 |
| `motor.has_encoder(id)` | M1/M2 → true |

### 6.4. `servo.*` (L2)

| API | Mô tả |
|---|---|
| `servo.write(id, deg)` | Tới góc 0..180° (tức thời) |
| `servo.write(id, deg, speed_dps)` | Có giới hạn tốc độ (blocking) |
| `servo.read(id)` | Đọc lại góc cuối |

### 6.5. `Sys.*`

| API | Mô tả |
|---|---|
| `Sys.wait(sec)` / `Sys.wait_sec(sec)` | Chờ giây (float) |
| `Sys.wait_millisec(ms)` | Chờ mili-giây |
| `Sys.stop()` | Dừng tất cả động cơ |
| `Sys.uptime_sec()` / `Sys.uptime_ms()` | Thời gian từ boot |

---

## 7. Hằng số ID

```cpp
MOTOR_1 .. MOTOR_6
SERVO_1 .. SERVO_4
PORT_MF4P_1, PORT_MF4P_2, PORT_MF3P_1, PORT_MF3P_2, PORT_MF3P_3
ULTRASONIC, TOF, SHARP_IR, CONTACT, POT, DIGITAL_OUT, NEOPIXEL, BUZZER
MODE_OPEN_LOOP, MODE_SPEED, MODE_POSITION
STOP_COAST, STOP_BRAKE
```

---

## 8. Cấu trúc thư mục

```
ControlHub/
├── library.properties
├── keywords.txt
├── README.md
├── src/
│   ├── ControlHub.h / .cpp        # entry point, init
│   ├── ch_config.h                # pin mapping (sửa khi đổi PCB)
│   ├── ch_types.h                 # enums, tags, handle bases
│   ├── PortManager.h / .cpp       # ownership + capability matrix
│   ├── DeviceRegistry.h / .cpp    # slot table cho handle lookup
│   ├── hal/
│   │   ├── DRV8874.{h,cpp}        # H-bridge driver
│   │   └── Encoder.{h,cpp}        # quadrature ISR
│   ├── iface/
│   │   ├── IDistanceSensor.{h,cpp}# + DistanceHandle
│   │   ├── IDigitalIO.h
│   │   ├── IAnalogSensor.h
│   │   └── ILightOutput.h
│   ├── devices/
│   │   ├── Motor.{h,cpp}          # core: 6 DC motors with PID
│   │   ├── Servo.{h,cpp}          # core: 4 RC servos
│   │   ├── Ultrasonic.{h,cpp}     # HC-SR04
│   │   ├── ToF.{h,cpp}            # VL53L0X
│   │   ├── SharpIR.{h,cpp}        # GP2Y0A21
│   │   ├── Contact.{h,cpp}        # button / switch
│   │   ├── Pot.{h,cpp}            # potentiometer
│   │   ├── DigitalOut.{h,cpp}     # LED / relay
│   │   ├── NeoPixel.{h,cpp}       # WS2812
│   │   └── Buzzer.{h,cpp}         # passive buzzer
│   └── api/
│       ├── SysAPI.{h,cpp}         # Sys.*
│       ├── RobotAPI.{h,cpp}       # robot.*
│       └── Create.{h,cpp}         # create() overloads
└── examples/
    ├── L1_BasicDrive/
    ├── L1_StatusLED_ToF/          # Phase-3 showcase
    ├── L2_MotorRotate/
    └── L3_PIDTuning/
```

PID chạy trên task FreeRTOS pinned core 1 ở `CH_PID_TASK_HZ` (mặc định 100 Hz).

---

## 9. Tinh chỉnh phần cứng

Mọi pin mapping, capability matrix, gripper, CPR encoder nằm trong **`src/ch_config.h`**.

---

## 10. Mở rộng — viết driver riêng

Pattern tối thiểu:

```cpp
class MyDevice : public IDistanceSensor {
public:
    MyDevice(SensorPort port) : _port(port), _ok(false) {
        if (!PortManager::claim(port, DEV_NONE /*hoặc enum mới*/, CAP_GPIO_DIGITAL))
            return;
        _pin = PortManager::pin(port, 0);
        pinMode(_pin, INPUT);
        _ok = true;
    }
    ~MyDevice() { if (_ok) PortManager::release(_port); }
    float distance_cm() override { return _ok ? readSomething() : -1.0f; }
private:
    SensorPort _port;
    uint8_t    _pin;
    bool       _ok;
};
```

Có thể trả về như `IDistanceSensor*` rồi `robot.set_primary_distance(...)` qua một `DistanceHandle` tự build, hoặc thêm tag mới + overload `create()`.

---

## 11. Roadmap

- [ ] BLE/WiFi command channel (Blockly)
- [ ] Mecanum / Omni drivetrain preset
- [ ] OneWire / DHT / GPS drivers
- [ ] MicroPython binding
