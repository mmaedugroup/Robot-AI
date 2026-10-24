# 🛹 Hướng Dẫn Điều Khiển Hoverboard LKS32 Qua UART Passthrough

## 🎯 Bối Cảnh & Mô Tả Vấn Đề (Project Overview)

**Dự án:** Robot tự hành SIFAR (Smarter Intelligent Farm Autonomous Robot).  
**Mục tiêu:** Tận dụng hệ thống cơ khí và driver công suất lớn của 01 Hoverboard cũ để chế tạo robot tự hành có khả năng chạy SLAM và điều hướng Nav2 (ROS2).

### 🧩 Thách thức kỹ thuật:
1. **Chip điều khiển LKS32MC081:** Đây là dòng chip ARM mới trên các board hoverboard đời sau. Khác với dòng STM32 phổ biến, chip LKS32 thường bị khóa bảo vệ (Read-protection) và hiện tại **CHƯA có** firmware Open-source (FOC) hoàn chỉnh và ổn định như hệ sinh thái STM32.
2. **Firmware gốc (Stock Firmware):** Được thiết kế riêng cho việc tự cân bằng (Self-balancing). Nó không có API điều khiển trực tiếp qua UART và có tính logic không tuyến tính (vì phụ thuộc vào cảm biến độ nghiêng IMU nội bộ).
3. **Yêu cầu High-level:** Để chạy được SLAM/Nav2, robot cần **Odometry (vị trí/tốc độ)** cực kỳ chính xác. Firmware gốc thường không gửi dữ liệu RPM qua UART, hoặc nếu có thì độ phân giải rất thấp.

### 💡 Giải pháp lựa chọn: UART Passthrough (Sniff & Spoof)
Thay vì cố gắng bẻ khóa hoặc nạp lại firmware (rủi ro brick board cao), chúng ta sử dụng phương pháp **"Man-in-the-Middle"**:
* **Bước 1:** Nghe trộm (Sniff) giao thức UART giữa Mainboard và Sideboard gốc.
* **Bước 2:** Dùng ESP32-S3 giả lập lại các gói tin đó để điều khiển Mainboard.
* **Bước 3 (Dự phòng):** Nếu dữ liệu RPM từ UART không đủ độ chính xác, sẽ lắp thêm Encoder ngoài (AS5600) hoặc trích dây Hall Sensor để đếm xung độc lập.

---

> **Mục tiêu tài liệu này:** Hướng dẫn ESP32-S3 thay thế hoàn toàn Sideboard để điều khiển Mainboard một cách an toàn và hiệu quả nhất.

---

## 📖 Mục Lục

