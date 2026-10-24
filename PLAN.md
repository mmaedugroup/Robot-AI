# KẾ HOẠCH DỰ ÁN ROBOT SIFAR (Smart Indoor Farm Assistant Robot)
*Phiên bản Tổng hợp - Dành cho Người Không Chuyên Kỹ Thuật (với sự hỗ trợ của AI)*

> [!IMPORTANT]
> **CẬP NHẬT THỰC ĐỊA MỚI NHẤT (Tháng 10/2026):**
> Đã khảo sát phần cứng thực tế và thử nghiệm thành công động cơ 10 inch, giải mã giao thức UART và lập trình điều khiển qua ESP32-S3 mạch tím. Chi tiết báo cáo tiến độ, kết quả đo đạc và mã nguồn xem tại:
> * 📄 Báo cáo kỹ thuật: [BAO_CAO_TIEN_DO_VA_HUONG_DAN_KY_THUAT.md](file:///d:/ANTIGRAVITY%20APPs/ROBOT%20AI%20HOVERBOARD/BAO_CAO_TIEN_DO_VA_HUONG_DAN_KY_THUAT.md)
> * 💻 Mã nguồn ESP32-S3 hoàn chỉnh: [esp32_s3_hoverboard_controller.ino](file:///d:/ANTIGRAVITY%20APPs/ROBOT%20AI%20HOVERBOARD/firmware/esp32_s3_hoverboard_controller/esp32_s3_hoverboard_controller.ino)

## 1. TỔNG QUAN DỰ ÁN (Project Overview)
**SIFAR** là hệ thống robot tự hành di động được thiết kế chuyên biệt cho trang trại nuôi trồng thủy sản tuần hoàn trong nhà (RAS). Robot sẽ thay thế và số hóa các công việc lặp đi lặp lại như: tuần tra, quan sát cá mắc bệnh, kiểm tra hệ thống lọc nước, và rải thức ăn thông minh theo đúng nhu cầu.

**Mục tiêu cốt lõi của tài liệu này:** Được thiết kế để một người **không có chuyên môn sâu về lập trình (non-tech)** hoàn toàn có thể từng bước tự phát triển thành công hệ thống robot phức tạp này, thông qua phương pháp "Vibe Coding" (Lập trình qua tương tác với AI: ChatGPT, Claude,...).

### 1.1 Tính năng chính
- 🚀 **Tự hành (SLAM & Nav2):** Tự di chuyển và lập bản đồ nhờ LiDAR, tránh chướng ngại vật an toàn.
- 🐟 **Cho ăn thông minh & Phân tích hành vi:** Tự động rải cám. Camera AI và thuật toán Optical Flow đánh giá mức độ thèm ăn của cá để điều chỉnh lượng thức ăn (tránh dư thừa).
- 👁️ **Giám sát sức khỏe:** Nhận diện cá bệnh/chết, báo cáo tức thì qua Telegram.
- 🔋 **Tự động hóa toàn diện (Giai đoạn 2):** Khả năng tự động quay về trạm sạc pin (Auto-docking) và tự nạp thức ăn (Auto-refill).

### 1.2 Phương pháp Vibe Coding (Dùng AI để lập trình)
Bạn không cần tự viết code, hãy để AI làm điều đó. Cách hỏi AI hiệu quả:
1. **Không hỏi chung chung:** Thay vì *"Viết code robot cho tôi"*.
2. **Hãy hỏi chi tiết:** *"Tôi đang dùng hệ điều hành Ubuntu 22.04, ROS2 Humble và Python. Hãy viết cho tôi một node tên `serial_bridge` đọc dữ liệu từ cổng /dev/ttyUSB0 với baudrate 115200. Hãy giải thích từng dòng code."*
3. **Sửa lỗi (Debug):** Copy toàn bộ dòng báo lỗi màu đỏ trên màn hình, dán cho AI kèm câu: *"Tôi chạy lệnh X và bị lỗi này. Chỉnh sửa code giúp tôi."*

---

## 2. KIẾN TRÚC HỆ THỐNG (System Architecture)

Hệ thống SIFAR được thiết kế module hóa, chia thành 2 khối chính để đảm bảo an toàn và hiệu năng thực thi:
1. **Low-Level Control (ESP32 + STM32 Hoverboard):** Chịu trách nhiệm phần cứng thời gian thực. Băm xung từ encoder, tính toán PID, điều khiển động cơ quay, đọc cảm biến IMU (MPU6050) ở tần số cao (100Hz).
2. **High-Level Compute (Laptop hoặc Orange Pi 5):** "Bộ脑" chạy hệ điều hành Ubuntu & ROS2 Humble. Chịu trách nhiệm xử lý thuật toán nặng như nhận diện AI (YOLO), điều hướng (SLAM, Nav2), và ra quyết định.

---

## 3. DANH SÁCH VẬT TƯ (BOM - Bill of Materials)

Đây là các linh kiện cốt lõi cần chuẩn bị.

### 3.1 Giai đoạn 1 (Robot Cơ bản ~ 10.6 triệu VNĐ)
* **Khung gầm & Chuyển động:**
  - 1x **Hoverboard cũ (10 inch):** CỰC KỲ QUAN TRỌNG - Phải đảm bảo bo mạch dùng chip STM32F103 hoặc GD32F103 (Không mua loại chip lạ).
  - 1x Khung nhôm định hình 2020 (~60x70cm) + Bánh xe Caster phía trước.
  - 1x Hệ thống phanh đĩa xe đạp + Solenoid 12V (Phanh Fail-safe: mất điện tự động khóa bánh).
* **Mạch điều khiển:**
  - 1x Orange Pi 5 (Khuyến nghị bản 16GB RAM) + Tản nhiệt quạt 5V.
  - 1x ESP32 Dev Kit V1.
  - 1x Mạch nạp ST-Link V2 (để nạp firmware cho bo Hoverboard).
* **Cảm biến:**
  - 1x MPU6050 (IMU - Đo góc nghiêng, gia tốc).
  - 1x RPLiDAR A1 (Quét chướng ngại vật).
  - 1x Camera USB góc siêu rộng (>120 độ).
* **Cơ cấu nạp:** Phễu Inox 304 + Motor trục vít + Module ACS712 đo dòng (phát hiện kẹt) + Servo SG90 (chỉnh hướng rải).
* **Nguồn điện:** Mạch Buck DC-DC XL4016 (Hạ 36V xuống 5V 10A cho Orange Pi) + Tụ lọc 1000uF để chống khởi động lại (crash) đột ngột do NPU chạy AI.

### 3.2 Giai đoạn 2 (Trạm sạc tự động & Refill ~ 4.5 triệu VNĐ)
* **Trạm sạc:** Mạch sạc 42V, tiếp điểm đồng, Cảm biến IR định vị Docking.
* **Trạm rải thức ăn lớn:** Bồn chứa thức ăn dự trữ + Van điện từ (Solenoid Valve).
* **Nâng cấp Pin:** Lên Li-ion 36V 10Ah để tăng thời lượng.

---

## 4. LỘ TRÌNH THỰC HIỆN TỪNG BƯỚC (Step-by-Step Implementation)

Dự án được chia thành các giai đoạn nhỏ để dễ dàng quản lý rủi ro.

### GIAI ĐOẠN 0: MÔ PHỎNG CHI PHÍ THẤP (Tuần 1)
Trước khi làm phần cứng, hãy thử nghiệm hệ thống trong không gian ảo.
1. **Cài đặt môi trường:** Cài Ubuntu 22.04 (hoặc cài dual-boot trên Laptop). Cài ROS2 Humble. Cài phần mềm mô phỏng Gazebo.
2. **Tạo Robot ảo:** Nhờ AI viết file `URDF` định nghĩa hình dáng robot (khối hộp chữ nhật có 2 bánh xe). Đưa robot vào môi trường Gazebo.
3. **Chạy SLAM ảo:** Cài gói `slam_toolbox`, dùng bàn phím lái robot đi vòng quanh để vẽ bản đồ ảo.

### GIAI ĐOẠN 1A: CHẾ TẠO PHẦN CỨNG CHIẾN LƯỢC (Tuần 2 - 4)
Khâu phần cứng quyết định sự ổn định của toàn hệ thống.
1. **"Hack" Hoverboard:** 
   - Tháo vỏ, giữ lại 2 bánh motor, pin 36V, và bo mạch chủ.
   - Nạp firmware FOC (kho tàng mã nguồn mở của Emanuel Feru) qua ST-Link V2. Lệnh điều khiển sẽ đổi từ cảm biến nghiêng sang nhận lệnh tốc độ bánh trái/phải thông qua cổng UART.
2. **Lắp ráp bộ khung:** Khung nhôm 2020 cần gia cố vững chắc, đặc biệt là cột cao (~1.0m) gắn Camera và Orange Pi. Cột Camera cần có thanh chéo/ke góc kép để không bị rung lắc khi robot thắng gấp đĩa phanh.
3. **Lắp ráp & Mạch điện:** Cấp nguồn riêng lẻ (dùng Buck DC) cho Orange Pi (5V-10A), cho ESP32 & Servo (5V-5A), cho Solenoid phanh (12V-3A) để cô lập nhiễu nguồn đi lên não bộ robot.

### GIAI ĐOẠN 1B: KẾT NỐI PHẦN MỀM CƠ BẢN (Tuần 5 - 8)
1. **Viết mã vi điều khiển ESP32:** Nhờ AI viết code (C++/Arduino) để ESP32 trở thành trạm trung chuyển (Bridge). ESP32 sẽ đảm nhiệm việc trò chuyện với board Hoverboard qua chuẩn UART, đọc IMU gửi lên cho Orange Pi, điều khiển Servo. Gắn kèm Timestamp MS vào mỗi gói tin.
2. **Setup Mạng ROS2:** Trong Ubuntu (Orange Pi 5/Laptop), nhờ AI tạo các Node cơ bản (Node giao tiếp Serial, Node bật Lidar, Node Camera).
3. **Cấu hình BỘ LỌC EKF (Kalman Filter):** Vô cùng quan trọng. Để robot không bị "Trôi" tọa độ (drift), kết hợp dữ liệu vòng quay bánh xe (Odometry), MPU6050 (IMU) và ArUco Marker vào module `robot_localization` để tính ra quỹ đạo thực của robot mượt mà.

### GIAI ĐOẠN 1C: BỘ NÃO AI VÀ ĐIỀU HƯỚNG TỰ ĐỘNG (Tuần 9 - 12)
1. **Bản đồ thật & Nhận diện ArUco:** Bật `slam_toolbox`. Trong môi trường trại cá, các bể rất giống nhau (Perceptual Aliasing), dễ làm Lidar bị lạc. Khắc phục: in các mã QR đặc biệt (ArUco markers) dán ở 4 góc phòng và đầu mỗi dãy bể. Khai báo tọa độ thực `aruco_map.yaml`. Camera nhận diện ArUco sẽ thiết lập lại sai số vị trí bằng 0.
2. **Điều hướng (Nav2):** Load bản đồ đã quét. Cấu hình điểm đến (Waypoint) trước các bể cá. Robot sẽ tự tính đường đi qua mọi điểm và tránh người cản đường. Cấu hình cẩn thận `inflation_layer`.
3. **Cho ăn & Xử lý sự cố nạp (Gỡ kẹt - Unjam):**
   - Rải thức ăn: Nhờ AI lập trình module phát hiện dòng điện (qua ACS712).
   - Nếu động cơ bị kẹt cám (Dòng điện tăng vọt > 1A quá 500ms): Đưa servo xả về góc trung gian (neutral) 90 độ, ESP32 tự động quay ngược trục vít 2 giây, dừng 1s rồi quay thuận lại. Thất bại quá 2 lần sẽ báo Telegram.
4. **AI Nhận diện cá (YOLOv8):**
   - Thu thập ảnh chụp bề mặt bể. Máy tính phân tích (Google Colab hoặc máy Laptop cá nhân) train model YOLO với dataset khởi tạo (Bootstrap ~300 ảnh) -> Fine-tune với ảnh thực tế trại cá (1000 ảnh). 
   - Chuyển đổi file model sang `.rknn` để Orange Pi 5 chạy trực tiếp trên chip AI siêu tốc độ NPU (30 FPS).
   - *Optical Flow:* Module luồng quang học phân tích hình dạng sóng nước để quyết định cá thèm ăn hay đã no. Giảm 50% thức ăn rải hoặc dừng ngay.

### GIAI ĐOẠN 2: NÂNG CẤP VÀ TỰ ĐỘNG HÓA CAO CẤP (Giai đoạn sau)
- Chế tạo trạm sạc: Robot tự sử dụng điểm chớp hồng ngoại (IR) dò mục tiêu và lùi đít vào 2 cọc sạc 42V (sai số +- 2cm) với module thuật toán PID.
- Chế tạo bồn chứa tổng (Auto Refill): Dùng cảm biến quang phát hiện phễu cạn, tự mở và nạp thêm thức ăn tổng qua trạm treo cao.
- Telegram Bot Alerting: Cảnh báo liên tục qua điện thoại nếu có lỗi hệ thống (như sụt pin < 20%, mất kết nối wifi quá 10s tự ghim phanh khẩn cấp đĩa xe, cá chết, nước sủi bọt).

---

## 5. BẢNG KIỂM SOÁT VÀ QUẢN LÝ CÁC RỦI RO QUAN TRỌNG TỐI CAO (RISK MANAGEMENT)

Đừng để mất công sức tốn thời gian, hãy lưu ý trước:

| Tình huống Rủi Ro | Hậu Quả & Dấu Hiệu | Phương Án Khắc Phục (Chống nhiễu/Dự phòng) | Mức độ |
| --- | --- | --- | --- |
| **Bo mạch Hoverboard KHÔNG thể "Hack"** | ST-Link không nhận chíp. Đọc mờ mờ thấy ghi chíp xanh lạ (Ví dụ: MM32, CKS32, MH32, AT32...) không phải STM32/GD32. Chớp đèn đỏ x3. | Mua cẩn thận chọn kỹ chip. **Kế hoạch B (Plan B):** Mua bộ KIT FOC Driver rờivà mã nguồn mở như ODrive, SimpleFOC (tương thích ESP32 luôn). Giữ nguyên dây điện + Mô tơ Hub cũ. | **Cao Nhất** |
| **OrangePi 5 "Crash" Khởi Động Lại liên tục** | Khi bật Node AI Camera chạy tốn 5A dòng tức thì, kéo tụt điện áp 5V. | Lắp thêm cụm Tụ Điện Lọc Nguồn lớn (2x1000uF + 1 tụ gốm 100nF mắc song song) ngay giắc nguồn 5V đi vào mạch Buck XL4016 (dòng 10A). Tuyệt đối không dùng dòng mạch yếu LM2596. | Cao |
| **Nhiễu Điện (EMI) đi từ động cơ Hoverboard** | IMU ESP32 gửi thông báo bị rác (Noise) hoặc Delay MS liên tục OPi5 chập chờn liên hoàn. Cổng Serial chết cứng. | Bọc dây Motor bằng giấy bạc/hạt ferit. Tách cách ly riêng phần cấp nguồn hệ thống 5V Logic (Opi5, Lidar, ESP) bằng dây cấp riêng nối thẳng từ cọc âm dương pin 36V chứ không đấu nối sau qua công tắc nhồi Motor. | Trung |
| **Quá nhiệt CPU/NPU gây Mất Phản Xạ Nav2** | Nhiệt độ nhảy > 85 độ. Cụm xử lý tự hạ gài tần số (Throttling) khiến AI và SLAM không cập nhật theo thời gian thực kịp tốc dộ đi thực tế. Robot lao sầm vào tường hay bể nát camera. | Luôn luôn tản nhiệt nhôm lớn + Quạt 5V thốc trực tiếp vào vi xử lý RockChip OPi5. Giới hạn set `CPU governor`. | Cao |
| **Bản đồ Lidar méo biến dạng, Vong Lặp Hỏng** | Lặp quá nhiều điểm đường nét. 50 cái bể tròn nhìn Lidar SLAM giống hệt 1 chuỗi dài. (Perceptual Aliasing/Nhầm Vị Trí Điểm Ngắm). | Bắt buộc dán ArUco Marker. Đổi `slam_toolbox` sang chế độ `localization mode` tắt tự đóng lặp lại Loop Closure tự động đi. Bổ trợ đo cự li chính xác qua file YAML gắn marker mốc tọa độ sàn x, y vĩnh viễn. | Cao |
| **Nhả Cám Bị Văng Tung Tóe Ra Sàn Nhà** | Rải cám sai tâm vòng ngoài lúc gỡ nghẹt cám. Máy đảo chiều vít đùn quay ngược. Cơ cấu servo đang chếch ngang văng tung toé. | Phần mềm ESP: Ép code phải xoay Servo ống chỉ góc 90 độ thu thẳng đứng mũi nhả xuống họng xả TRƯỚC 0.5 giây rồi mới kích khởi động cơ trục vít đảo chiều vòng gỡ rối. | TB |
| **Vấp Vật Cản Giật Đổ Gãy Ngã Cột Camera** | Robot lao vấp hố / Lidar dừng thắng khẩn cấp quá gấp 0.5s tắt ngay động cơ 30Kg đĩa phanh giật hãm mạnh ngã nhào cắm gãy cả cọc Camera 1.5m đứt dây. | Tối đa Moment lật. Giới hạn tốc độ `max_vel_x = 0.5 m/s` trong file param Nav2. Gắn chống chéo (Thanh giằng Bracket/Bản mã V/Guy wire thép) kiên cố 2 điểm chân Mast với mâm xe đúc thép. | Trung |

---

## 6. KẾ HOẠCH BẢO TRÌ

- **Hàng ngày:** Lau ống rải cám (tránh cám dính ẩm), nhặt rác vướng bánh xe. 
- **Hàng tuần:** Xiết chặt ốc nhôm cọc Camera Mast. Thử tắt bật điện 12V test cụm dây kéo thắng đĩa phanh xe đạp an toàn.
- **Hàng tháng:** Lau gương quay Lidar và Kính ống kính Camera. Test van điện từ (Solenoid) đóng nhả phanh đĩa thủ công 5 lần liên tục. 
- **6 Tháng:** Kiểm tra tiếp điểm trạm sạc đồng oxy hóa rỉ đen đánh nhám sạch sẽ, tra dầu vòng bi Caster. Train bổ sung và Fine-tune lại File AI TFLite/RKNN NPU theo mùa độ mờ màu nước.

***Dự án này là minh chứng: Miễn là bạn theo đúng bài bản thiết kế, kết hợp tài liệu này và công cụ AI hỗ trợ, bạn sẽ làm chủ hoàn toàn!***
