# DỰ ÁN ROBOT TRỢ LÝ TRANG TRẠI TRONG NHÀ THÔNG MINH
# (Smart Indoor Farm Assistant Robot — SIFAR)

> **Phiên bản:** 4.2 | **Cập nhật:** Tháng 2/2026
> **Loại tài liệu:** Kế hoạch kỹ thuật chi tiết (Technical Project Plan)

---

## 1. Tổng quan dự án

### 1.1. Mô tả ngắn gọn

Dự án chế tạo một **robot di động tự hành** hoạt động bên trong **trang trại nuôi trồng thủy sản trong nhà (Indoor Aquaculture Farm)**, nơi sử dụng hệ thống tuần hoàn nước RAS (Recirculating Aquaculture System). Robot có khả năng:
- **Tuần tra giám sát bể nuôi:** Di chuyển dọc các dãy bể nhỏ (mỗi bể cao ~1.2m), dùng Camera AI nhìn từ trên xuống để quan sát sức khỏe cá/tôm trong từng bể.
- **Giám sát hệ thống lọc RAS:** Kiểm tra trực quan các bể lọc — mức nước, độ trong, bọt, trạng thái thiết bị phụ trợ.
- **Cho ăn tự động & thông minh:** Rải thức ăn vào từng bể theo lịch, Camera AI quan sát phản ứng ăn mồi để tự điều chỉnh lượng thức ăn.
- **Tự hành SLAM:** Dùng Lidar 360° lập bản đồ trang trại, tự di chuyển chính xác giữa các bể mà không cần người điều khiển. Hệ dẫn động sử dụng **motor brushless từ hoverboard** — yên tĩnh, bền bỉ, điều khiển FOC mượt mà.
- **Tự động sạc pin (Auto-Docking):** Khi pin yếu, robot tự quay về trạm sạc, căn chỉnh bằng cảm biến IR và tự nạp điện.
- **Tự động nạp thức ăn (Auto-Refill):** Khi hết thức ăn trong phễu, robot tự di chuyển tới trạm nạp liệu để nhận thức ăn mới.
- **Cảnh báo thời gian thực** qua Telegram/Push khi phát hiện bất thường (cá chết, nước đục, thiết bị lọc lỗi).
- **Khả năng mở rộng:** Kiến trúc phần mềm dạng module — dễ dàng thêm chức năng: quét mã QR định danh bể, đo cảm biến nước không tiếp xúc, điều khiển van/bơm từ xa...

### 1.2. Phạm vi triển khai

- **Giai đoạn 1 (MVP):** Robot tự hành SLAM, tuần tra bể nuôi, cho ăn thông minh, Camera AI phân tích hành vi cá. Hệ dẫn động: **Hoverboard 10" + Khung tự chế + Caster + Phanh đĩa fail-safe**. Bộ xử lý: **Orange Pi 5 (RAM 8GB)** chạy Ubuntu + ROS2 — xử lý trung tâm mọi tác vụ AI & SLAM.
- **Giai đoạn 2 (Nâng cấp):** Thêm auto-docking sạc pin, auto-refill thức ăn, giám sát RAS, cảm biến nước. Tối ưu hóa thuật toán AI trên NPU Rockchip.

**[CẬP NHẬT v4.1]** Dự phòng dẫn động: Nếu board hoverboard không tương thích firmware Emanuel Feru (chip clone GD32 hoặc pinout khác), sẽ chuyển sang **Plan B — ODrive/SimpleFOC** điều khiển trực tiếp motor BLDC (xem chi tiết tại Section 5.1 và Section 12).

### 1.3. Điểm khác biệt so với giải pháp hiện tại

| Tiêu chí | Giám sát thủ công | Robot SIFAR |
| :--- | :--- | :--- |
| Kiểm tra bể | Người đi từng bể, 2–3 lần/ngày | Robot tuần tra tự động 24/7, AI phân tích |
| Phát hiện cá bệnh | Bằng mắt thường, dễ bỏ sót | AI nhận diện hành vi bất thường, cảnh báo tức thì |
| Cho ăn | Rải tay hoặc máy cố định | Robot rải từng bể, AI điều chỉnh lượng theo phản ứng cá |
| Giám sát RAS | Phải đi kiểm tra thủ công | Robot tự quét, cảnh báo khi bể lọc bất thường |
| Hoạt động ban đêm | Không | Robot tuần tra liên tục, tự sạc pin |
| Khả năng mở rộng | Rất hạn chế | Thêm cảm biến, quét QR, điều khiển van/bơm |

---

## 2. Phân tích môi trường trang trại trong nhà

### 2.1. Đặc điểm môi trường

| Đặc điểm | Điều kiện | Ảnh hưởng đến thiết kế |
| :--- | :--- | :--- |
| **Bể nuôi cao ~1.2m** | Bể composite/nhựa, miệng mở hoặc có lưới | Robot cần cột mast camera cao ≥ 1.3m để nhìn xuống bể |
| **Không gian hẹp** | Lối đi giữa các dãy bể rộng ~1–1.5m | Robot chiều rộng ≤ 65cm, quay tại chỗ (differential drive) |
| **Nền phẳng** | Bê tông/gạch/epoxy, phẳng, có thể ẩm ướt | Bánh hoverboard 10" (D=254mm) bám tốt, vượt ngưỡng cửa/rãnh thoát nước dễ |
| **Môi trường ẩm + hóa chất** | Độ ẩm cao, chlorine, muối (bể mặn) | Khung nhôm/nhựa chống ăn mòn, mạch IP54, cáp chống ẩm |
| **Ánh sáng nhân tạo** | Đèn LED trang trại (có thể tắt ban đêm) | Camera cần hoạt động cả điều kiện sáng lẫn thiếu sáng |
| **Hệ thống RAS** | Bể lắng, bể lọc sinh học, UV, máy bơm | Robot cần giám sát trực quan: mức nước, bọt, độ trong |
| **Trạm cố định** | Có vị trí đặt trạm sạc + trạm nạp thức ăn | Robot cần cơ cấu docking chính xác (cảm biến IR) |

### 2.2. Bài toán cần giải quyết

> Chế tạo robot tự hành cho trang trại trong nhà (< 10 triệu VNĐ GĐ1), tận dụng **motor brushless từ hoverboard cũ** làm hệ dẫn động (motor BLDC 350W, pin 36V Li-ion, điều khiển FOC), kết hợp **khung tự chế + 2 caster + phanh đĩa fail-safe**, tích hợp Lidar SLAM + Camera AI từ điện thoại Android, di chuyển trong lối đi giữa các dãy bể cao ~1.2m, giám sát sức khỏe cá từng bể, cho ăn thông minh, và mở rộng chức năng (auto-docking, giám sát RAS) — phù hợp cho trang trại RAS quy mô vừa và nhỏ.

---

## 3. Yêu cầu kỹ thuật

### 3.1. Yêu cầu chức năng (Functional Requirements)

| ID | Yêu cầu | Mức ưu tiên | Giải thích đơn giản |
| :---: | :--- | :---: | :--- |
| FR-01 | Di chuyển tự hành trong trang trại | Bắt buộc | SLAM Lidar lập bản đồ, robot tự di chuyển giữa các bể |
| FR-02 | Điều khiển từ xa qua WiFi | Bắt buộc | Người dùng có thể can thiệp thủ công từ Laptop/phone |
| FR-03 | Tuần tra bể nuôi (Patrol Mode) | Bắt buộc | Robot đi dọc từng dãy bể, Camera AI nhìn xuống từng bể |
| FR-04 | Cho ăn tự động từng bể | Bắt buộc | Dừng tại mỗi bể, rải thức ăn đúng lượng vào miệng bể. **[CẬP NHẬT v4.1]** Bao gồm cơ chế phát hiện kẹt phễu (đo dòng motor rải) và recovery tự động (đảo chiều auger) |
| FR-05 | Camera AI phân tích hành vi cá | Bắt buộc | Nhận diện cá bệnh, đánh giá phản ứng ăn mồi. **[CẬP NHẬT v4.2]** Độ chính xác target ≥80% đạt được sau giai đoạn fine-tune (cuối GĐ1), không phải ngay từ đầu. Dùng YOLOv8n + optical flow — xem chi tiết dataset & thuật toán bên dưới |
| FR-06 | Tránh vật cản (Lidar) | Bắt buộc | Lidar quét 360° phát hiện người/vật cản → dừng/chờ |
| FR-07 | Cảnh báo thời gian thực | Bắt buộc | Gửi Telegram khi phát hiện bất thường |
| FR-08 | Cảnh báo pin yếu | Bắt buộc | Thông báo khi pin sắp hết |
| FR-09 | Hẹn giờ tuần tra + cho ăn | Bắt buộc | Đến giờ → Robot tự khởi động nhiệm vụ |
| FR-10 | Giám sát hệ thống RAS | GĐ2 | Camera kiểm tra bể lọc: mức nước, bọt, trạng thái thiết bị |
| FR-11 | Tự động sạc pin (Auto-Docking) | GĐ2 | Pin < 20% → robot tự tìm trạm sạc, căn chỉnh, nạp điện |
| FR-12 | Tự động nạp thức ăn (Auto-Refill) | GĐ2 | Hết thức ăn → robot tự tới trạm nạp, mở phễu, nhận thức ăn |
| FR-13 | Quét mã QR định danh bể | GĐ2+ | Đọc mã QR trên mỗi bể để xác định chính xác bể nào |
| FR-14 | Phát hiện cá bệnh / cá chết | Nên có | AI phân tích hành vi bơi bất thường, cá nổi, đổi màu. **[CẬP NHẬT v4.1]** Transfer learning từ YOLOv8n pre-trained, fine-tune với dataset trang trại thực tế |

**[CẬP NHẬT v4.1] Kế hoạch Dataset AI & Thuật toán nhận diện cá (FR-05, FR-14):**

**A. Kế hoạch thu thập Dataset:**

| Hạng mục | Chi tiết |
| :--- | :--- |
| **Số lượng tối thiểu** | 500 ảnh/class × 6 class = **3.000 ảnh** (trước augmentation) |
| **Các class cần label** | `healthy_fish` (cá khỏe), `sick_fish` (cá bệnh — bơi lờ đờ/nghiêng), `dead_fish` (cá chết nổi), `normal_feeding` (cá bơi hướng thức ăn), `active_feeding` (cá ăn tích cực — tụ đông), `no_feeding` (cá bỏ ăn — phân tán) |
| **Điều kiện chụp** | Góc từ trên xuống (top-down từ mast 1.35m), qua mặt nước, dưới đèn LED trang trại (trắng/vàng), cả ban ngày lẫn ban đêm (đèn tắt + IR camera GĐ2) |
| **Công cụ label** | LabelImg hoặc Roboflow (export YOLO format) |
| **Thu thập khi nào** | Giai đoạn 2 (Tuần 5–8), mỗi ngày chụp ~100 ảnh tại trang trại thực tế |

**B. Transfer Learning Strategy:**

