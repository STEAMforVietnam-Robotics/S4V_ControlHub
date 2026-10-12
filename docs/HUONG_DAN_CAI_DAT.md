# 📘 Hướng dẫn cài đặt thư viện ControlHub

> Dành cho học sinh mới bắt đầu lập trình robot với mạch **Control Hub** (ESP32-S3).
> Làm lần lượt từng bước, **đừng bỏ bước nào nhé!** ⏱️ Thời gian: khoảng 20–30 phút.

---

## 🗺️ Tổng quan: chúng ta sẽ làm gì?

```
Bước 1  Cài phần mềm Arduino IDE
   ↓
Bước 2  Cài "gói ESP32" để Arduino IDE hiểu mạch Control Hub
   ↓
Bước 3  Cài thư viện ControlHub
   ↓
Bước 4  Cài các thư viện phụ (Adafruit, ...)
   ↓
Bước 5  Chọn đúng loại mạch và cổng kết nối
   ↓
Bước 6  Nạp chương trình đầu tiên 🎉
```

### Bạn cần chuẩn bị

| Thứ cần có | Ghi chú |
|---|---|
| 💻 Máy tính | Windows, macOS hoặc Linux, **có kết nối Internet** |
| 🔌 Cáp USB-C | Phải là cáp **có truyền dữ liệu**, không phải cáp chỉ để sạc |
| 🤖 Mạch Control Hub | Kèm pin hoặc nguồn nếu muốn chạy động cơ |

> 💡 **Mẹo:** Nhiều cáp USB rẻ tiền chỉ sạc được mà không truyền dữ liệu. Nếu máy tính không nhận mạch, hãy thử đổi cáp khác trước tiên!

---

## Bước 1 — Cài Arduino IDE

**Arduino IDE** là phần mềm để viết code và nạp code vào mạch.

1. Mở trình duyệt, vào trang: **https://www.arduino.cc/en/software**
2. Tải bản **Arduino IDE 2.x** phù hợp với máy của bạn:
   - Windows → chọn **Windows Win 10 and newer, 64 bits**
   - macOS → chọn bản **Apple Silicon** (máy M1/M2/M3...) hoặc **Intel**
   - Linux → chọn **AppImage**
3. Có thể trang web hỏi bạn quyên góp — bấm **Just Download** để tải miễn phí.
4. Mở file vừa tải về và cài đặt như phần mềm bình thường (cứ bấm **Next / I Agree / Install**).
5. Khi Windows hỏi có cài **driver** (trình điều khiển) không → bấm **Install / Cài đặt**.

✅ **Kiểm tra:** Mở Arduino IDE lên, bạn thấy một cửa sổ có sẵn đoạn code `void setup()` và `void loop()` là thành công.

---

## Bước 2 — Cài gói hỗ trợ mạch ESP32

Mặc định Arduino IDE chưa biết mạch ESP32-S3 là gì. Ta cần "dạy" cho nó.

### 2.1. Thêm địa chỉ tải gói ESP32

1. Vào menu **File → Preferences...** (trên macOS: **Arduino IDE → Settings...**)
2. Tìm ô **Additional boards manager URLs** (ở gần cuối cửa sổ).
3. Dán dòng sau vào ô đó:

   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

4. Bấm **OK**.

> 💡 Nếu ô đó đã có sẵn địa chỉ khác, bấm vào biểu tượng 🗗 bên phải ô để mở danh sách, rồi thêm địa chỉ trên vào **một dòng mới**.

### 2.2. Cài gói esp32

1. Bấm vào biểu tượng **Boards Manager** ở thanh bên trái (hình bảng mạch 🔲), hoặc vào **Tools → Board → Boards Manager...**
2. Gõ vào ô tìm kiếm: `esp32`
3. Tìm mục **esp32 by Espressif Systems** (chú ý đúng chữ *Espressif Systems*).
4. Bấm **INSTALL** và chờ. Gói này khá nặng (vài trăm MB), có thể mất **5–15 phút** tùy mạng.

✅ **Kiểm tra:** Mục *esp32 by Espressif Systems* hiện chữ **INSTALLED** là xong.

> ⚠️ Đừng tắt Arduino IDE khi đang cài! Nếu bị lỗi giữa chừng, bấm **REMOVE** rồi cài lại.

