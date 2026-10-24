# 🤖 BÁO CÁO TIẾN ĐỘ & TỔNG KẾT KỸ THUẬT DỰ ÁN ROBOT HOVERBOARD (SIFAR)
*Ngày cập nhật: 24/10/2026 | Phiên làm việc: Khảo sát thực tế & Thử nghiệm Phần cứng*

---

## 🎯 1. TỔNG QUAN HIỆN TRẠNG PHẦN CỨNG THỰC TẾ

Sau quá trình tháo lắp, đo đạc và kiểm tra thực tế trên xe Hoverboard 10 inch của dự án, các thông số kỹ thuật thực tế đã được xác định chính xác 100% như sau:

| Hạng mục | Thông số trong tài liệu ban đầu | Thực tế kiểm tra phần cứng | Đánh giá & Kết luận |
| :--- | :--- | :--- | :--- |
| **Bánh xe & Motor** | Bánh Hoverboard 10 inch | 02 Motor Hub BLDC 10 inch 36V 350W | **Hoạt động hoàn hảo 100%**. Cuộn dây và bạc đạn trơn tru, phanh từ BEMF tốt. |
| **Bo mạch điều khiển** | Yêu cầu bo đơn chip STM32F103/GD32 | **02 Bo mạch chia (Splitboard) model `HBC-V9.4 0712`** | Không dùng firmware FOC EFeru được vì chip không tương thích ST-Link. |
| **Vi xử lý trên bo** | STM32F103 | **LKS32MC R11C6T8M** (Linko Semiconductor) | Chip ARM nội địa Trung Quốc chuyên cho motor FOC, đã khóa bảo vệ bộ nhớ. |
| **Giao tiếp giữa 2 bo** | UART qua cáp 4 dây | Cáp bẹ 4 dây trắng in chữ: `TX - RX - 15V - GND` | **Đã giải mã thành công giao thức 4 byte ở tốc độ 9600 baud**. |
| **Khối Pin** | Pin Li-ion 36V (10S) | Pin Li-ion 10S xanh bọc màng co | **Bị sụt áp xuống 26V–28V** (đã test thành công bằng cách ghép nối tiếp bình 12V lên 43V). |
| **Bộ não ESP32** | ESP32 DevKit V1 | **ESP32-S3 mạch tím `XH-S3E-AI V1.0`** (Xiaozhi) | Đã nạp code, phát WiFi, tạo web Virtual Joystick điều khiển mượt mà. |

---

## 🔬 2. NHỮNG KẾT QUẢ ĐÃ CHINH PHỤC & NGHIỆM THU ĐẠT

1. **Khối Điều Khiển ESP32-S3 & Web Remote Không Dây:**
   * ESP32-S3 tự phát WiFi: `ROBOT_HOVERBOARD` (không mật khẩu).
   * Web Server chạy tại địa chỉ IP: `192.168.4.1` với giao diện HTML5 tiếng Việt chuẩn UTF-8, có cần gạt ảo (Virtual Joystick) và các nút bấm lệnh nhanh `TIẾN THỬ`, `LÙI THỬ`, `DỪNG LẠI`.
   * Đã cấu hình chuẩn cổng Serial máy tính: Phải bật **`USB CDC On Boot: Enabled`** trong Arduino IDE Tools menu để nhìn thấy log qua Type-C.

2. **Giải Mã Giao Thức UART Của Bo Xe `HBC-V9.4` (9600 Baud):**
   * Định dạng gói tin chuẩn 4 byte:
     $$\mathbf{[0xB5] \quad [Byte\ 1] \quad [Byte\ 2] \quad [0xE7]}$$
   * **Trạng thái Đứng yên (IDLE):** `0xB5` `0xA8` (hoặc `0xE8`) `0xBC` `0xE7`.
   * **Trạng thái Nghiêng trái:** `Byte 1` tụt xuống `0x28` hoặc `0x68`.
   * **Trạng thái Nghiêng phải:** `Byte 2` vọt lên `0xE5`, `0xFD`, `0xFF`.
   * **Trạng thái Nghiêng cả 2 bên:** `0xB5` `0x68` `0xBD` `0xE7`.

3. **Cơ Chế Khóa An Toàn Của Xe:**
   * **Khóa điện áp thấp (Low-Voltage Cutoff):** Khi điện áp $< 33\text{V}$, xe báo đèn đỏ liên tục và ngắt hoàn toàn tầng công suất MOSFET. Khi nâng áp lên $38\text{V} - 43\text{V}$ (bằng cách mắc nối tiếp thêm bình ắc quy 12V), đèn pin chuyển sang xanh ngay lập tức.
   * **Khóa chống ngã / lật xe (Tilt Crash Protection):** Xe phải được đặt thăng bằng đúng chiều xuôi của xe. Nếu lật ngửa, cảm biến IMU đọc ngược trọng trường sẽ kêu tít tít và khóa bánh.
   * **Cảm biến chân người đạp (Optical Foot Sensor):** Hai khe chữ U màu đen ở 2 mép bo mạch phải được nhét mẩu giấy che kín thì xe mới nhận diện có tải để mở khóa động cơ.
   * **Động cơ đã quay thực tế:** Khi nghiêng nhẹ bo mạch bằng tay ở mức nguồn 43V, bánh xe 10 inch đã **tự động quay tiến/lùi cực kỳ khỏe và mượt mà**!