| Bước | Chi tiết |
| :--- | :--- |
| **Model gốc** | YOLOv8n (nano) pre-trained trên COCO → convert sang RKNN cho NPU |
| **Fine-tune** | Freeze backbone (CSPDarknet53), chỉ train head detection trên dataset trang trại |
| **Augmentation phù hợp ảnh qua mặt nước** | Motion blur (kernel 5–15px), Random reflection (horizontal flip + brightness jitter ±30%), Barrel distortion (mô phỏng khúc xạ mặt nước, k1=0.1–0.3), Color jitter (hue ±10°, saturation ±20%) — xử lý ánh sáng LED thay đổi |
| **Epochs** | 100–200 epochs, batch size 16, learning rate 0.001 → 0.0001 |
| **Validation** | 80/20 train-val split, đánh giá mAP@0.5 |

**C. Thuật toán `feeding_analyzer.py` (Optical Flow):**

```
Input: 2 frame liên tiếp từ camera (t và t+1)
1. Detect vùng ROI chứa thức ăn vừa rải (tọa độ từ feeding waypoint)
2. cv2.goodFeaturesToTrack() → tìm feature points trên cá
3. cv2.calcOpticalFlowPyrLK(frame_t, frame_t1, points) → tính vector di chuyển
4. Đếm số vector hướng VỀ PHÍA thức ăn (cosine similarity > 0.7 với vector tới ROI)
5. Tính tỷ lệ: ratio = vectors_toward_food / total_vectors

Ngưỡng quyết định:
  - ratio > 0.6  → ACTIVE_FEEDING  → Rải đủ lượng cài đặt
  - 0.3 < ratio ≤ 0.6 → REDUCED_FEEDING → Giảm 50% lượng, ghi log
  - ratio ≤ 0.3 → NO_FEEDING → Dừng rải, cảnh báo Telegram

Cập nhật mỗi 2 giây (5 frame pair), lấy trung bình trượt 3 lần đo.
```

### 3.2. Yêu cầu phi chức năng (Non-Functional Requirements)

| ID | Yêu cầu | Chỉ tiêu |
| :---: | :--- | :--- |
| NFR-01 | Thời lượng pin | ≥ 3 giờ hoạt động liên tục |
| NFR-02 | Tải trọng phễu thức ăn | ≥ 2 kg thức ăn viên |
| NFR-03 | Tốc độ di chuyển | 0.2 – 0.5 m/s (tốc độ thấp cho indoor) |
| NFR-04 | Phạm vi WiFi | Phủ toàn bộ trang trại (có repeater nếu cần) |
| NFR-05 | Khả năng chống ẩm | IP54 (chống bụi, chống nước bắn) |
| NFR-06 | Chiều rộng robot (khung xe lăn) | ≤ 65 cm (xe lăn gấp, vừa lối đi ≥ 80cm) |
| NFR-07 | Chiều cao tổng (tính cột mast) | ≥ 1.3m (nhìn được vào bể 1.2m) |
| NFR-08 | Chi phí GĐ1 | ≤ 10.000.000 VNĐ |
| NFR-09 | Độ phân giải Camera AI | ≥ 1080p, góc rộng ≥ 120° |
| NFR-10 | Tốc độ xử lý AI | ≥ 10 FPS inference |
| NFR-11 | Độ chính xác SLAM | ≥ 360° quét, ≥ 5.5m phạm vi |
| NFR-12 | Độ chính xác docking | ≤ 2cm sai lệch khi vào trạm sạc |
| NFR-13 | Thời gian phản hồi cảnh báo | ≤ 5 giây từ khi phát hiện đến gửi thông báo |

---

## 4. Kiến trúc hệ thống

### 4.1. Sơ đồ trạng thái (State Machine)

Robot vận hành dựa trên **máy trạng thái** để đảm bảo tính tự động hóa cao:

```
                    ┌─────────────────────────────────┐
                    │           IDLE (Chờ)             │
                    │  - Đậu tại trạm sạc             │
                    │  - Chờ lịch hoặc lệnh thủ công  │
                    └───┬───────┬────────────┬─────────┘
                        │       │            │
              Đến giờ   │       │ Lệnh       │ Pin < 20%
              tuần tra  │       │ thủ công   │
                        ▼       ▼            ▼
              ┌──────────────┐        ┌──────────────┐
              │ PATROL_TANKS │        │ GO_TO_CHARGER│
              │ (Tuần tra    │        │ (Về trạm sạc)│
              │  các bể)     │        └──────┬───────┘
              └──────┬───────┘               ▼
                     │               ┌──────────────┐
              Có lịch│               │  CHARGING    │
              check  │               │ (Đang sạc)   │
              RAS    │               │ Pin > 90%    │
                     ▼               │ → IDLE       │
              ┌──────────────┐       └──────────────┘
              │ MONITOR_RAS  │
              │ (Giám sát    │
              │  bể lọc)     │
              └──────┬───────┘
                     │
              Đến giờ│
              cho ăn │
                     ▼
              ┌──────────────┐       ┌──────────────┐
              │   FEEDING    │──────►│ GO_TO_REFILL │
              │ (Cho ăn      │ Hết   │ (Nạp thức ăn)│
              │  từng bể)    │ thức  └──────┬───────┘
              └──────┬───────┘ ăn            │
                     │                       ▼
              Hoàn   │               ┌──────────────┐
              thành  │               │  REFILLING   │
                     ▼               │ (Đang nạp)   │
                   IDLE              │ → FEEDING    │
                                     └──────────────┘
```

### 4.2. Sơ đồ khối phần cứng (Hardware Block Diagram)

```
┌────────────────────────────────────────────────────────────────────┐
│                      TẦNG ĐIỀU KHIỂN (BRAIN & LOW-LEVEL)            │
│  ┌───────────────────┐  USB/UART  ┌──────────────────────────────┐  │
│  │ 🍊 Orange Pi 5    │◄─────────►│    ESP32 (Vi điều khiển)     │  │
│  │   (Master Node)   │            │    (Low-level Slave)         │  │
│  │   - ROS2 Humble   │            │    - Nhận lệnh vận tốc (vel) │  │
│  │   - SLAM & Nav2   │            │    - PID Motor Control       │  │
│  │   - YOLOv8 (NPU)  │            │    - Servo camera/ống rải    │  │
│  │   - Web Dashboard │            │    - Solenoid phanh đĩa      │  │
│  │   - Camera USB/CSI│            │    - Safety Monitor          │  │
│  │   - Lidar Driver  │            │    - Docking sensor read     │  │
│  │   - odom_publisher │            │    - Đọc RPM Hall sensor     │  │
│  │                   │            │    - **MPU6050 (IMU 6-axis)**│  │
│  └─┬───────┬─────────┘            └──────────┬───────────────────┘  │
│    │ USB   │ USB/CSI                         │ UART (FOC cmd)       │
│  ┌─▼─────┐ ┌▼────────┐                     ┌─▼─────────┐            │
│  │RPLidar│ │Camera   │                     │Board      │            │
│  │A1     │ │Wide/CSI │                     │Hoverboard │            │
│  └───────┘ └─────────┘                     └───────────┘            │
├─────────────────────────────────────────────────────────────────────┤
│   TẦNG CHẤP HÀNH (Hoverboard + Khung tự chế + Phanh đĩa)            │
│     ┌──────────────────────────────────┼─────────────────┐          │
│     │           │          │           │         │       │          │
│ ┌───▼─────┐ ┌───▼──────┐   │         ┌▼──────┐ ┌▼────┐   │          │
│ │Board    │ │Motor rải │   │         │Phanh  │ │IR   │   │          │
│ │Hoverboard│ │thức ăn   │   │         │đĩa    │ │Dock │   │          │
│ │(STM32   │ │+Servo nắp│   │         │+Solo- │ │Sen- │   │          │
│ │FOC,UART)│ │phễu      │   │         │noid   │ │sor  │   │          │
│ │2x BLDC  │ │          │   │         │fail-  │ │     │   │          │
│ │350W,36V │ └──────────┘   │         │safe   │ └─────┘   │          │
│ │+Hall    │               │         └───────┘         │          │
│ └─────────┘               │                           │          │
├───────────────────────────┼───────────────────────────┼─────────────┤
│        TẦNG NGUỒN ĐIỆN    │                           │             │
│  Pin Hoverboard 36V ──────┴──► Buck 5V-10A ──► OPi5 + Lidar         │
│  (Li-ion ~4.4Ah,          ───► Buck 5V-5A  ──► ESP32 + Servo        │
│   nâng cấp 6-10Ah)        ───► Trực tiếp 36V ──► Board HB           │
│                                                                     │
│  [TRẠM SẠC] ──► Tiếp điểm đồng ──► Mạch sạc 42V                     │
│                  (tận dụng bộ sạc hoverboard sẵn có)                │
└─────────────────────────────────────────────────────────────────────┘

**[CẬP NHẬT v4.1] Luồng Odometry cho ROS2 Nav2:**

Nav2 yêu cầu topic `/odometry/filtered` (nav_msgs/Odometry) sau khi fuse IMU và Wheel Odom để giảm drift.

**Luồng dữ liệu Odometry Fusion (Issue 1 & 4):**
```
Board Hoverboard (STM32 FOC) → Hall Sensor RPM
  → [UART] → ESP32 (Đọc RPM + MPU6050 IMU @100Hz + Timestamp MS)
  → [USB Serial] → Orange Pi 5 
  → [odom_publisher_node] (Tính wheel odom dùng timestamp ESP32) → /odom
  → [imu_publisher_node] (Xử lý raw IMU) → /imu/data
  → [robot_localization EKF] (Fusion odom + imu) → /odometry/filtered
  → [Nav2 stack] (Dẫn đường tự động)
```

**[CẬP NHẬT v4.2] Sơ đồ luồng ROS2 Topic tích hợp ArUco (Issue 2):**
Cơ chế reset drift bằng landmark nhân tạo khi robot đi qua ArUco marker:

```
[Camera USB] → /image_raw → [aruco_detector_node]
                                    │
                                    ▼ /aruco_poses (geometry_msgs/PoseArray)
                                    │
                        [robot_localization EKF node]
               (Fuse: /odom + /imu/data + /aruco_poses correction)
                                    │
                                    ▼ /odometry/filtered
                           ┌────────┴────────┐
                           ▼                 ▼
                      [Nav2 Stack]     [slam_toolbox]
                  (Local Planner)    (Map correction)
```
- **Cơ chế:** Khi camera nhận dạng được marker ID, `aruco_detector_node` tính toán vị trí robot trong Map frame và gửi dữ liệu sửa lỗi (`initialpose` hoặc fusion pose) về EKF. Do marker có tọa độ cố định ("Anchor"), nó giúp triệt tiêu sai số tích lũy từ bánh xe và IMU.

---

**Công thức tính Odometry từ Hall Sensor RPM (Hoverboard 10"):**

| Thông số | Giá trị | Ghi chú |
| :--- | :--- | :--- |
| Đường kính bánh xe (D) | 254 mm (10 inch) | Hub motor hoverboard |
| Chu vi bánh xe (C) | π × 0.254 = **0.7980 m** | |
| Wheelbase (L) | **520 mm** (0.52 m) | Khoảng cách tâm 2 bánh chủ động |
| Hall sensor ticks/vòng | 90 ticks (30 pole pairs × 3 hall) | Firmware Feru trả về RPM đã tính |

```
# Từ RPM → vận tốc tuyến tính mỗi bánh:
V_left  = (L_RPM / 60) × C   # m/s
V_right = (R_RPM / 60) × C   # m/s