---

## Bước 3 — Cài thư viện ControlHub

Chọn **MỘT** trong hai cách dưới đây. Cách A dễ hơn, phù hợp với hầu hết các bạn.

### 🅰️ Cách A: Cài từ file ZIP (khuyên dùng)

1. Vào trang GitHub của thư viện:
   **https://github.com/STEAMforVietnam-Robotics/S4V_ControlHub**
2. Bấm nút màu xanh **`<> Code`** → chọn **Download ZIP**.
3. Một file tên kiểu `S4V_ControlHub-main.zip` sẽ được tải về. **Không cần giải nén!**
4. Quay lại Arduino IDE, vào menu:
   **Sketch → Include Library → Add .ZIP Library...**
5. Chọn file `.zip` vừa tải về → bấm **Open**.
6. Góc dưới cửa sổ hiện thông báo **"Library installed"** là xong.

> 💡 Nếu thầy cô gửi cho bạn file ZIP qua Zalo, Google Drive hay USB, cũng làm từ bước 4 như trên.

### 🅱️ Cách B: Dùng Git (cho bạn đã biết dùng Git)

Mở Terminal (hoặc Git Bash trên Windows) và chạy:

```bash
# Windows (Git Bash)
cd ~/Documents/Arduino/libraries

# macOS
cd ~/Documents/Arduino/libraries

# Linux
cd ~/Arduino/libraries

git clone https://github.com/STEAMforVietnam-Robotics/S4V_ControlHub.git
```

Sau đó **tắt hẳn Arduino IDE và mở lại**.

Cách này giúp cập nhật thư viện rất nhanh về sau, chỉ cần:

```bash
cd S4V_ControlHub
git pull
```

### 📁 Thư viện được cài vào đâu?

| Hệ điều hành | Thư mục |
|---|---|
| Windows | `C:\Users\<tên bạn>\Documents\Arduino\libraries\` |
| macOS | `/Users/<tên bạn>/Documents/Arduino/libraries/` |
| Linux | `/home/<tên bạn>/Arduino/libraries/` |

Bên trong thư mục `libraries` phải có thư mục thư viện, và **ngay bên trong** nó phải thấy file `library.properties` cùng thư mục `src`:

```
libraries/
└── S4V_ControlHub/          ← thư mục thư viện
    ├── library.properties   ← phải nằm ở đây!
    ├── src/
    └── examples/