1. [Tại sao chọn UART Passthrough?](#1-tại-sao-chọn-uart-passthrough)
2. [Nguyên lý hoạt động](#2-nguyên-lý-hoạt-động)
3. [Phần cứng cần chuẩn bị](#3-phần-cứng-cần-chuẩn-bị)
4. [Phase 1 — Sniff: Nghe trộm tín hiệu giữa 2 board](#4-phase-1--sniff-nghe-trộm-tín-hiệu-giữa-2-board)
5. [Phase 2 — Decode: Giải mã giao thức](#5-phase-2--decode-giải-mã-giao-thức)
6. [Phase 3 — Inject: ESP32 thay thế điều khiển](#6-phase-3--inject-esp32-thay-thế-điều-khiển)
7. [Phase 4 — Test từng bước](#7-phase-4--test-từng-bước)
8. [Xử lý sự cố thường gặp](#8-xử-lý-sự-cố-thường-gặp)
9. [Sơ đồ tổng thể cuối cùng](#9-sơ-đồ-tổng-thể-cuối-cùng)

---

## 1. Tại Sao Chọn UART Passthrough?

### ❌ Vấn đề với cách khác

| Phương pháp | Vấn đề |
|---|---|
| Flash firmware mới (EmanuelFeru) | Viết cho STM32, không chạy trên LKS32 |
| Viết firmware từ đầu cho LKS32 | Cực kỳ phức tạp, cần kiến thức FOC chuyên sâu |
| Unlock read-protection LKS32 | Chip có thể bị xóa sạch, rủi ro cao |
| Thay board STM32 | Tốn tiền, mất thời gian tháo lắp |

### ✅ Tại sao UART Passthrough hoạt động?

> **Ý tưởng cốt lõi:** Firmware gốc của hoverboard *vốn đã có* giao tiếp UART giữa 2 board.  
> Chúng ta chỉ cần **"học" ngôn ngữ** đó rồi **ESP32 nói thay** sideboard.

```
Trước đây (bình thường):
[Sideboard] ──UART──► [Mainboard] ──FOC──► [Motor]
  Gyro/remote               Xử lý lệnh

Sau khi làm xong:
[ESP32-S3]  ──UART──► [Mainboard] ──FOC──► [Motor]
  "Não" robot              Xử lý lệnh
```

**Firmware gốc KHÔNG bị thay đổi gì cả.** Ta chỉ thay "nguồn lệnh".

---

## 2. Nguyên Lý Hoạt Động

### 2.1 Kiến trúc giao tiếp gốc

```
┌─────────────────────────────────────────────────────┐
│                   HOVERBOARD                         │
│                                                      │
│  ┌──────────┐   Dây trắng   ┌──────────┐            │
│  │ SIDEBOARD│ ◄───UART─────► │MAINBOARD │            │
│  │(board phải)│              │(board trái)│           │
│  │          │               │          │            │
│  │ LKS32MC  │               │ LKS32MC  │            │
│  └──────────┘               └──────────┘            │
│       │                          │                  │
│    Motor phải                Motor trái              │
└─────────────────────────────────────────────────────┘
```

### 2.2 Cáp trắng giữa 2 board mang những gì?

Cáp trắng (hoặc flat cable) thường gồm nhiều dây:

| Dây | Tín hiệu | Mục đích |
|---|---|---|
| Đỏ | VCC (12V hoặc 5V) | Nguồn cấp |
| Đen | GND | Mass chung |
| Xanh/Vàng | UART TX/RX | Giao tiếp lệnh |
| Các dây khác | Gyro data, Hall sensor | Cảm biến |

> ⚠️ **Quan trọng:** Chúng ta quan tâm nhất đến **dây UART TX/RX**.

### 2.3 Dữ liệu UART chứa gì?

Firmware gốc thường gửi gói tin dạng:

```
[START_BYTE] [SPEED] [STEER] [FLAGS] [CHECKSUM]
```

- **SPEED**: Tốc độ tiến/lùi (giá trị âm = lùi, dương = tiến)
- **STEER**: Góc lái (âm = trái, dương = phải)
- **CHECKSUM**: Kiểm tra lỗi gói tin

*Cấu trúc chính xác phụ thuộc nhà sản xuất — cần sniff để xác định.*

---

## 3. Phần Cứng Cần Chuẩn Bị

### 3.1 Thiết bị bắt buộc

| Thiết bị | Số lượng | Mục đích |
|---|---|---|
| ESP32-S3 DevKit | 1 | Não điều khiển chính |
| USB-to-UART (CH340 hoặc CP2102) | 1 | Kết nối máy tính để sniff |
| Dây hàn (0.3mm) | 1 cuộn | Hàn vào pad UART |
| Mỏ hàn | 1 | Dĩ nhiên 😄 |
| Đồng hồ đo điện (multimeter) | 1 | Xác định chân GND, VCC |
| Logic Analyzer (tùy chọn nhưng rất nên có) | 1 | Giúp decode UART dễ hơn |

### 3.2 Phần mềm cần cài

| Phần mềm | Link | Mục đích |
|---|---|---|
| Arduino IDE | arduino.cc | Lập trình ESP32 |
| PlatformIO (thay thế tốt hơn) | platformio.org | Build code ESP32 |
| PuTTY hoặc CoolTerm | putty.org | Monitor UART serial |
| Sigrok / PulseView | sigrok.org | Đọc logic analyzer (nếu có) |

---

## 4. Phase 1 — Sniff: Nghe Trộm Tín Hiệu Giữa 2 Board

> 🎯 **Mục tiêu Phase 1:** Biết chính xác firmware gốc đang gửi **dữ liệu gì** qua UART.

### 4.1 Tìm chân UART trên cáp nối 2 board

**Bước 1:** Trace dây trắng nối 2 board. Thường là connector dạng Molex hoặc JST.

**Bước 2:** Dùng multimeter đo:
- Chân nào **= 0V** liên tục → GND
- Chân nào **= 3.3V hoặc 5V** liên tục → VCC
- Chân nào **dao động** khi hoverboard chạy → UART TX/RX

**Bước 3:** Xác định TX và RX:

```
💡 Mẹo: Dùng multimeter ở chế độ DC voltage
- Chân IDLE ở mức 3.3V khi không truyền = UART line
- Chân TX của board A = RX của board B và ngược lại
```

### 4.2 Sơ đồ kết nối để sniff

```
                    CÁP NỐI 2 BOARD
                         │
              ┌──────────┴──────────┐
              │   Tap vào giữa cáp  │
              │   (KHÔNG cắt đứt)   │
              └──────────┬──────────┘
                         │
                    ┌────┴────┐
                    │  UART   │
                    │  TX/RX  │
                    └────┬────┘
                         │
              ┌──────────▼──────────┐
              │   USB-to-UART       │
              │   (CH340/CP2102)    │
              │                     │
              │  RX ◄── TX (board)  │
              │  GND ── GND         │
              └──────────┬──────────┘
                         │ USB
                    ┌────▼────┐
                    │  Laptop  │
                    │ PuTTY/   │
                    │ CoolTerm │
                    └─────────┘
```

> ⚠️ **KHÔNG cắt cáp** — chỉ tap vào (nối thêm dây song song).  
> ⚠️ Chỉ kết nối **RX** của USB-UART vào TX của board (chỉ nghe, không gửi).

### 4.3 Lắng nghe tín hiệu

**Bước 1:** Mở PuTTY hoặc CoolTerm

**Bước 2:** Thử các baud rate phổ biến:
```
9600 → 115200 → 19200 → 38400
```
*Baud rate đúng khi bạn thấy ký tự có nghĩa, không phải ký tự loạn.*

**Bước 3:** Bật hoverboard lên, **đặt xuống sàn an toàn**, để nó cân bằng

**Bước 4:** Ghi lại HEX output trên màn hình. Ví dụ bạn có thể thấy:
```
FF 55 00 00 00 FF
FF 55 0A 00 00 F5
FF 55 F6 00 00 09
```

### ✅ QC Checkpoint - Phase 1
- [ ] **Xác định GND:** Multimeter đo thông mạch chân đen với cực âm của acquy.
- [ ] **Tín hiệu RAW:** PuTTY/CoolTerm hiển thị các chuỗi HEX (ví dụ `FF 55...`) chảy liên tục.
- [ ] **Baud rate:** Dữ liệu nhìn sạch sẽ, có cấu trúc lặp lại (không phải các ký tự rác kiểu `?`).
- [ ] **Ghi chép:** Đã chụp màn hình hoặc copy lại ít nhất 5-10 dòng log HEX khi board đứng yên và khi board chạy.

---

## 5. Phase 2 — Decode: Giải Mã Giao Thức

> 🎯 **Mục tiêu Phase 2:** Hiểu ý nghĩa của từng byte trong gói tin.

### 5.1 Phân tích cấu trúc gói tin

Khi đã có dữ liệu HEX raw, tìm **pattern lặp lại**:

```
Ví dụ dữ liệu sniff được:
FF 55 00 00 00 FF   ← Đứng yên, cân bằng
FF 55 14 00 00 EB   ← Tiến chậm
FF 55 28 00 00 D7   ← Tiến nhanh hơn
FF 55 EC 00 00 13   ← Lùi chậm (0xEC = -20 dạng signed)
FF 55 00 0A 00 F5   ← Quay phải
FF 55 00 F6 00 09   ← Quay trái
```

**Phân tích:**
| Byte | Giá trị | Ý nghĩa |
|---|---|---|
| Byte 0 | `0xFF` | START byte (luôn cố định) |
| Byte 1 | `0x55` | START byte 2 (luôn cố định) |
| Byte 2 | thay đổi | **SPEED** (tiến/lùi) |
| Byte 3 | thay đổi | **STEER** (trái/phải) |
| Byte 4 | `0x00` | FLAGS hoặc padding |
| Byte 5 | thay đổi | **CHECKSUM** |

### 5.2 Tính CHECKSUM

Checksum thường tính theo công thức XOR hoặc SUM:

```c
// Thử công thức XOR:
checksum = byte0 ^ byte1 ^ byte2 ^ byte3 ^ byte4

// Thử công thức SUM đơn giản:
checksum = (byte2 + byte3 + byte4) & 0xFF

// Thử complement:
checksum = 0xFF - ((byte2 + byte3) & 0xFF)
```

Kiểm tra: Áp checksum formula vào các gói đã sniff — nếu kết quả khớp byte cuối thì đúng.

### 5.3 Xác định giá trị Speed/Steer

```
Speed = 0x00 = 0    → Đứng yên
Speed = 0x14 = 20   → Tiến (20 đơn vị)
Speed = 0xEC = -20  → Lùi (số âm dạng signed int8)

Steer = 0x00 = 0    → Thẳng
Steer = 0x0A = 10   → Phải
Steer = 0xF6 = -10  → Trái
```

### ✅ QC Checkpoint - Phase 2
- [ ] **Xác định Speed:** Tìm thấy byte tăng dần khi nghiêng board tới và giảm/trở thành số bù 2 khi lùi.
- [ ] **Xác định Steer:** Tìm thấy byte thay đổi khi quay board.
- [ ] **Checksum xác thực:** Áp dụng công thức tính thử (XOR hoặc SUM) vào 1 dòng log bất kỳ, kết quả trả về **phải khớp** với byte cuối cùng của dòng đó.
- [ ] **Độ dài gói:** Biết chính xác 1 gói tin gồm bao nhiêu byte (thường là 6, 8 hoặc 9 byte).

---

## 6. Phase 3 — Inject: ESP32 Thay Thế Điều Khiển

> 🎯 **Mục tiêu Phase 3:** ESP32-S3 gửi đúng gói tin để mainboard thực thi.

### ⚠️ CẢNH BÁO AN TOÀN QUAN TRỌNG (ĐỌC KỸ)
**KHÔNG ĐƯỢC ĐẤU SONG SONG (TAP) CHÂN TX CỦA ESP32 VÀ SIDEBOARD VÀO CÙNG MỘT CHÂN RX CỦA MAINBOARD.**
UART là giao tiếp điểm-đối-điểm (Point-to-Point). Nếu đấu song song, hai thiết bị sẽ tranh chấp đường truyền (Hardware Contention), dẫn đến:
1. **Cháy cổng I/O:** Dòng điện đổ trực tiếp giữa 2 vi điều khiển gây hỏng chip.
2. **Nhiễu dữ liệu:** Mainboard sẽ không nhận được lệnh chính xác.

**Giải pháp:** Sau khi Sniff thành công, bạn phải **NGẮT KẾT NỐI** Sideboard và dùng ESP32 thay thế hoàn toàn vị trí của nó.

### 6.1 Sơ đồ kết nối Dual-UART (Thay thế hoàn toàn)

Trong kiến trúc này, ESP32-S3 sẽ sử dụng 2 bộ Hardware Serial để điều khiển trực tiếp 2 board công suất. **Sideboard cũ không còn kết nối với Mainboard nữa.**

```
       MAINBOARD (Bánh Trái)                        SIDEBOARD (Bánh Phải)
       ┌─────────────────┐                          ┌─────────────────┐
       │   LKS32MC081    │                          │   LKS32MC081    │
       │                 │                          │                 │
       │  RX ◄───────────┼──────────┐    ┌──────────┼─── RX           │
       │  TX ────────────┼──────┐   │    │   ┌──────┼─── TX           │
       │  GND ───────────┼────┐ │   │    │   │ ┌────┼─── GND          │
       └─────────────────┘    │ │   │    │   │ │    └─────────────────┘
                              │ │   │    │   │ │
      ┌───────────────────────┼─┼───┼────┼───┼─┼──────────────────────┐
      │      GND (Nối chung)──┘ │   │    │   │ └─ GPIO16 (RX2 ESP32)  │
      │   GPIO18 (RX1 ESP32) ◄──┘   │    └───┼─── GPIO15 (TX2 ESP32)  │
      │   GPIO17 (TX1 ESP32) ───────┘        │                        │
      │                                      │                        │
      │           ESP32-S3 DevKit            │                        │
      └──────────────────────────────────────┘
```

> **Lưu ý:** Chân GND của ESP32, Mainboard và Sideboard bắt buộc phải nối chung với nhau.

### 6.2 Code ESP32-S3 điều khiển vi sai (Differential)

```cpp
// ============================================
// Hoverboard Dual-UART Controller for SIFAR
// ESP32-S3 → 2 x LKS32MC081 Boards
// ============================================

#define HOVER_SERIAL_BAUD   9600      
#define UART1_TX            17        // Nối Mainboard RX
#define UART1_RX            18        // Nối Mainboard TX
#define UART2_TX            15        // Nối Sideboard RX
#define UART2_RX            16        // Nối Sideboard TX

HardwareSerial LeftBoard(1); 
HardwareSerial RightBoard(2);

typedef struct {
  uint8_t  start1;    // 0xFF
  uint8_t  start2;    // 0x55
  int8_t   speed;     
  int8_t   steer;     // Thường để 0 vì mỗi board chỉ lái 1 bánh
  uint8_t  flags;     
  uint8_t  checksum;  
} HoverCommand;

uint8_t calcChecksum(HoverCommand* c) {
  return (uint8_t)(c->start1 ^ c->start2 ^ c->speed ^ c->steer ^ c->flags);
}

void sendToBoard(HardwareSerial &serial, int8_t speed) {
  HoverCommand cmd;
  cmd.start1   = 0xFF;
  cmd.start2   = 0x55;
  cmd.speed    = speed;
  cmd.steer    = 0;  // Board đơn bánh không cần steer
  cmd.flags    = 0x00;
  cmd.checksum = calcChecksum(&cmd);
  serial.write((uint8_t*)&cmd, sizeof(cmd));
}

// Hàm điều khiển Robot từ lệnh ROS2 (vật lý)
void driveRobot(int8_t linear, int8_t angular) {
  // Tính toán tốc độ vi sai cho 2 bánh
  int8_t leftSpeed  = linear - angular;
  int8_t rightSpeed = linear + angular;
  
  sendToBoard(LeftBoard, leftSpeed);
  sendToBoard(RightBoard, rightSpeed);
}

void setup() {
  Serial.begin(115200);
  LeftBoard.begin(HOVER_SERIAL_BAUD, SERIAL_8N1, UART1_RX, UART1_TX);
  RightBoard.begin(HOVER_SERIAL_BAUD, SERIAL_8N1, UART2_RX, UART2_TX);
}

void loop() {
  // Gửi lệnh 50Hz (20ms)
  driveRobot(20, 0); // Đi thẳng chậm
  delay(20);
}
```

### ✅ QC Checkpoint - Phase 3
- [ ] **Compilation:** Code nạp vào ESP32-S3 thành công, không báo lỗi thư viện.
- [ ] **Wiring:** Dây TX của ESP32 đã nối vào RX của Mainboard (và ngược lại). GND đã nối chung.
- [ ] **Safety:** Mọi điểm trích dây đã được bọc cách điện kỹ càng, không có lõi đồng nào bị hở.
- [ ] **No Error Beep:** Khi bật nguồn, Mainboard không kêu tít tít liên tục (nếu kêu nghĩa là checksum hoặc baud rate trong code đang sai).

---

### 6.3 Nhận phản hồi từ mainboard (tùy chọn)

Mainboard cũng gửi lại dữ liệu (tốc độ thực, voltage, nhiệt độ):

```cpp
// Đọc phản hồi từ mainboard
void readFeedback() {
  if (HoverSerial.available() > 0) {
    uint8_t rawByte = HoverSerial.read();
    Serial.print("RX: 0x");
    Serial.println(rawByte, HEX);
    // Sau khi decode được format → parse thành struct feedback
  }
}
```

---

## 7. Phase 4 — Test Từng Bước

> 🎯 **Quy tắc vàng:** Test từng bước nhỏ, KHÔNG test nhiều thứ cùng lúc.

### 7.1 Test 1: Kiểm tra kết nối UART

```
1. Kết nối ESP32 vào máy tính qua USB
2. Upload code với sendCommand(0, 0) — lệnh đứng yên
3. Bật hoverboard
4. Kiểm tra: Board có phát ra tiếng beep lạ không?
   → Nếu beep liên tục = checksum sai hoặc baud rate sai
   → Nếu im lặng bình thường = kết nối OK
```

### 7.2 Test 2: Kiểm tra lệnh tốc độ nhỏ

```
1. Đặt hoverboard lên giá đỡ (bánh xe KHÔNG chạm sàn)
2. Upload code sendCommand(10, 0) — tiến rất chậm
3. Quan sát: Motor có quay không?
   → Nếu quay = THÀNH CÔNG!
   → Nếu không quay = xem mục Xử lý sự cố
```

### 7.3 Test 3: Test đầy đủ tiến/lùi/quay

```
1. Vẫn để bánh xe KHÔNG chạm sàn
2. Chạy code loop() đầy đủ
3. Quan sát chiều quay motor theo từng lệnh
4. Điều chỉnh dấu (+/-) của speed/steer nếu chiều quay ngược
```

### 7.4 Test 4: Test trên sàn nhà có kiểm soát

```
1. Đặt xuống sàn
2. Tốc độ rất nhỏ (speed = 5)
3. Có người đứng bên cạnh sẵn sàng tắt nguồn
4. Tăng dần tốc độ
```

### ✅ QC Checkpoint - Phase 4
- [ ] **Bánh trái:** Quay tiến khi lệnh `leftSpeed > 0`. Nếu ngược → đảo dấu trong code.
- [ ] **Bánh phải:** Quay tiến khi lệnh `rightSpeed > 0`. Nếu ngược → đảo dấu trong code.
- [ ] **Responsiveness:** Khi dừng lệnh (0,0), cả 2 bánh dừng đồng thời.
- [ ] **Turn Test:** Robot xoay tròn tại chỗ mượt mà (bánh tiến, bánh lùi).
- [ ] **Stability:** Chạy lâu không có hiện tượng 1 bánh bị ngắt giữa chừng do mất đồng bộ UART.

---

## 8. Xử Lý Sự Cố Thường Gặp

| Triệu chứng | Nguyên nhân | Giải pháp |
|---|---|---|
| Motor không quay | Baud rate sai | Thử lần lượt: 9600, 19200, 38400, 115200 |
| Board beep liên tục | Checksum sai | Xem lại công thức checksum từ Phase 2 |
| Motor quay ngược chiều | Dấu speed bị ngược | Đổi `sendCommand(-speed, steer)` |
| Board tắt sau vài giây | Timeout: gửi lệnh quá chậm | Giảm delay xuống ≤ 20ms |
| Dữ liệu sniff toàn ký tự rác | Baud rate sai | Thử từng baud rate |
| Board không phản hồi gì | TX/RX nối ngược | Hoán đổi 2 dây TX và RX |
| ESP32 reset liên tục | Nhiễu từ motor | Thêm tụ 100µF trên đường nguồn ESP32 |

---

## 9. Sơ Đồ Tổng Thể Cuối Cùng

```
┌──────────────────────────────────────────────────────────────────┐
│                        ROBOT HOVERBOARD                           │
│                                                                    │
│   ┌──────────────┐             ┌──────────────────────────────┐  │
│   │  SIDEBOARD   │             │         MAINBOARD            │  │
│   │ (Bánh Phải)  │             │        (Bánh Trái)           │  │
│   └──────┬───────┘             └──────────────┬───────────────┘  │
│          │                                    │                  │
│          │  UART2 (RX2/TX2)    UART1 (RX1/TX1)│                  │
│          └──────────────┬─────────────────────┘                  │
│                         │                                        │
│   ┌─────────────────────▼──────────────────────────────────────┐  │
│   │                    ESP32-S3 DevKit                         │  │
│   │                                                            │  │
│   │  USB ◄─────────────────── Máy tính (debug & upload code) │  │
│   │                                                            │  │
│   │  Logic điều khiển:                                        │  │
│   │  • Tổng tư lệnh (Master của 2 board công suất)            │  │
│   │  • Tính toán tốc độ vi sai cho từng bánh                  │  │
│   │  • Publish/Subscribe ROS2 topics                          │  │
│   │  • Gửi nhịp Heartbeat đồng thời xuống cả 2 kênh           │  │
└──────────────────────────────────────────────────────────────────┘
```

---

## ✅ Checklist Thực Hiện

- [ ] **Chuẩn bị phần cứng** đầy đủ (ESP32-S3, USB-UART, dây hàn)
- [ ] **Phân tích cáp** nối 2 board, xác định chân TX/RX/GND
- [ ] **Phase 1:** Tap vào UART, sniff dữ liệu HEX thô
- [ ] **Phase 2:** Decode giao thức (start byte, speed, steer, checksum)
- [ ] **Hàn dây** TX/RX/GND vào **CẢ 2 bo mạch** (Mainboard & Sideboard)
- [ ] **Phase 3:** Upload code ESP32 (Dual-UART) với thông số đã decode
- [ ] **Test 1:** Xác nhận kết nối UART không gây lỗi cho cả 2 kênh
- [ ] **Test 2:** Cả 2 motor quay khi nhận lệnh (bánh không chạm sàn)
- [ ] **Test 3:** Test đầy đủ tiến/lùi/quay (calibrate chiều quay)
- [ ] **Test 4:** Test trên sàn có kiểm soát

---

> 💡 **Lưu ý cuối:** Nếu sniff được protocol mà vẫn không điều khiển được,  
> rất có thể board này **dùng PWM thay vì UART** giữa 2 board.  
> Trong trường hợp đó, hãy liên hệ lại — sẽ có hướng xử lý khác!