# Vận tốc robot (differential drive):
V_linear  = (V_right + V_left) / 2        # m/s
V_angular = (V_right - V_left) / L        # rad/s

# Tích phân vị trí (Euler integration, dt = khoảng thời gian giữa 2 mẫu):
theta += V_angular × dt
x     += V_linear × cos(theta) × dt
y     += V_linear × sin(theta) × dt

# Publish: nav_msgs/Odometry trên topic /odom
# Publish: TF transform odom → base_link
```

> **[CẬP NHẬT v4.1] Quyết định giao tiếp: Serial Packet (không dùng micro-ROS)**
>
> Sau khi đánh giá, dự án chọn **Serial Packet tự định nghĩa** (Section 7.4) thay vì micro-ROS vì:
> 1. **Đơn giản:** ESP32 chỉ là Slave chuyển tiếp lệnh, không cần chạy ROS2 stack phức tạp trên MCU.
> 2. **Debug dễ:** Có thể dùng Serial Monitor để xem raw packet, không cần ROS2 toolchain trên ESP32.
> 3. **Tài nguyên:** micro-ROS chiếm ~60% RAM ESP32, không đủ cho PID + safety monitor + servo control.
> 4. **Latency:** Serial packet 115200 baud đủ nhanh cho 50Hz odometry update (20ms/packet, mỗi packet ~20 bytes).
>
> Node `odom_publisher_node` trên OPi5 sẽ đọc Serial Packet từ ESP32, parse RPM, tính odometry và publish `/odom` + TF. Đây là cách tiếp cận phổ biến trong cộng đồng ROS2 cho các robot dùng MCU đơn giản.

### 4.3. Bộ xử lý trung tâm — Orange Pi 5 (8GB)

Sử dụng **Orange Pi 5 (phiên bản 8GB RAM)** làm bộ não trung tâm (SBC) thay thế hoàn toàn điện thoại Android.

| Đặc tính | Chi tiết kỹ thuật | Lợi ích cho SIFAR |
| :--- | :--- | :--- |
| **CPU** | Rockchip RK3588S (8-core 64-bit) | Hiệu năng vượt trội so với phone cũ, chạy ROS2 mượt mà |
| **NPU AI** | 6 TOPS (Neural Processing Unit) | Chạy YOLOv8s/m full size > 30 FPS mà không nóng CPU |
| **RAM** | 8GB LPDDR4x | Đủ bộ nhớ cho SLAM (Cartographer), Nav2 và AI chạy song song |
| **OS** | Ubuntu 22.04 LTS (Jammy Jellyfish) | Hỗ trợ **Native ROS2 Humble** (Standard Industry) |
| **I/O** | USB 3.0, USB 2.0, GPIO, CSI, HDMI | Kết nối trực tiếp Lidar USB (băng thông cao), Camera USB/CSI |
| **Độ bền** | Thiết kế bo mạch công nghiệp | Hoạt động 24/7 không bị phồng pin như điện thoại |

**Cách kết nối & Luồng dữ liệu:**
```
Orange Pi 5 (Ubuntu + ROS2)
  ├──[USB]───────► RPLidar A1 (Scan data 8000Hz, trực tiếp vào OS)
  ├──[USB/CSI]───► Camera góc rộng (Hình ảnh cho YOLOv8 Detect)
  ├──[UART/USB]──► ESP32 (Gửi lệnh vận tốc v/w, nhận Odom/IMU giả lập)
  ├──[HDMI]──────► Màn hình debug (tùy chọn) hoặc Web Dashboard qua WiFi
  └──[WiFi]──────► Cảnh báo Telegram + Remote Control