```

> ⚠️ **Lỗi hay gặp:** giải nén ZIP bằng tay tạo ra 2 lớp thư mục lồng nhau
> (`S4V_ControlHub-main/S4V_ControlHub-main/...`). Khi đó Arduino IDE **không nhận** thư viện.
> Hãy đảm bảo `library.properties` nằm đúng như hình trên.

---

## Bước 4 — Cài các thư viện phụ thuộc

### 4.1. Thư viện bắt buộc (ai cũng phải cài)

ControlHub cần thêm các thư viện sau của Adafruit để chạy:

| Thư viện | Tác giả | Dùng để làm gì |
|---|---|---|
| **Adafruit PWM Servo Driver Library** | Adafruit | Điều khiển chip PCA9685 → điều khiển động cơ |
| **Adafruit BusIO** | Adafruit | Thư viện "hỗ trợ" mà thư viện PWM Servo Driver cần (cài tự động ở bước dưới) |
| **Adafruit NeoPixel** | Adafruit | Điều khiển đèn LED màu (WS2812) |

**Cách cài:**

1. Bấm biểu tượng **Library Manager** ở thanh bên trái (hình những cuốn sách 📚), hoặc vào **Tools → Manage Libraries...**
2. Gõ: `Adafruit PWM Servo Driver`
3. Tìm đúng mục **Adafruit PWM Servo Driver Library** by *Adafruit* → bấm **INSTALL**.
4. Một hộp thoại hiện ra hỏi có cài thêm **Adafruit BusIO** không → bấm **INSTALL ALL**.
   > ⚠️ Nếu lỡ bấm *Install without dependencies*, hãy gõ `Adafruit BusIO` và cài thủ công.
5. Gõ tiếp: `Adafruit NeoPixel`
6. Tìm mục **Adafruit NeoPixel** by *Adafruit* → bấm **INSTALL**.

✅ **Kiểm tra:** Cả 3 thư viện trong bảng đều hiện chữ **INSTALLED**.

> 💡 Các thư viện **BLE (Bluetooth)**, **WiFi**, **Wire (I²C)**, **WebServer**, **DNSServer**, **Preferences** đã có sẵn trong gói ESP32 (Bước 2), bạn **không cần** cài thêm.

### 4.2. Thư viện tùy chọn (chỉ cần cho ví dụ `gripperbot_client`)

Nếu bạn muốn chạy ví dụ **gripperbot_client** (robot kết nối WiFi với mô phỏng trên máy tính), cài thêm:

| Thư viện | Gõ vào ô tìm kiếm | Chọn đúng mục | Phiên bản |
|---|---|---|---|
| **ArduinoJson** | `ArduinoJson` | ArduinoJson by *Benoit Blanchon* | 6.x trở lên |
| **WebSockets** | `WebSockets` | WebSockets by *Markus Sattler* | 2.4 trở lên |

> ⚠️ Khi gõ `WebSockets` sẽ hiện ra rất nhiều thư viện tên giống nhau. Hãy chọn đúng tác giả **Markus Sattler**!

Các ví dụ khác **không cần** 2 thư viện này.

---

## Bước 5 — Chọn mạch và cổng kết nối

### 5.1. Cắm mạch vào máy tính

Dùng cáp USB-C nối mạch Control Hub với máy tính.

### 5.2. Chọn loại mạch (Board)

Vào **Tools → Board → esp32 →** chọn **ESP32S3 Dev Module**.

> 💡 Danh sách rất dài! Bạn có thể bấm vào ô chọn mạch ở thanh trên cùng, chọn **Select other board and port...** rồi gõ `ESP32S3 Dev` để tìm cho nhanh.

### 5.3. Chỉnh các thiết lập trong menu Tools

Sau khi chọn mạch, menu **Tools** sẽ có thêm nhiều mục. Chỉnh như sau:

| Mục trong menu Tools | Chọn giá trị | Vì sao? |
|---|---|---|
| **USB CDC On Boot** | **Enabled** | ⭐ Bắt buộc! Để xem được `Serial` qua cổng USB |
| **Upload Speed** | 921600 | Nạp code nhanh |
| **Partition Scheme** | **Huge APP (3MB No OTA/1MB SPIFFS)** | Code có Bluetooth khá nặng, cần nhiều chỗ |
| Các mục còn lại | Để mặc định | |

> ⚠️ Nếu quên bật **USB CDC On Boot**, code vẫn nạp được nhưng **Serial Monitor sẽ không hiện gì cả**.

### 5.4. Chọn cổng (Port)

Vào **Tools → Port** và chọn cổng của mạch:

| Hệ điều hành | Tên cổng thường gặp |
|---|---|
| Windows | `COM3`, `COM4`, `COM5`... |
| macOS | `/dev/cu.usbmodem...` |
| Linux | `/dev/ttyACM0` |

> 💡 **Không biết cổng nào?** Rút cáp ra → xem danh sách → cắm lại → cổng nào **mới xuất hiện** chính là mạch của bạn.

---

## Bước 6 — Nạp chương trình đầu tiên 🎉

### 6.1. Mở ví dụ có sẵn

Vào **File → Examples →** kéo xuống phần **Examples from Custom Libraries → ControlHub → L1_HelloRobot**.

> Nếu không thấy mục *ControlHub* trong Examples → thư viện chưa cài đúng, xem lại **Bước 3**.

Chương trình này làm robot chạy theo hình vuông:

```cpp
#include <ControlHub.h>

Motor leftMotor (M1);    // động cơ trái cắm cổng M1
Motor rightMotor(M2);    // động cơ phải cắm cổng M2

void setup() {
    Serial.begin(115200);

    ControlHub.begin();               // khởi động mạch
    leftMotor.begin();
    rightMotor.begin();

    Robot.setDriveTrain(&leftMotor, &rightMotor);
    Robot.invertRight(true);          // động cơ phải lắp ngược chiều
}