---

## ⚠️ 3. BẢN CHẤT VẤN ĐỀ TỒN TẠI & NGUYÊN NHÂN KỸ THUẬT

* **Vì sao lệnh UART từ ESP32 chưa làm quay bánh xe khi đặt bo nằm phẳng?**
  * Bo mạch Hoverboard nguyên bản được nạp firmware **Tự cân bằng (Self-Balancing)**.
  * Bộ điều khiển của xe luôn ưu tiên tuyệt đối cho **cảm biến góc nghiêng nội tại (IMU MPU6050 gắn chết trên bo mạch)**.
  * Khi bo mạch nằm phẳng ($0^\circ$), cảm biến bên trong bo báo không nghiêng, nên xe giữ nguyên trạng thái đứng yên bất chấp tín hiệu UART gửi tới. Khi lấy tay nghiêng bo mạch, bánh xe lập tức quay vù vù theo tay.

---

## 🛠️ 4. HAI HƯỚNG TRIỂN KHAI TIẾP THEO (TÙY CHỌN)

### Hướng A: Mẹo Cơ Học Dùng Servo Bẻ Góc Cảm Biến (Chi phí: 0 VNĐ - Tận dụng đồ có sẵn)
* **Nguyên lý:** Gắn thân động cơ Servo (SG90) lên 1 nửa xe, cánh tay Servo tỳ vào nửa xe còn lại qua trục xoay nhôm ở giữa.
* **Cách hoạt động:**
  * Điện thoại bấm TIẾN ➡️ ESP32 xoay Servo nghiêng bo mạch $+10^\circ$ ➡️ Xe tự động phóng tới.
  * Điện thoại bấm LÙI ➡️ ESP32 xoay Servo nghiêng bo mạch $-10^\circ$ ➡️ Xe tự động lùi.
  * Buông tay (DỪNG) ➡️ Servo trả bo mạch về phẳng $90^\circ$ ➡️ Xe tự động phanh đứng yên.
* **Mã nguồn:** Đã viết sẵn trong thư mục `firmware/esp32_s3_hoverboard_controller/`.

### Hướng B: Thay 2 Bo Điều Khiển BLDC Đa Năng 36V 350W (Khuyên dùng cho Robot thực thụ)
* **Chi phí:** Khoảng 120.000 – 150.000 VNĐ / bo trên Shopee (Tổng ~300k cho 2 bánh).
* **Tìm kiếm Shopee:** *"Bo điều khiển xe đạp điện đa năng 36V 48V 350W không chổi than"*.
* **Cách nối:**
  * Giữ nguyên: 2 bánh xe 10 inch (3 dây pha to + 5 dây Hall cắm thẳng vào bo xe điện).
  * Giữ nguyên: Nguồn pin 36V.
  * Điều khiển từ ESP32: Nối dây Ga (Throttle) vào chân PWM của ESP32, dây Đảo chiều (Reverse) vào 1 chân GPIO của ESP32.
* **Ưu điểm:** Vứt bỏ hoàn toàn cảm biến nghiêng, không còn tiếng kêu bíp bíp, robot chạy tiến/lùi mượt mà chính xác từng centimet để sau này gắn LiDAR và chạy SLAM/Nav2.

---

## 📌 5. SƠ ĐỒ CHÂN KẾT NỐI ESP32-S3 MẠCH TÍM (XH-S3E-AI V1.0)

| Thứ tự chân trên hàng 8 chân | Tên in trên mạch | Chức năng trong dự án |
| :--- | :--- | :--- |
| **Chân 1 (Trên cùng)** | `TXD` | GPIO 43 (Dùng cho USB máy tính) |
| **Chân 2** | `RXD` | GPIO 44 (Dùng cho USB máy tính) |
| **Chân 3** | `GND` | Nối Mass chung với bo Hoverboard và Servo |
| **Chân 4** | `VIN` | Nguồn 5V nuôi mạch ESP32 và cấp nguồn đỏ cho Servo |
| **Chân 5** | `IO41/SDA` | GPIO 41 (Chân điều khiển Servo PWM hoặc chân RX đọc xe) |
| **Chân 6** | `IO42/SCL` | GPIO 42 (Chân truyền lệnh TX sang xe) |
| **Chân 7** | `3V3` | Nguồn 3.3V out |
| **Chân 8 (Dưới cùng)** | `GND` | Nối Mass phụ |

---

## 🚀 6. HƯỚNG DẪN DÀNH CHO PHIÊN LÀM VIỆC TIẾP THEO

Khi mở lại dự án ở bất kỳ IDE hoặc phiên làm việc AI mới, bạn chỉ cần gửi yêu cầu:
> *"Tôi muốn tiếp tục dự án Robot Hoverboard SIFAR. Hãy đọc file `BAO_CAO_TIEN_DO_VA_HUONG_DAN_KY_THUAT.md` và mã nguồn trong thư mục `firmware/` để nắm rõ hiện trạng."*
AI mới sẽ lập tức nắm bắt toàn bộ ngữ cảnh, mã nguồn và tiếp tục đồng hành cùng bạn mà không cần làm lại từ đầu!