```

> ✅ **Lý do chọn Orange Pi 5:**
> 1.  **Giải quyết vấn đề nhiệt & pin:** Điện thoại chạy 24/7 rất dễ nóng và phồng pin gây cháy nổ. OPi5 dùng nguồn trực tiếp và tản nhiệt chủ động.
> 2.  **Băng thông Lidar:** Cắm Lidar vào USB của OPi5 đảm bảo tốc độ 8000 mẫu/giây ổn định. (Cách cũ qua ESP32 WiFi dễ bị rớt gói tin và trễ).
> 3.  **Hệ sinh thái ROS2:** Ubuntu + ROS2 là tiêu chuẩn vàng của robot tự hành. Dễ dàng sử dụng các package xịn như `nav2`, `slam_toolbox` thay vì tự viết lại SLAM bằng Python.

### 4.4. Luồng hoạt động chính

**Luồng A — Tuần tra bể nuôi (Patrol Mode):**
```
[Hẹn giờ / Lệnh thủ công]
       → [Robot rời trạm sạc, di chuyển tới dãy bể #1]
       → [Dừng tại bể #1, cột mast camera nhìn xuống bể]
       → [AI phân tích: cá bình thường? cá bệnh? cá chết?]
            ├─ Bình thường → Chuyển sang bể tiếp theo
            ├─ Phát hiện cá bệnh → Chụp ảnh, gửi cảnh báo Telegram
            └─ Phát hiện cá chết nổi → Cảnh báo KHẨN
       → [Lặp lại cho tất cả bể trong trang trại]
       → [Hoàn thành → Quay về IDLE hoặc chuyển sang FEEDING]
```

**Luồng B — Cho ăn thông minh (Feeding Mode):**
```
[Hẹn giờ cho ăn]
       → [Robot di chuyển tới bể #1]
       → [Dừng, căn chỉnh phễu hướng vào miệng bể]
       → [Rải thức ăn, Camera AI quan sát phản ứng cá]
            ├─ Cá ăn tích cực → Rải đủ lượng cài đặt
            ├─ Cá giảm ăn → Giảm lượng, ghi log
            └─ Cá bỏ ăn → Dừng rải, cảnh báo
       → [Chuyển sang bể tiếp theo] → Lặp lại
       → [Hết thức ăn → Chuyển sang GO_TO_REFILL]
       → [Hoàn thành tất cả bể → Quay về IDLE]
```

**Luồng C — Auto-Docking sạc pin (GĐ2):**
```
[Pin < 20%] → [Robot tạm dừng nhiệm vụ]
       → [SLAM dẫn đường tới khu vực trạm sạc]
       → [Cảm biến IR căn chỉnh chính xác ± 2cm]
       → [Lùi vào trạm, tiếp điểm đồng kết nối]
       → [Sạc đến > 90% → Resume nhiệm vụ hoặc IDLE]
```

---

## 5. Thiết kế cơ khí

### 5.1. Cấu trúc khung robot — Hoverboard + Khung tự chế + Caster + Phanh đĩa

Robot SIFAR sử dụng **motor brushless từ hoverboard cũ** làm hệ dẫn động, kết hợp **khung tự chế** + **2 bánh caster** + **phanh đĩa fail-safe**:

```
         [Caster trước #1]     [Caster trước #2]
              ●                      ●
              │                      │
    ┌─────────┴──────────────────────┴─────────┐
    │             KHUNG TỰ CHẾ (nhôm/thép)      │
    │              ~60cm × 70cm                  │
    │                                            │
    │  ┌─Phễu─┐  ┌─ESP32─┐  ┌─Lidar A1─┐       │
    │  │      │  │       │  │         │       │
    │  └──────┘  └───────┘  └─────────┘       │
    │                                            │
    │    ══════════════════════════               │
    │    ║  HOVERBOARD (trung tâm)  ║            │
    │    ║  ●━━━━━━━━━━━━━━━━━━━●  ║            │
    │    ║ Bánh L           Bánh R  ║            │
    │    ║ [Đĩa phanh]  [Đĩa phanh]║            │
    │    ══════════════════════════               │
    │              │                              │
    └──────────────┼──────────────────────────────┘
                   │ Cột Mast nhôm 2020
                   │ (~1.0m)
              ┌────┴────┐
              │ 🍊 OPi5 │ ← Brain (ROS2 + AI)
              │+Camera  │
              └─────────┘
```

**Phần TẬN DỤNG từ hoverboard cũ:**
- 2× Motor BLDC 350W (36V) tích hợp trong bánh 10" (hub motor) — yên tĩnh, bền, torque dư cho 30kg.
- Board điều khiển STM32F103 — flash firmware Emanuel Feru (FOC) → điều khiển UART từ ESP32.
- Hall sensor (3 pha/motor) — tích hợp sẵn trong hub motor.
- Pin Li-ion 36V (~4.4Ah = 158Wh) + bộ sạc 42V sẵn có.

**Phần BỎ ĐI:**
- Vỏ nhựa hoverboard, đế đứng, cảm biến nghiêng tự cân bằng.
- Firmware gốc (thay bằng firmware Emanuel Feru FOC).

**Phần TỰ CHẾ / THÊM MỚI:**
- **Khung nhôm/thép** (~60×70cm) gắn hoverboard ở vị trí trung tâm.
- **2× Bánh caster** Φ75–100mm phía trước → tạo 4 điểm tiếp đất, ổn định tĩnh.
- **Phanh đĩa fail-safe** (×1 hoặc ×2): caliper cơ xe đạp + đĩa Φ120mm gắn trục hub motor + solenoid kéo cáp (lò xo hồi → mất điện = phanh tự đóng).
- Cột mast nhôm 2020 cao ~1.0m (tổng ~1.35m từ mặt đất).
- Orange Pi 5 + Camera USB/CSI + servo xoay trên đỉnh mast.
- RPLidar A1 gắn trên khung (~25cm).
- Phễu + ống rải thức ăn.
- ESP32 (giao tiếp UART với board hoverboard, điều khiển servo/solenoid).

| Thông số khung | Giá trị |
| :--- | :--- |
| **Kích thước đế** | ~60cm (rộng) × 70cm (dài) × 20cm (cao đế) |
| **Chiều cao tổng** | ~1.35m (có cột mast + OPi5) |
| **Trọng lượng robot** | ~20–25kg (nhẹ hơn xe lăn 10–15kg) |
| **Tải trọng an toàn** | ≤ 30kg (motor thiết kế cho 100kg người) |
| **Cấu hình bánh xe** | 2 bánh chủ động BLDC 10" (giữa) + 2 caster (trước) |
| **Khả năng quay** | Quay tại chỗ 360° (differential drive) |
| **Phanh an toàn** | Phanh đĩa fail-safe (lò xo đóng khi mất điện) |

### 5.1.1. Cơ cấu phanh đĩa fail-safe

Hoverboard không có phanh cơ khí → tự chế phanh fail-safe theo nguyên lý **"lò xo đóng — solenoid mở"**:

- **Có điện (robot chạy):** Solenoid 12V kéo cáp phanh → nhả phanh → bánh quay tự do.
- **Mất điện (an toàn):** Lò xo hồi trong solenoid đẩy → cáp phanh thắt → caliper ép đĩa → bánh KHÓA.

| Linh kiện phanh | Mô tả |
| :--- | :--- |
| Caliper phanh đĩa cơ xe đạp | Loại kéo cáp, Shopee ~80–100k |
| Đĩa phanh Φ120mm | Gắn vào trục hub motor |
| Solenoid kéo 12V (pull type, có lò xo hồi) | Hành trình ~10–15mm, ~50–80k |
| Bracket gắn caliper | Tự chế nhôm L |

**Tại sao dùng Hoverboard?**
> Motor brushless bền gấp đôi motor brushed (10,000+ giờ), yên tĩnh hơn nhiều cho indoor farm, firmware Emanuel Feru cung cấp FOC + PID sẵn → ESP32 chỉ gửi lệnh tốc độ qua UART (đơn giản hơn tự viết PID cho BTS7960). Robot nhẹ hơn xe lăn ~15kg. Chi phí tương đương (~1.6–3 triệu tổng cộng hoverboard + khung + caster + phanh).

### 5.1.2. [CẬP NHẬT v4.1] Checklist kiểm tra board Hoverboard TRƯỚC khi mua

| Bước | Kiểm tra | Cách thực hiện | Đạt/Không |
| :---: | :--- | :--- | :---: |
| 1 | **Nhận dạng chip chính** | Mở nắp board, đọc marking trên MCU: `STM32F103RCT6` = OK. Nếu thấy `GD32F103` hoặc `CKS32` = **chip clone, rủi ro cao** | ✅/❌ |
| 2 | **Kiểm tra pinout UART** | Tìm 2 pad/header GND + TX + RX trên board. Firmware Feru cần UART để nhận lệnh | ✅/❌ |
| 3 | **Đọc chip qua ST-Link** | Cắm ST-Link V2, dùng STM32CubeProgrammer → "Connect". Nếu đọc được chip ID = flash được | ✅/❌ |
| 4 | **Kiểm tra Hall sensor** | Quay tay từng bánh, đo tín hiệu Hall bằng oscilloscope/logic analyzer (3 kênh/motor) | ✅/❌ |
| 5 | **Pin + BMS** | Đo điện áp pin ≥ 33V (10S đầy ~42V). Kiểm tra BMS có led báo | ✅/❌ |

> ⚠️ **Nếu bước 1 hoặc 3 KHÔNG ĐẠT → Kích hoạt Plan B ngay, không cố flash.**

### 5.1.3. [CẬP NHẬT v4.1] Plan B — Nếu flash firmware thất bại

**Khi nào kích hoạt:** Board hoverboard dùng chip clone (GD32/CKS32), ST-Link không nhận, hoặc flash firmware Feru thành công nhưng motor không quay đúng.

**Phương án thay thế:**

| Phương án | Mô tả | Chi phí thêm | Thời gian thêm |
| :--- | :--- | :---: | :---: |
| **ODrive v3.6 (clone)** | Board điều khiển FOC chuyên dụng, hỗ trợ 2 motor BLDC. Giao tiếp UART/USB trực tiếp, không cần firmware hack | ~500–800k | ~3–5 ngày |
| **SimpleFOC (Arduino)** | Thư viện FOC mã nguồn mở chạy trên ESP32 + driver MOSFET riêng (L6234 hoặc DRV8302). Tự chế driver board | ~300–500k | ~5–7 ngày |

**Quy trình Plan B:**
1. Tháo board hoverboard gốc, giữ lại motor BLDC + Hall sensor + pin.
2. Nối motor trực tiếp vào ODrive/SimpleFOC driver.
3. Cấu hình encoder = Hall sensor (3 kênh), calibrate motor.
4. ESP32 giao tiếp với ODrive qua UART (protocol tương tự, chỉ đổi command format).
5. Firmware ESP32 cần sửa: thay hàm `sendHoverboardCmd()` bằng `sendODriveCmd()`.

> 💡 **Lưu ý:** Plan B vẫn sử dụng cùng motor BLDC và pin 36V, chỉ thay board điều khiển. Phần còn lại của kiến trúc (khung, caster, phanh, ESP32, OPi5) không thay đổi.

### 5.2. Cột Mast Camera

- **Chiều cao:** ~1.1m tính từ đế robot (tổng ~1.35m từ mặt đất).
- **Vật liệu:** Ống nhôm φ25mm hoặc thanh nhôm 2020, cố định bằng ke góc.
- **Đỉnh cột:** Đế xoay (servo nhỏ) gắn **Camera USB/CSI** (hướng xuống bể), cho phép xoay 180° để nhìn bể ở cả 2 bên lối đi. Orange Pi 5 được đặt gần camera hoặc ở đế robot.

**[CẬP NHẬT v4.1] Tính toán moment lật cột mast khi phanh đĩa đóng đột ngột:**

| Thông số | Giá trị | Ghi chú |
| :--- | :--- | :--- |
| Khối lượng đỉnh mast (OPi5 + Camera + Servo) | ~500g (0.5 kg) | |
| Chiều cao trọng tâm tải đỉnh | 1.35m từ mặt đất | |
| Gia tốc phanh (worst case) | 1–2 m/s² | Từ 0.5 m/s về 0 trong 0.25–0.5s |
| **Moment lật tại đế cột** | **M = 0.5 × 1.35 × 2 = 1.35 N·m (max)** | F = m × a, M = F × h |
| Lực ngang tại ke góc nhôm 2020 | F = M / h_ke = 1.35 / 0.2 ≈ **6.75 N** | h_ke = 0.2m (chiều cao ke góc) |
| Torque siết ốc M5 tối thiểu | **≥ 4 N·m** | Ke góc nhôm 2020 + ốc M5 inox |

**Biện pháp gia cố:**
1. **Ke góc kép:** Dùng 2 ke góc nhôm 2020 đối xứng 2 bên tại chân cột mast.
2. **Thanh giằng chéo thứ 2:** Từ điểm 2/3 cột mast (~0.7m) xuống khung đế, tạo tam giác chống xoay. Vật liệu: thanh nhôm 2020 hoặc ống nhôm φ15mm.
3. **Guy wire (tùy chọn):** 2 dây thép inox φ1.5mm từ điểm 3/4 cột mast (~0.8m) neo xuống 2 điểm trên khung đế, tạo hình chữ V ngược. Tăng đơ M4 để căng dây.

> ✅ **Kết luận:** Moment lật 1.35 N·m là nhỏ (tương đương lực 675g ở đầu cần 20cm). Ke góc kép + thanh giằng chéo là đủ. Guy wire chỉ cần nếu nền nhà không phẳng gây rung lắc thêm.

### 5.3. Cơ cấu rải thức ăn (cho bể nhỏ)

- **Phễu:** Dung tích 2–3 kg, miệng rộng 15cm, cổ hẹp 5cm.
- **Cơ chế rải:** Motor DC nhỏ gắn trục vít (auger screw) tại cổ phễu.
- **Ống dẫn linh hoạt:** Thức ăn từ phễu đi qua ống nhựa mềm dài ~30cm, hướng vào miệng bể. Đầu ống có thể xoay/nghiêng bằng servo nhỏ.
- **Nắp phễu tự động (GĐ2):** Servo mở nắp phễu khi ở trạm nạp thức ăn, đóng lại khi di chuyển.

**[CẬP NHẬT v4.1] Thiết kế chống kẹt phễu & Recovery:**

**A. Thiết kế cơ khí chống kẹt:**

| Thông số | Yêu cầu | Lý do |
| :--- | :--- | :--- |
| Góc nghiêng phễu | **≥ 45°** | Thức ăn viên 3–5mm tự trượt xuống, không đọng |
| Vật liệu trong phễu | **Inox 304** (hoặc nhựa HDPE) | Không thấm nước, chống ẩm >80%, dễ vệ sinh |
| Đường kính auger screw | **≥ 20mm** | Đủ lớn cho thức ăn viên 3–5mm, giảm kẹt |
| Khe hở auger–ống | **2–3mm** | Tránh nghiền nát thức ăn thành bột |

**B. Cảm biến phát hiện kẹt:**
- **Phương pháp chính:** Đo dòng điện motor rải qua module ACS712 (5A) trên chân ADC ESP32.
- **Ngưỡng phát hiện:** Dòng bình thường ~0.3–0.5A. Nếu **dòng > 1.0A liên tục > 500ms** → kẹt cơ học.
- **Phương pháp phụ (tùy chọn):** Encoder nhỏ (optical encoder disc) trên trục auger. Nếu motor chạy mà encoder không đếm tick → kẹt.

**C. Recovery Procedure (Firmware ESP32):**
```
Khi phát hiện kẹt (dòng > 1.0A > 500ms):
  0. [CẬP NHẬT v4.2] Xoay servo hướng ống rải về vị trí NEUTRAL (90° — hướng thẳng xuống)
     để tránh văng thức ăn vào bể lân cận khi đảo chiều. Delay 500ms để servo hoàn tất.
  1. Dừng motor rải
  2. Đảo chiều motor 2 giây (xoay ngược auger để nới thức ăn)
  3. Dừng 1 giây
  4. Chạy lại chiều thuận, đo dòng
  5. Nếu vẫn kẹt (dòng > 1.0A):
     → Đảo chiều lần 2 (3 giây) 
  6. Nếu vẫn kẹt sau 2 lần retry:
     → Dừng motor rải hoàn toàn
     → Gửi cảnh báo Telegram: "⚠️ FEEDER JAMMED - Tank #X"
     → Set flag feeder_jammed = true
     → Robot bỏ qua bể hiện tại, tiếp tục bể tiếp theo
```

### 5.4. Vị trí lắp đặt cảm biến & camera

| Thiết bị | Vị trí | Vai trò |
| :--- | :--- | :--- |
| **🍊 Orange Pi 5** | Trên khung robot hoặc gần đỉnh cột mast | Brain (ROS2 + AI NPU) |
| **Camera USB/CSI** | Đỉnh cột mast (~1.35m), trên đế xoay servo | Camera AI nhìn xuống bể |
| **RPLidar A1** | Trên khung (~25cm), trung tâm | Quét 360° lập bản đồ trang trại, tránh vật cản |
| **Phanh đĩa + Solenoid** | Gắn caliper lên bracket cạnh hub motor | Fail-safe brake: mất điện = phanh tự đóng |
| **Cảm biến IR docking (×2)** | Phía sau robot, 2 bên | Căn chỉnh chính xác khi lùi vào trạm sạc (GĐ2) |
| **Cảm biến mức thức ăn** | Trong phễu | Phát hiện khi gần hết thức ăn (GĐ2) |

### 5.5. Trạm sạc & Trạm nạp thức ăn (GĐ2)

**Trạm sạc:**
- Tấm tiếp điểm bằng đồng gắn cố định trên tường/chân tường.
- Robot có 2 tiếp điểm phía sau, lùi vào và tiếp xúc.
- Beacon IR trên trạm → Cảm biến IR trên robot giúp căn chỉnh ± 2cm.

**Trạm nạp thức ăn:**
- Phễu lớn cố định trên cao (~1.5m), có van điện solenoid.
- Robot dừng dưới phễu, mở nắp phễu on-board, mở van trạm → Thức ăn rơi vào phễu robot.

---

## 6. Thiết kế điện – điện tử

### 6.1. Sơ đồ kết nối (Wiring Diagram)

```
Pin Hoverboard 36V ────┬──► Board Hoverboard STM32 (firmware Feru FOC)
  (Li-ion, sạc 42V)    │     ├──► Motor BLDC trái (36V, hall sensor)
                        │     └──► Motor BLDC phải (36V, hall sensor)
                        │
                        ├──► Mạch Buck 12V ──► Solenoid phanh đĩa (×1 hoặc ×2)
                        │                      └──► Motor rải thức ăn (+ ACS712 đo dòng) [v4.1]
                        │
                        ├──► Mạch Buck 5V-10A ──► ORANGE PI 5 (Nguồn chính)
                        │   [v4.1: XL4016/MP2307]   └──╸ 2×1000µF 10V tổng kháng tại OPi5
                        │                         └──► RPLidar A1 (qua USB OPi5)
                        │                         └──► Camera (qua USB OPi5)
                        │
                         ├──► Mạch Buck 5V-5A  ──► ESP32 (3.3V onboard)
                         │                         ├──► Servo #1 (Camera Mast)
                         │                         ├──► Servo #2 (Ống rải)
                         │                         ├──► **MPU6050 (IMU - I2C)** [v4.2]
                         │                         └──► Cảm biến IR docking (GĐ2)
                         │
ORANGE PI 5 ──[USB Cable]──► RPLidar A1 (Dữ liệu scan tốc độ cao)
ORANGE PI 5 ──[USB/UART]──► ESP32 (Gửi CMD_VEL, nhận RPM+IMU+sensor) [v4.2]
ESP32 ──[UART TX/RX]──► Board Hoverboard (gửi speed_left, speed_right)
```

### 6.2. Bảng phân bổ chân GPIO ESP32

**(ESP32 chỉ đóng vai trò Slave Controller)**

| Chân ESP32 | Kết nối tới | Chức năng |
| :--- | :--- | :--- |
| GPIO 16 (TX2) | Board Hoverboard (RX) | UART gửi lệnh tốc độ motor |
| GPIO 17 (RX2) | Board Hoverboard (TX) | UART nhận phản hồi (pin, dòng, nhiệt, **L_RPM, R_RPM**) **[CẬP NHẬT v4.1]** |
| GPIO 23 | MOSFET gate (H-bridge) | Motor rải thức ăn (12V) — **[CẬP NHẬT v4.1]** H-bridge cho đảo chiều |
| GPIO 22 | MOSFET gate (H-bridge DIR) | **[CẬP NHẬT v4.1]** Chiều quay motor rải (LOW=thuận, HIGH=đảo chiều recovery) |
| GPIO 18 | Servo #1 (PWM) | Xoay camera trên mast |
| GPIO 19 | Servo #2 (PWM) | Hướng ống rải thức ăn |
| GPIO 27 | MOSFET gate solenoid | Điều khiển phanh đĩa (HIGH = nhả phanh) |
| GPIO 4 | Voltage divider | Đo điện áp pin 36V (Dự phòng) |
| GPIO 34 (ADC) | ACS712 (5A) output | **[CẬP NHẬT v4.1]** Đo dòng motor rải thức ăn — phát hiện kẹt phễu |
| GPIO 32, 33 | IR Sensor #1, #2 (GĐ2) | Căn chỉnh docking |
| GPIO 21 (SDA) | MPU6050 IMU | **[CẬP NHẬT v4.2]** Dữ liệu IMU cho EKF Fusion |
| GPIO 22 (SCL) | MPU6050 IMU | **[CẬP NHẬT v4.2]** Dữ liệu IMU cho EKF Fusion |
| GPIO 35 (ADC) | Cảm biến mức thức ăn (GĐ2) | **[CẬP NHẬT v4.2]** Đổi từ 21 sang 35 để lấy I2C pin |
| **USB/UART** | Orange Pi 5 | Nhận lệnh điều khiển từ OPi5, **[CẬP NHẬT v4.2]** gửi RPM + IMU data |

> 📝 **Lưu ý:** Đã **loại bỏ** kết nối UART với Lidar trên ESP32 vì Lidar giờ cắm trực tiếp vào Orange Pi 5.

### 6.3. Tính toán nguồn điện

Do thay đổi sang Orange Pi 5 (ngốn dòng hơn điện thoại) và Lidar chạy qua OPi, cần bổ sung nguồn riêng:

| Thiết bị | Điện áp | Dòng (max) | Công suất max | Ghi chú |
| :--- | :---: | :---: | :---: | :--- |
| **Orange Pi 5 + Fan** | 5V | **4.5A (đột biến NPU full)** | **22.5W** | **[CẬP NHẬT v4.1]** NPU full load hút 4–5A transient |
| **RPLidar A1** (qua OPi) | 5V | 0.5A | 2.5W | Khởi động motor quét cần dòng cao |
| **Camera USB** | 5V | 0.5A | 2.5W | |
| **Hệ thống servo/ESP32** | 5V | 2.0A | 10W | Nuôi 2 servo, ESP32 |
| **Tổng tải 5V** | | **~7.5A** | **37.5W** | **[CẬP NHẬT v4.1]** Tăng do transient load |
| 2x Motor BLDC (Board) | 36V | 2A | 72W | Tải nhẹ |
| Solenoid + Motor rải | 12V | 2A | 24W | Dùng Buck 12V-3A riêng |

> ⚡ **YÊU CẦU QUAN TRỌNG:** Phải sử dụng Mạch Buck 5V dòng tối thiểu **5A** (tốt nhất là **10A**) cấp riêng cho cụm Orange Pi 5 + Lidar + Camera để tránh sụt áp khi khởi động hoặc khi AI chạy full tải (NPU). ESP32 và Servo nên dùng chung nguồn 5V nhưng có tụ lọc lớn hoặc Buck riêng nếu có thể để tránh nhiễu servo làm treo MCU.

**[CẬP NHẬT v4.1] Yêu cầu kỹ thuật mạch Buck 5V cho OPi5:**

| Thông số | Yêu cầu tối thiểu | Lý do |
| :--- | :--- | :--- |
| **Ripple voltage** | ≤ 50mV peak-to-peak | OPi5 nhạy với ripple, gây crash/reboot nếu >100mV |
| **Transient response** | ≤ 100mV khi load step 0→3A | NPU chuyển từ idle sang full load gây đột biến dòng |
| **Hiệu suất** | ≥ 85% tại 3A output | Giảm nhiệt, kéo dài pin |
| **Dòng output max** | ≥ 10A | Headroom cho OPi5 (4.5A) + Lidar (0.5A) + Camera (0.5A) |

**Module Buck đề xuất (thay thế module generic 150k):**

| Module | Chip | Dòng max | Giá (ước) | Ghi chú |
| :--- | :--- | :---: | :---: | :--- |
| **XL4016 based** | XL4016E1 | 8A (12A peak) | ~200–250k | Được recommend, ripple thấp, điện áp input 8–40V |
| **MP2307 based** | MP2307DN | 3A (5A peak) | ~150–200k | Nhỏ gọn, hiệu suất 95%, nhưng dòng hơi thấp — cần 2 module song song |
| **LM2596S (không khuyến nghị)** | LM2596 | 3A | ~80–100k | Ripple cao ~200mV, transient kém — **KHÔNG DÙNG cho OPi5** |

**Tụ lọc đầu ra (bắt buộc):**
- **2× tụ điện phân 1000µF 10V** mắc song song ngay tại chân nguồn OPi5 (dây ngắn nhất có thể).
- **1× tụ gốm 100nF** song song để lọc nhiễu cao tần.
- Mục đích: xử lý transient load khi NPU chuyển từ idle → full load (đột biến 0→4.5A trong <1ms). Tụ cung cấp dòng tức thời trong khi Buck chưa kịp phản hồi.

**Thời lượng pin:**
- Pin hoverboard gốc ~4.4Ah: 36V × 4.4Ah = 158Wh ÷ 37.5W ≈ **~4 giờ** (tải thực tế).
- Nâng cấp pin 6Ah: 36V × 6Ah = 216Wh ÷ 37.5W ≈ **~5.7 giờ**.
- Nâng cấp pin 10Ah: 36V × 10Ah = 360Wh ÷ 37.5W ≈ **~9.6 giờ**.
- Với auto-docking (GĐ2): hoạt động **liên tục 24/7**.

**[CẬP NHẬT v4.2] Ghi chú cân chỉnh (Calibration):**
1. **Wheelbase L:** Sau khi lắp khung, dùng thước đo chính xác khoảng cách giữa tâm 2 bánh chủ động. Tuyệt đối không dùng giá trị lý thuyết 520mm để tránh sai số quay (yaw drift).
2. **Wheel Radius R:** Điều khiển robot đi thẳng 2m trên sàn trang trại. Đo quãng đường thực tế và điều chỉnh hệ số Wheel Radius trong code để sai số < 1%.
3. **IMU Bias:** Khi robot khởi động, giữ đứng yên 5 giây để ESP32 lấy giá trị trung bình Gyro Offset.
4. **IMU Orientation:** Đảm bảo trục X của MPU6050 trùng với hướng tiến của robot.

---

## 7. Thiết kế phần mềm

### 7.1. Firmware ESP32 (Arduino C++) — Low Level Control
ESP32 đóng vai trò **Slave**, thực hiện các lệnh từ Orange Pi 5:
- Nhận lệnh vận tốc (`cmd_vel`) từ ROS2 (qua UART/USB Serial).
- Gửi lệnh điều khiển xuống Board Hoverboard.
- Điều khiển Servo, Solenoid, Motor rải.
- Gửi dữ liệu cảm biến (Pin, IR, trạng thái phanh) ngược lại OPi5.

**[CẬP NHẬT v4.2] Chức năng bổ sung trong Firmware ESP32:**

1. **Đọc MPU6050 IMU:** Đọc dữ liệu 6 trục (accel, gyro) qua I2C mỗi 10ms (100Hz). Dùng thư viện `Adafruit_MPU6050`.
2. **Quản lý Timestamp:** Mỗi packet gửi lên OPi5 được đính kèm `TIMESTAMP_MS` (lấy từ `millis()`). Odom publisher dùng giá trị này để tính `dt` chính xác, triệt tiêu jitter do hệ điều hành Linux. Xử lý rollover `millis()` được thực hiện bằng cách so sánh hiệu số unsinged long.
3. **Phát hiện & Recovery kẹt phễu (Phát triển):** 
   - Trước khi đảo chiều, xoay servo 90° (Neutral).
   - Đọc ADC ACS712 để kiểm tra dòng motor rải.
4. **Tần suất gửi data:** IMU (100Hz), RPM Odometry (50Hz), Sensor data (10Hz).

### 7.2. Phần mềm trên Orange Pi 5 (Ubuntu + ROS2)

Hệ thống chạy trên nền tảng **ROS2 Humble**, tận dụng sức mạnh cộng đồng và tính ổn định công nghiệp.

**Kiến trúc ROS2 Nodes:**

| Node Name | Chức năng | Package sử dụng |
| :--- | :--- | :--- |
| **[CẬP NHẬT v4.1]** `odom_publisher_node` | Đọc Serial Packet từ ESP32, tính odometry, publish `/odom`. **[v4.2]** Dùng ESP32 timestamp. | Tự viết (Python) |
| **[CẬP NHẬT v4.2]** `imu_publisher_node` | Parse packet IMU `<IMU, ...>`, publish `sensor_msgs/Imu` lên `/imu/data` | Tự viết (Python) |
| **[CẬP NHẬT v4.2]** `ekf_node` | **Robot Localization:** Fuse `/odom` và `/imu/data` → `/odometry/filtered` | `robot_localization` |
| `serial_bridge_node` | Giao tiếp Serial với ESP32, publish `/cmd_vel` xuống | Tự viết (Python) |
| `sllidar_node` | Driver cho RPLidar A1, publish `/scan` | `sllidar_ros2` |
| `usb_cam_node` | Driver cho Camera, publish `/image_raw` | `usb_cam` |
| `slam_toolbox` | Thuật toán SLAM, tạo bản đồ `/map` | `slam_toolbox` |
| `nav2_bringup` | Dẫn đường tự động, tránh vật cản (yêu cầu `/odometry/filtered`) | `nav2` |
| `yolo_detect_node` | AI Vision nhận diện cá/bọt khí (dùng NPU RKNN) | `rknn_yolo_python` |
| **[CẬP NHẬT v4.1]** `feeding_analyzer_node` | Optical flow phân tích hành vi ăn mồi (Section 3.1) | Tự viết (Python) |
| **[CẬP NHẬT v4.1]** `aruco_detector_node` | Phát hiện ArUco marker. **[v4.2]** Publish `/aruco_poses` về EKF correction | `ros2_aruco` |
| `sifar_brain_node` | Logic chính: Tuần tra, Cho ăn, Xử lý sự kiện | Tự viết (Python) |

**AI Vision (YOLOv8 + Rockchip NPU):**
Thay vì chạy trên CPU/GPU yếu, chúng ta convert model YOLOv8 sang định dạng `.rknn` để chạy trên **NPU 6 TOPS** của RK3588S.
- Tốc độ dự kiến: 30–60 FPS (so với 5–10 FPS của Android cũ).
- Độ trễ cực thấp, không ảnh hưởng đến CPU chạy SLAM.

**[CẬP NHẬT v4.2] Kế hoạch Dataset thực tế (Bootstrap & Fine-tune):**
Do công sức label lớn (~3000 ảnh), việc thu thập được chia làm 2 giai đoạn:

1. **GĐ Bootstrap (Tuần 1–4):** Tìm kiếm và label ~300 ảnh từ public datasets (Roboflow Universe - keywords: "fish aquaculture top-down"). Mục tiêu: Đủ để train model chạy thử nghiệm và demo flow AI (Accuracy ≥50%).
2. **GĐ Fine-tune (Tuần 9–12):** Robot đã hoạt động tuần tra → tự động chụp ảnh mỗi 10s khi đi qua các bể. Thu thập ~1000 ảnh thực tế tại trang trại. Label 50 ảnh/ngày (1 giờ/ngày). Model sau giai đoạn này đạt Production-ready (Accuracy ≥80%).

**[CẬP NHẬT v4.2] Cấu hình EKF (robot_localization):**
Tạo file `ekf.yaml` để tối ưu hóa việc dẫn đường:
- `odom0`: `/odom` (fuse: x, y, yaw)
- `imu0`: `/imu/data` (fuse: yaw rate, linear acceleration)
- `frequency`: 50Hz
- `two_d_mode`: true
- `publish_tf`: true (map → odom correction bằng ArUco)

**[CẬP NHẬT v4.1] Chiến lược SLAM cho môi trường bể lặp lại (Issue 4):**

Bể nuôi có hình dạng lặp lại hoàn toàn (bể #1 = bể #2 = ... = bể #50), gây nguy cơ **perceptual aliasing** — slam_toolbox nhầm lẫn vị trí vì các bể trông giống hệt nhau trên bản đồ Lidar.

**A. Landmark nhân tạo — ArUco Marker:**
- Đặt **ít nhất 4 ArUco marker (6×6, kích thước 15×15cm)** tại 4 góc trang trại.
- Thêm marker tại mỗi đầu dãy bể (mỗi 5–10 bể đặt 1 marker trên tường/cột).
- `aruco_detector_node` phát hiện marker qua camera → cung cấp **absolute position correction** cho SLAM.
- Marker phải có ID duy nhất, in trên vật liệu chống nước (nhựa PP/acrylic).

**B. Cấu hình slam_toolbox:**

| Tham số | Giá trị | Lý do |
| :--- | :--- | :--- |
| **Mode** | `localization` (khi bản đồ đã có) / `mapping` (lần đầu) | Không dùng `lifelong` — tránh bản đồ bị corrupted bởi aliasing |
| `loop_search_maximum_distance` | **3.0m** (giảm từ default 10m) | Giới hạn phạm vi tìm loop closure, tránh match nhầm bể xa |
| `loop_match_minimum_response_coarse` | **0.7** (tăng từ default 0.35) | Yêu cầu match score cao hơn, giảm false loop closure |
| `loop_match_minimum_response_fine` | **0.7** (tăng từ default 0.45) | Tương tự |
| `minimum_travel_distance` | **0.5m** | Chỉ thêm scan mới khi đã di chuyển đủ xa |
| `do_loop_closing` | **false** (khi dùng ArUco) | Tắt loop closure tự động của slam_toolbox, thay bằng ArUco correction |

**C. Quy trình Mapping lần đầu:**
1. Đặt ArUco marker tại 4 góc + đầu mỗi dãy bể.
2. Điều khiển robot thủ công đi hết trang trại 1 lần (chế độ `mapping`).
3. Lưu bản đồ (`ros2 service call /slam_toolbox/serialize_map`).
4. Chuyển sang chế độ `localization` cho vận hành hàng ngày.

### 7.3. Triết lý phát triển
- **Standardization:** Dùng chuẩn ROS2 (Robot Operating System 2). Dễ dàng tích hợp code của người khác, dễ debug, dễ nâng cấp.
- **Hardware Acceleration:** Tận dụng tối đa phần cứng (NPU, GPU) của Orange Pi 5.
- **Modular:** Tách biệt rõ ràng Low-level (ESP32) và High-level (OPi5).

### 7.4. Giao thức giao tiếp (Serial Packet)
Giao tiếp giữa OPi5 và ESP32 qua cổng USB Serial (baudrate 115200).
### 7.4. Giao thức giao tiếp (Serial Packet)
Giao tiếp giữa OPi5 và ESP32 qua cổng USB Serial (baudrate 115200).
- **OPi5 gửi:** `<CMD, VAL1, VAL2>` (Ví dụ: `<VEL, 0.2, 0.1>` cho vận tốc dài 0.2m/s, góc 0.1rad/s).
- **ESP32 gửi (v4.2):** 
  - `<FB, TIMESTAMP_MS, BAT_V, L_RPM, R_RPM, FEEDER_I>` (Phản hồi pin, tốc độ bánh trái/phải, dòng motor rải).
  - `<IMU, TIMESTAMP_MS, AX, AY, AZ, GX, GY, GZ>` (Dữ liệu gia tốc và vận tốc góc raw).

---

## 8. Danh sách vật tư & Dự toán (BOM)

### Giai đoạn 1 — MVP (Smart Robot với Orange Pi 5)

| STT | Linh kiện | SL | Đơn giá (VNĐ) | Thành tiền | Vai trò |
| :---: | :--- | :---: | ---: | ---: | :--- |
| 1 | **Orange Pi 5 (8GB RAM)** | 1 | 2.500.000 | 2.500.000 | Brain (ROS2 + AI NPU) |
| 2 | **Tản nhiệt + Quạt cho OPi5** | 1 | 150.000 | 150.000 | Làm mát CPU/NPU |
| 3 | Camera USB góc rộng (120°) | 1 | 300.000 | 300.000 | Vision AI |
| 4 | **Thẻ nhớ MicroSD 64GB Class 10** | 1 | 150.000 | 150.000 | Cài OS Ubuntu |
| 5 | Hoverboard cũ 10" (motor, board, pin) | 1 | 1.500.000 | 1.500.000 | Hệ dẫn động |
| 6 | ST-Link V2 | 1 | 100.000 | 100.000 | Flash firmware |
| 7 | ESP32 Dev Kit V1 | 1 | 150.000 | 150.000 | Low-level Controller |
| 8 | RPLidar A1 (Slamtec) | 1 | 2.800.000 | 2.800.000 | SLAM & Avoidance |
| 9 | Mạch Buck DC-DC **5V 10A (XL4016)** | 1 | **250.000** | **250.000** | **[CẬP NHẬT v4.1]** Nguồn cho OPi5 & Lidar (thay module generic) |
| 10 | Servo SG90 | 2 | 50.000 | 100.000 | Xoay cam, ống rải |
| 11 | Nhôm 2020 + Khung tự chế + Caster | 1 bộ | 1.000.000 | 1.000.000 | Cơ khí robot |
| 12 | Hệ thống phanh (Caliper + Solenoid) | 1 bộ | 300.000 | 300.000 | Phanh Fail-safe |
| 13 | Hệ thống rải (Motor + Phễu **inox 304**) | 1 bộ | **900.000** | **900.000** | **[CẬP NHẬT v4.1]** Bộ cho ăn (phễu inox thay nhựa) |
| 14 | Mạch Buck 12V + MOSFET + Dây | 1 bộ | 300.000 | 300.000 | Phụ kiện điện |
| 15 | **[CẬP NHẬT v4.1]** Tụ lọc 1000µF 10V + 100nF gốm | 1 bộ | 20.000 | 20.000 | Tụ lọc transient cho OPi5 |
| 16 | **[CẬP NHẬT v4.1]** Module ACS712 (5A) | 1 | 30.000 | 30.000 | Đo dòng motor rải — phát hiện kẹt phễu |
| 17 | **[CẬP NHẬT v4.2]** ArUco Marker (in A4 laminate ×8) | 8 | **6.000** | **50.000** | Landmark cho SLAM |
| 18 | **[CẬP NHẬT v4.2]** Module MPU6050 (IMU) | 1 | 40.000 | 40.000 | IMU fusion cho odometry |
| | **TỔNG CỘNG GĐ1** | | | **~10.610.000** | |

> 💡 **Ghi chú:** Chi phí GĐ1 tăng ~3 triệu so với phương án dùng điện thoại cũ, nhưng đổi lại được hệ thống **chuẩn công nghiệp (ROS2)**, ổn định 24/7, không lo sự cố pin điện thoại, và AI mạnh gấp 10 lần. Đây là sự đầu tư xứng đáng cho dài hạn.

### Giai đoạn 2 — Nâng cấp (tùy chọn)

| STT | Linh kiện | Giá (VNĐ) | Lợi ích |
| :---: | :--- | ---: | :--- |
| 1 | Màn hình 5-7 inch HDMI (gắn lên robot) | 800.000 | Hiển thị trực quan tại chỗ |
| 2 | Trạm sạc (tiếp điểm đồng + beacon IR + mạch sạc 42V) | 800.000 | Auto-docking sạc pin |
| 3 | Cảm biến IR (cho docking) × 2 | 60.000 | Căn chỉnh docking ± 2cm |
| 4 | Trạm nạp thức ăn (phễu lớn + van solenoid) | 600.000 | Auto-refill thức ăn |
| 5 | Camera hồng ngoại IR (Vision ban đêm) | 800.000 | Thay camera thường nếu cần |
| 6 | Pin Li-ion 36V 10Ah+ | 1.500.000 | Tăng thời gian hoạt động |
| | **TỔNG CỘNG GĐ2 (thêm)** | | **~4.560.000** |

---

## 9. Quy trình chế tạo & Tích hợp

### Giai đoạn 1: Flash firmware Hoverboard + Dựng khung Robot (Tuần 1–4)

| Bước | Công việc | Tiêu chí hoàn thành |
| :---: | :--- | :--- |
| 1.1 | Mua hoverboard cũ 10", **[CẬP NHẬT v4.1]** kiểm tra theo checklist Section 5.1.2 (nhận dạng chip STM32 vs clone, pinout UART, ST-Link connect) | Motor quay đều, pin giữ điện, chip STM32F103 xác nhận |
| 1.2 | Tháo vỏ nhựa, đế đứng, cảm biến nghiêng | Giữ lại board + motor + pin + sạc |
| 1.3 | **Flash firmware Emanuel Feru** qua ST-Link V2. **[CẬP NHẬT v4.1]** Nếu thất bại → kích hoạt Plan B (Section 5.1.3): đặt ODrive/SimpleFOC, ước thêm ~500–800k, 3–5 ngày | Board nhận lệnh UART, motor quay được khi gửi lệnh từ serial |
| 1.4 | Chế khung nhôm/thép (~60×70cm), gắn hoverboard vào giữa | Khung chắc, 4 điểm tiếp đất ổn định |
| 1.5 | Gắn 2 bánh caster phía trước + phanh đĩa fail-safe | Phanh tự đóng khi ngắt điện, nhả khi cấp điện |
| 1.6 | Nối ESP32 → Board HB (UART), test điều khiển từ OPi5 qua WiFi | Điều khiển W/A/S/D từ OPi5 qua WiFi |
| 1.7 | Dựng cột mast nhôm 2020 (~1.0m), gắn servo + Camera USB/CSI | Cột không rung, camera nhìn rõ vào bể |
| 1.8 | Test di chuyển trong lối đi hẹp ~80cm | Robot quay tại chỗ, không va bể |

### Giai đoạn 2: SLAM + Camera AI (Tuần 5–8)

| Bước | Công việc | Tiêu chí hoàn thành |
| :---: | :--- | :--- |
| 2.1 | Gắn RPLidar A1 (→ OPi5 USB), cài đặt `sllidar_ros2` | Bản đồ 2D trang trại hiển thị đúng |
| **2.1a** | **[CẬP NHẬT v4.2] Quy trình Mapping 5 bước:** 1. Đặt 6-10 ArUco marker (ghi tọa độ/ID). 2. Chạy `mapping` mode. 3. Teleop 1 vòng trang trại. 4. Map saver cli lưu bản đồ. 5. Chuyển sang `localization` mode. | ArUco detect OK, TF map→odom được sửa lỗi liên tục |
| 2.2 | Test Camera USB/CSI trên mast, servo xoay | Camera nhìn rõ vào bể ở cả 2 bên |
| 2.3 | **[CẬP NHẬT v4.2]** Train model Bootstrap từ public data (Tuần 1-4) | Accuracy ≥50% (đủ để demo flow) |
| 2.4 | Viết `state_machine.py` + `patrol_manager.py` | Robot tuần tra tự động qua các bể |
| 2.5 | Viết `alert_system.py` (Telegram + loa) | Gửi cảnh báo + phát âm thanh khi bất thường |

### Giai đoạn 3: Cho ăn thông minh + Thử nghiệm (Tuần 9–12)

| Bước | Công việc | Tiêu chí hoàn thành |
| :---: | :--- | :--- |
| 3.1 | Lắp phễu + ống rải + servo hướng, code FeederControl | Rải đúng vào miệng bể, không rơi ra ngoài |
| 3.2 | Viết `feeding_analyzer.py`: AI quan sát → điều chỉnh lượng | Robot tự giảm/dừng khi cá bỏ ăn |
| 3.3 | Code `scheduler.py` (hẹn giờ tuần tra + cho ăn) | Robot hoạt động tự động đúng lịch |
| 3.4 | Viết `dashboard.py` (web UI trên OPi5, truy cập qua WiFi) | Bản đồ SLAM + video AI + log |
| 3.5 | Thử nghiệm 5 ngày tại trang trại | Vận hành ổn định, SLAM chính xác |

### Giai đoạn 4: Docking + RAS + Mở rộng (Tuần 13–18, GĐ2)

| Bước | Công việc | Tiêu chí hoàn thành |
| :---: | :--- | :--- |
| 4.1 | Chế tạo trạm sạc 42V, gắn cảm biến IR, code `docking_manager.py` | Robot tự lùi vào trạm, sạc tự động |
| 4.2 | Chế tạo trạm nạp thức ăn, code auto-refill | Robot tự nạp đầy phễu |
| 4.3 | Viết `ras_monitor.py`: AI giám sát bể lọc RAS | Phát hiện mức nước thấp, bọt nhiều |
| 4.4 | (Tùy chọn) Nâng cấp Camera CSI chất lượng cao hơn | AI mạnh hơn, multi-camera, ROS2 |
| 4.5 | Thử nghiệm tích hợp 7 ngày | Toàn bộ hệ thống hoạt động ổn định |

---

## 10. Kế hoạch thử nghiệm & Nghiệm thu

### 10.1. Bài test chức năng

| TC | Tên test | Điều kiện đạt |
| :---: | :--- | :--- |
| TC-01 | Di chuyển trong lối đi hẹp | Không va chạm bể, quay đầu mượt |
| TC-02 | Đi thẳng 5 mét | Sai lệch ngang ≤ 5cm |
| TC-03 | Tránh vật cản (Lidar) | Dừng cách vật cản 30–50cm, 10/10 lần |
| TC-04 | Bản đồ SLAM trang trại | Lập bản đồ chính xác, sai lệch ≤ 10cm |
| TC-05 | Camera xoay nhìn vào bể | Servo xoay chính xác ± 5°, hình ảnh rõ |
| TC-06 | Rải thức ăn vào bể | 90% thức ăn rơi vào miệng bể, không ra ngoài |
| TC-07a | **[CẬP NHẬT v4.2]** AI nhận diện cá (Bootstrap - Tuần 8) | ≥ 50% mAP@0.5 với dataset public/bootstrap, ≥ 10 FPS |
| TC-07b | **[CẬP NHẬT v4.2]** AI nhận diện cá (Fine-tuned - Tuần 16) | ≥ 80% mAP@0.5 sau fine-tune với dataset thực tại trang trại, ≥ 30 FPS |
| TC-08 | Phân tích hành vi cho ăn | Phân biệt đúng cá ăn tích cực vs bỏ ăn |
| TC-09 | Tuần tra tự động | Hoàn thành tất cả bể đúng thứ tự |
| TC-10 | Cảnh báo Telegram | Gửi thông báo ≤ 5 giây sau khi phát hiện |
| TC-11 | Pin yếu | Cảnh báo khi < 20%, tự dừng khi < 10% |
| TC-12 | Mất WiFi | Tự dừng sau 10 giây mất kết nối |
| TC-13 | Auto-docking sạc pin (GĐ2) | Căn chỉnh ≤ 2cm, sạc tự động |
| TC-14 | Auto-refill thức ăn (GĐ2) | Nạp đầy phễu, không tràn |
| TC-15 | Giám sát RAS (GĐ2) | Phát hiện mức nước thấp ≥ 90% trường hợp |

### 10.2. Bài test tích hợp tại thực địa

- Chạy liên tục 4 giờ trong trang trại thực tế.
- Tuần tra 20+ bể, cho ăn 3 lần/ngày trong 5 ngày.
- Ghi nhận: độ chính xác rải thức ăn, false alarm rate, thời gian tuần tra, sự cố.

---

## 11. Vận hành & Bảo trì

### 11.1. Hướng dẫn vận hành hàng ngày

1.  **Sạc pin** đầy (hoặc để robot tự sạc tại trạm nếu GĐ2).
2.  **Đổ thức ăn** vào phễu (hoặc đổ vào trạm nạp nếu GĐ2).
3.  **Bật robot**, kiểm tra WiFi trên Dashboard.
4.  **Cài lịch** tuần tra + cho ăn hoặc chạy thủ công.
5.  Robot sẽ tự hoạt động theo lịch và gửi cảnh báo khi cần.

### 11.2. Bảo trì định kỳ

| Tần suất | Công việc |
| :--- | :--- |
| Hàng ngày | Lau khô phễu, kiểm tra bánh xe (không dính rác) |
| Hàng tuần | Kiểm tra dây nối, siết ốc cột mast, kiểm tra phanh đĩa + cáp phanh |
| Hàng tháng | Lau ống kính Lidar + Camera, kiểm tra solenoid phanh |
| 6 tháng | Kiểm tra vòng bi bánh caster, tiếp điểm sạc, cập nhật firmware |

---

## 12. Phân tích rủi ro kỹ thuật

| Rủi ro | Xác suất | Hậu quả | Biện pháp phòng ngừa |
| :--- | :---: | :---: | :--- |
| Nước bắn vào mạch điện | Trung bình | Nghiêm trọng | IP54 bọc kín, cáp chống ẩm, đặt mạch trên cao |
| Pin Li-ion cháy/phồng | Thấp | Nguy hiểm | BMS bảo vệ, không sạc quá 90%, tránh va đập |
| Mất WiFi trong trang trại | Thấp | Trung bình | Dùng repeater, ESP32 tự dừng + phanh đĩa sau 10s |
| Motor BLDC quá nhiệt | Rất thấp | Nhẹ | Robot chỉ 30kg vs tải motor 100kg, tốc độ 2–5 km/h → motor dùng <5% công suất |
| Robot va chạm bể | Trung bình | Nhẹ–TB | Lidar phát hiện, tốc độ thấp + phanh đĩa fail-safe, bumper foam |
| Thức ăn rơi ngoài bể | Trung bình | Nhẹ | Ống dẫn + servo căn chỉnh hướng, test từng bể |
| Cột mast rung lắc | Trung bình | Nhẹ | Ke góc chắc, thanh giằng, tốc độ thấp giảm rung. **[CẬP NHẬT v4.1]** Moment lật max 1.35 N·m — ke góc kép + thanh giằng chéo thứ 2 + guy wire tùy chọn (Section 5.2) |
| Docking lệch (GĐ2) | Trung bình | Nhẹ | 2 cảm biến IR, thuật toán PID căn chỉnh |
| Ăn mòn do hóa chất/muối | Trung bình | TB | Khung nhôm/thép xử lý chống rỉ, phủ thêm sơn |
| Flash firmware hoverboard thất bại | **20–30%** | **Cao** | **[CẬP NHẬT v4.1]** Checklist kiểm tra chip TRƯỚC khi mua (Section 5.1.2). **Plan B:** ODrive/SimpleFOC thay thế (+500–800k, +3–5 ngày). Không ảnh hưởng kiến trúc còn lại |
| **[CẬP NHẬT v4.1]** Kẹt phễu thức ăn | Trung bình | TB | Phễu inox 304 góc ≥45°, auger ≥20mm. Cảm biến dòng ACS712 phát hiện kẹt. Recovery tự động (đảo chiều 2s). Cảnh báo Telegram nếu hỏng |
| **[CẬP NHẬT v4.2]** Thức ăn văng khi recovery | Thấp | Nhẹ | Xoay servo ống về vị trí Neutral (thẳng đứng) trước khi đảo chiều auger |
| **[CẬP NHẬT v4.1]** SLAM aliasing (bể lặp lại) | Trung bình | Cao | ArUco marker tại 4 góc + đầu dãy bể. slam_toolbox chế độ localization, tắt loop closure tự động |
| **[CẬP NHẬT v4.1]** OPi5 crash do nguồn | Thấp | Cao | Buck XL4016 (ripple ≤50mV) + 2×1000µF tụ lọc transient. Không dùng LM2596 |

---

## 13. Lộ trình phát triển

```
GĐ1 — MVP (Nền tảng ROS2 mạnh mẽ)             GĐ2 — Tự động hóa hoàn toàn
───────────────────────────────────           ────────────────────────────
✓ Hoverboard cũ + khung tự chế + Phanh        → Auto-docking sạc pin tự động
✓ Orange Pi 5 (Ubuntu + ROS2 Humble)          → Auto-refill thức ăn
✓ RPLidar A1 (USB) + SLAM Toolbox             → Giám sát bể lọc RAS
✓ Camera AI (YOLOv8 Rockchip NPU)             → Pin Li-ion 36V dung lượng cao
✓ ESP32 Slave Control                         → Cảm biến chất lượng nước
✓ Cho ăn thông minh từng bể
✓ Dashboard ROS2 (Web based)
```

---

## 14. Phụ lục — Hỏi đáp kỹ thuật cho Hội đồng thẩm định

**Q1: Tại sao robot hoạt động trong nhà thay vì ngoài ao?**
> Trang trại RAS trong nhà là xu hướng nuôi trồng thủy sản hiện đại: kiểm soát môi trường tốt hơn, mật độ nuôi cao hơn, ít phụ thuộc thời tiết. Tuy nhiên, số lượng bể nhiều (hàng chục đến hàng trăm) khiến việc giám sát thủ công rất tốn công sức. Robot SIFAR giải quyết chính xác bài toán này.

**Q2: Camera gắn trên cột mast có ổn định không?**
> Cột mast bằng nhôm 2020 được cố định bằng ke góc và thanh giằng chéo. Servo xoay camera là SG90 nhẹ (9g), không tạo rung lắc đáng kể. Robot di chuyển tốc độ thấp (≤ 0.5m/s) trên nền phẳng nên cột rất ổn định.

**Q3: Robot rải thức ăn vào bể nhỏ như thế nào?**
> Thức ăn viên từ phễu đi qua trục vít → ống dẫn nhựa mềm dài ~30cm hướng vào miệng bể. Servo điều chỉnh góc ống để căn chỉnh chính xác. Robot dừng tại vị trí cố định (waypoint SLAM) trước mỗi bể.

**Q4: Auto-docking hoạt động như thế nào?**
> Trạm sạc có beacon IR (LED hồng ngoại) phát tín hiệu. Robot có 2 cảm biến IR ở phía sau. Khi pin < 20%, robot dùng SLAM di chuyển tới khu vực trạm sạc, sau đó dùng tín hiệu IR để căn chỉnh chính xác (± 2cm) và lùi vào tiếp điểm đồng.

**Q5: Hệ thống lọc RAS được giám sát như thế nào?**
> GĐ2: Robot dừng tại các bể lọc (waypoint trên bản đồ SLAM), camera AI phân tích hình ảnh: (1) mức nước — so sánh với mốc chuẩn; (2) bọt/váng — dấu hiệu lọc quá tải; (3) trạng thái máy bơm/UV — đèn báo có sáng không. Kết quả gửi về Dashboard.

**Q6: Robot có an toàn cho người khi hoạt động cùng trang trại?**
> Lidar quét 360° phát hiện người từ 2–5m, robot tự dừng khi vật cản < 50cm. Tốc độ tối đa chỉ 0.5m/s (đi bộ chậm). Bumper foam quanh đế robot giảm va chạm. Module SafetyMonitor trên ESP32 xử lý khẩn cấp cục bộ.

**Q12: Tại sao chọn Orange Pi 5 + ROS2 thay vì Điện thoại Android?**
> Mặc dù điện thoại Android rẻ (tận dụng đồ cũ), nó có 3 điểm yếu chí mạng trong môi trường công nghiệp/trang trại:
> 1.  **Nhiệt độ & Pin:** Chạy 24/7 cắm sạc liên tục làm pin phồng rất nhanh, nguy cơ cháy nổ cao. OPi5 dùng nguồn trực tiếp, tản nhiệt tốt.
> 2.  **Băng thông Lidar:** Truyền dữ liệu Lidar qua WiFi (từ ESP32 về Phone) gây trễ và rớt gói tin, làm SLAM bị lệch. OPi5 có USB 3.0, đọc Lidar trực tiếp ổn định tuyệt đối.
> 3.  **Chuẩn hóa:** ROS2 là tiêu chuẩn toàn cầu. Dùng OPi5 giúp chúng ta tiếp cận hàng ngàn package có sẵn (Navigation2, SLAM Toolbox) thay vì phải tự viết lại mọi thứ trên Android/Termux.

**Q7: Chi phí GĐ1 tăng lên ~10 triệu có cao không?**
> Mức giá này vẫn cực kỳ rẻ so với robot công nghiệp (50–200 triệu). Việc đầu tư thêm ~3 triệu cho Orange Pi 5 + Camera + Nguồn là khoản đầu tư cho **sự ổn định cốt lõi**. Nếu dùng điện thoại mà hệ thống chập chờn, hay hỏng vặt, chi phí bảo trì và rủi ro vận hành sẽ còn tốn kém hơn nhiều.

---

## 15. [CẬP NHẬT v4.1] Phản hồi Hội đồng Thẩm định

Bảng tóm tắt 7 vấn đề phản biện đã được giải quyết trong phiên bản 4.1:

| # | Vấn đề | Mức độ | Giải pháp đã bổ sung | Sections cập nhật |
| :---: | :--- | :---: | :--- | :--- |
| 1 | **Odometry cho Nav2** — Thiếu `/odom` topic | CRITICAL | Công thức tính odometry từ Hall RPM (D=254mm, L=520mm). Node `odom_publisher_node`. Chọn Serial Packet thay micro-ROS (giải thích 4 lý do). ESP32 gửi RPM 50Hz | 4.2, 6.2, 7.1, 7.2, 7.4 |
| 2 | **Dataset AI & Thuật toán** — Chưa định nghĩa cụ thể | CRITICAL | Kế hoạch dataset 3000 ảnh/6 class. Transfer learning YOLOv8n + augmentation cho ảnh qua nước. Thuật toán `feeding_analyzer.py` dùng optical flow + 3 ngưỡng quyết định. TC-07 chia 2 giai đoạn | 3.1, 7.2, 9, 10.1 |
| 3 | **Plan B Hoverboard** — 20-30% board clone không tương thích | CRITICAL | Checklist 5 bước kiểm tra board TRƯỚC khi mua. Plan B: ODrive (+500-800k) hoặc SimpleFOC (+300-500k). Quy trình chuyển đổi 5 bước | 1.2, 5.1, 9, 12 |
| 4 | **SLAM Aliasing** — Bể lặp lại gây loop closure failure | QUAN TRỌNG | ArUco marker (≥4 góc + đầu dãy bể). Cấu hình slam_toolbox: tắt loop closure, tăng match threshold. Quy trình mapping 4 bước. Node `aruco_detector_node` | 7.2, 9, 12 |
| 5 | **Nguồn 5V OPi5** — Transient load 4-5A gây crash | QUAN TRỌNG | Spec Buck: ripple ≤50mV, transient ≤100mV. Đề xuất XL4016 (250k). 2×1000µF + 100nF tụ lọc tại chân OPi5. Cập nhật tổng tải 7.5A | 6.1, 6.3, 8 |
| 6 | **Kẹt phễu thức ăn** — Không có cơ chế phát hiện/recovery | QUAN TRỌNG | Thiết kế cơ khí: phễu inox 304, góc ≥45°, auger ≥20mm. Cảm biến ACS712 đo dòng (ngưỡng 1.0A/500ms). Recovery: đảo chiều 2s → retry → Telegram alert | 3.1, 5.3, 6.2, 7.1, 7.2, 12 |
| 7 | **Moment lật cột mast** — Chưa tính toán khi phanh đột ngột | QUAN TRỌNG | Moment max 1.35 N·m, lực ngang 6.75N. Biện pháp: ke góc kép + thanh giằng chéo thứ 2 + guy wire tùy chọn. Torque siết ốc M5 ≥ 4 N·m | 5.2, 12 |

**Vòng 3 [CẬP NHẬT v4.2] — Tối ưu hóa Odometry & AI Workflow:**
| # | Vấn đề v4.2 | Mức độ | Giải pháp đã bổ sung | Sections |
| :---: | :--- | :---: | :--- | :--- |
| 1 | **Odometry Drift** — Chỉ dùng RPM bị tích lũy lỗi | CRITICAL | Thêm MPU6050 IMU. Sử dụng `robot_localization` (EKF) để fuse Wheel Odom + IMU Data | 4.2, 6.1, 6.2, 7.1, 7.2 |
| 2 | **Nav2 integration** — Luồng topic ArUco chưa rõ | CRITICAL | Sơ đồ luồng topic: Camera → ArUco → EKF. Marker reset drift tích lũy. Quy trình mapping 5 bước | 4.2, 7.2, 9 |
| 3 | **Dataset timeline** — 3000 ảnh label ngay là quá tải | QUAN TRỌNG | Tách làm 2 pha: Bootstrap (public data) và Fine-tune (tự động chụp khi patrol). Cập nhật TC-07a/b | 7.2, 9, 10.1, 3.1 |
| 4 | **Timestamp jitter** — OPi5 thiếu real-time gurantee | QUAN TRỌNG | Thêm `TIMESTAMP_MS` vào Serial Packet từ ESP32. EKF dùng timestamp MCU để tính dt chính xác | 7.1, 7.2, 7.4 |
| 5 | **Recovery Safety** — Văng thức ăn khi đảo chiều auger | QUAN TRỌNG | Quy trình recovery: quy vị trí Neutral (90°) trước khi đảo chiều. Cập nhật rủi ro Section 12 | 5.3, 7.1, 12 |
| 6 | **BOM Consistency** — Thông tin BOM rời rạc | NHỎ | Gộp các Item v4.1/v4.2 vào bảng BOM GĐ1 nhất quán. Thêm item IMU MPU6050 (~40k) | 8, 10.1 |

> **Tổng chi phí GĐ1 v4.2:** ~10.610.000 VNĐ.

---

> **Tài liệu tham khảo:** ESP32 Docs (espressif.com) · Arduino Reference · OpenCV · Ultralytics YOLOv8 · RPLidar SDK (Slamtec) · Emanuel Feru Hoverboard Firmware (GitHub) · Recirculating Aquaculture Systems (FAO) · **[v4.1]** OpenCV ArUco Module · slam_toolbox Documentation (Steve Macenski) · ODrive Documentation · SimpleFOC Library