void loop() {
    ControlHub.update();              // luôn gọi dòng này trong loop()

    Robot.go_forward(60);   Robot.wait(1.0);   // đi thẳng 1 giây
    Robot.turn_right(50);   Robot.wait(0.5);   // rẽ phải 0.5 giây
    Robot.stop();           Robot.wait(0.3);   // dừng
}
```

### 6.2. Kiểm tra code (Verify)

Bấm nút **✓ Verify** (góc trên bên trái).
Lần đầu sẽ hơi lâu (1–3 phút) vì máy phải biên dịch nhiều thứ. Kết quả thành công sẽ hiện dòng kiểu:

```
Sketch uses 1234567 bytes (39%) of program storage space...
```

### 6.3. Nạp code (Upload)

Bấm nút **→ Upload** (bên cạnh nút Verify). Chờ đến khi hiện:

```
Hard resetting via RTS pin...
```

🎉 **Chúc mừng! Bạn đã nạp thành công chương trình đầu tiên!**

> ⚠️ **An toàn:** Khi thử robot chạy, hãy **nhấc robot lên** hoặc đặt trên sàn rộng, tránh xa mép bàn để robot không bị rơi.

### 6.4. Các ví dụ khác để thử

| Ví dụ | Nội dung | Mức độ |
|---|---|---|
| `L1_HelloRobot` | Robot chạy hình vuông | ⭐ Dễ |
| `L1_ObstacleAvoid` | Robot tự tránh vật cản + gắp đồ | ⭐ Dễ |
| `L2_MotorControl` | Điều khiển từng động cơ, đọc cảm biến siêu âm | ⭐⭐ Trung bình |
| `Gamepad_Serial` | Điều khiển robot bằng tay cầm | ⭐⭐ Trung bình |
| `USB_BLE_Bridge` | Nhận lệnh từ máy tính và điện thoại cùng lúc | ⭐⭐⭐ Nâng cao |
| `SmartModule_RS485` | Giao tiếp với các Smart Module | ⭐⭐⭐ Nâng cao |

---

## 🛠️ Gặp lỗi? Xem ở đây!

### ❌ `ControlHub.h: No such file or directory`

**Nguyên nhân:** Thư viện ControlHub chưa được cài hoặc cài sai chỗ.

**Cách sửa:**
- Làm lại **Bước 3**.
- Kiểm tra file `library.properties` có nằm đúng trong `libraries/S4V_ControlHub/` không (xem mục *Thư viện được cài vào đâu?*).
- Tắt hẳn Arduino IDE rồi mở lại.

### ❌ `Adafruit_PWMServoDriver.h: No such file or directory`
### ❌ `Adafruit_NeoPixel.h: No such file or directory`

**Nguyên nhân:** Chưa cài thư viện phụ thuộc.
**Cách sửa:** Làm lại **Bước 4.1**.

### ❌ `Adafruit_I2CDevice.h: No such file or directory`

**Nguyên nhân:** Thiếu thư viện **Adafruit BusIO**.
**Cách sửa:** Vào Library Manager, gõ `Adafruit BusIO` và cài đặt.

### ❌ `ArduinoJson.h` hoặc `WebSocketsClient.h: No such file or directory`

**Nguyên nhân:** Bạn đang chạy ví dụ `gripperbot_client` mà chưa cài thư viện tùy chọn.
**Cách sửa:** Làm theo **Bước 4.2**.

### ❌ `BLEDevice.h: No such file or directory` hoặc lỗi lạ liên quan đến board

**Nguyên nhân:** Chọn sai loại mạch (ví dụ đang chọn *Arduino Uno*).
**Cách sửa:** Vào **Tools → Board** và chọn lại **ESP32S3 Dev Module** (Bước 5.2).

### ❌ `Sketch too big` / `text section exceeds available space`

**Nguyên nhân:** Code quá lớn so với vùng nhớ mặc định.
**Cách sửa:** Vào **Tools → Partition Scheme** → chọn **Huge APP (3MB No OTA/1MB SPIFFS)**.

### ❌ Menu Tools → Port bị mờ, không có cổng nào

**Cách sửa (thử lần lượt):**
1. Đổi sang **cáp USB khác** (cáp có truyền dữ liệu).
2. Cắm sang **cổng USB khác** trên máy tính (tránh cắm qua hub USB).
3. Đưa mạch vào **chế độ nạp thủ công** (xem mục ngay dưới).
4. **Linux:** chạy lệnh sau rồi **đăng xuất và đăng nhập lại**:
   ```bash
   sudo usermod -a -G dialout $USER
   ```

### ❌ `Failed to connect to ESP32-S3: No serial data received`

**Nguyên nhân:** Mạch không tự vào chế độ nạp code được.

**Cách sửa — vào chế độ nạp thủ công:**
1. **Nhấn và giữ** nút **BOOT** trên mạch.
2. Trong khi vẫn giữ BOOT, **nhấn rồi thả** nút **RESET** (hoặc **EN / RST**).
3. **Thả** nút BOOT.
4. Chọn lại cổng trong **Tools → Port** (tên cổng có thể đã thay đổi).
5. Bấm **Upload** lại.
6. Nạp xong, **nhấn nút RESET một lần** để chương trình bắt đầu chạy.

### ❌ Serial Monitor không hiện chữ nào

**Cách sửa:**
- Kiểm tra **Tools → USB CDC On Boot** đã chọn **Enabled** chưa → nạp lại code.
- Ở góc phải Serial Monitor, chọn tốc độ **115200 baud** (phải giống số trong `Serial.begin(115200)`).
- Nhấn nút **RESET** trên mạch.

### ❌ Nạp code thành công nhưng động cơ không quay

**Cách sửa:**
- Kiểm tra mạch đã **cắm pin / nguồn** chưa. Cổng USB **không đủ điện** để chạy động cơ!
- Kiểm tra động cơ cắm đúng cổng **M1** và **M2** như trong code.
- Nếu robot chạy lùi thay vì tiến, hoặc xoay vòng tại chỗ → đổi `Robot.invertRight(true)` thành `false` (hoặc ngược lại).

---

## ❓ Câu hỏi thường gặp

**Hỏi: Làm sao biết mình đang dùng phiên bản thư viện nào?**
Đáp: Mở file `library.properties` trong thư mục thư viện, xem dòng `version=...`.

**Hỏi: Làm sao cập nhật thư viện lên bản mới?**
Đáp:
- Nếu cài bằng **Cách A (ZIP)**: xóa thư mục `S4V_ControlHub` cũ trong `libraries`, rồi cài lại file ZIP mới.
- Nếu cài bằng **Cách B (Git)**: vào thư mục thư viện và chạy `git pull`.

Sau khi cập nhật, nhớ **tắt và mở lại Arduino IDE**.

**Hỏi: Có cần cài lại mọi thứ mỗi lần mở máy không?**
Đáp: **Không!** Chỉ cần cài một lần. Những lần sau chỉ cần cắm mạch, chọn đúng cổng (Bước 5.4) và nạp code.

**Hỏi: Máy ở trường và máy ở nhà có cần cài riêng không?**
Đáp: Có. Mỗi máy tính đều phải làm đủ 6 bước này một lần.

---

## ✅ Danh sách kiểm tra cuối cùng

Đánh dấu khi bạn đã làm xong:

- [ ] Đã cài **Arduino IDE 2.x**
- [ ] Đã cài gói **esp32 by Espressif Systems**
- [ ] Đã cài thư viện **ControlHub**
- [ ] Đã cài **Adafruit PWM Servo Driver Library**
- [ ] Đã cài **Adafruit BusIO**
- [ ] Đã cài **Adafruit NeoPixel**
- [ ] *(Tùy chọn)* Đã cài **ArduinoJson** và **WebSockets** nếu dùng ví dụ `gripperbot_client`
- [ ] Đã chọn mạch **ESP32S3 Dev Module** và bật **USB CDC On Boot**
- [ ] Đã chọn đúng **Port**
- [ ] Đã nạp thành công ví dụ **L1_HelloRobot** 🎉

Nếu vẫn gặp khó khăn, hãy **chụp màn hình thông báo lỗi** (phần chữ màu cam/đỏ ở dưới cùng Arduino IDE) và gửi cho thầy cô hướng dẫn nhé!

---

*Chúc các bạn học vui và chế tạo được nhiều robot thật ngầu! 🤖🚀*
