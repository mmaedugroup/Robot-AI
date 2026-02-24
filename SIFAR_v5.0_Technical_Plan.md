# DỰ ÁN ROBOT TRỢ LÝ TRANG TRẠI TRONG NHÀ THÔNG MINH
# (Smart Indoor Farm Assistant Robot — SIFAR)

> **Phiên bản:** 5.0 | **Cập nhật:** Tháng 2/2026
> **Loại tài liệu:** Kế hoạch kỹ thuật chi tiết (Technical Project Plan)

---

## 0. Lịch sử phiên bản

Bảng 0.1 — Lịch sử các phiên bản tài liệu

| **Phiên bản** | **Ngày**  | **Thay đổi chính**                                                  |
|:-------------:|:---------:|:---------------------------------------------------------------------|
| v3.0          | 01/2026   | Phiên bản gốc — Điện thoại Android làm bộ xử lý trung tâm          |
| v4.0          | 02/2026   | Nâng cấp lên Orange Pi 5 + ROS2 Humble, loại bỏ Android             |
| v4.1          | 02/2026   | Thêm IMU, ArUco SLAM, Plan B hoverboard, Dataset AI, chống kẹt phễu |
| v4.2          | 02/2026   | EKF fusion, Timestamp đồng bộ, BOM chuẩn hóa                       |
| v5.0          | 02/2026   | Fix ArUco TF/topic, thêm aruco_map.yaml, rewrite toàn bộ layout     |

---

## 1. Tổng quan dự án

### 1.1. Mô tả ngắn gọn

Dự án SIFAR hướng đến việc chế tạo một robot di động tự hành hoạt động bên trong trang trại nuôi trồng thủy sản trong nhà (Indoor Aquaculture Farm) sử dụng hệ thống tuần hoàn nước RAS (Recirculating Aquaculture System). Robot được thiết kế để thay thế phần lớn công việc giám sát và chăm sóc thủ công, hoạt động liên tục 24/7 với khả năng tự sạc pin.

Các năng lực cốt lõi của robot bao gồm:

- **Tuần tra giám sát bể nuôi:** Di chuyển dọc các dãy bể nhỏ (mỗi bể cao ~1.2m), dùng Camera AI nhìn từ trên xuống để quan sát sức khỏe cá/tôm trong từng bể.
- **Giám sát hệ thống lọc RAS:** Kiểm tra trực quan các bể lọc — mức nước, độ trong, bọt, trạng thái thiết bị phụ trợ.
- **Cho ăn tự động & thông minh:** Rải thức ăn vào từng bể theo lịch. Camera AI quan sát phản ứng ăn mồi để tự điều chỉnh lượng thức ăn — bao gồm cơ chế phát hiện kẹt phễu (đo dòng motor rải) và recovery tự động (đảo chiều auger).
- **Tự hành SLAM:** Dùng Lidar 360° lập bản đồ trang trại, tự di chuyển chính xác giữa các bể mà không cần người điều khiển. Hệ dẫn động sử dụng motor brushless từ hoverboard — yên tĩnh, bền bỉ, điều khiển FOC mượt mà.
- **Tự động sạc pin (Auto-Docking):** Khi pin yếu, robot tự quay về trạm sạc, căn chỉnh bằng cảm biến IR và tự nạp điện.
- **Tự động nạp thức ăn (Auto-Refill):** Khi hết thức ăn trong phễu, robot tự di chuyển tới trạm nạp liệu để nhận thức ăn mới.
- **Cảnh báo thời gian thực** qua Telegram/Push khi phát hiện bất thường (cá chết, nước đục, thiết bị lọc lỗi).
- **Khả năng mở rộng:** Kiến trúc phần mềm dạng module — dễ dàng thêm chức năng: quét mã QR định danh bể, đo cảm biến nước không tiếp xúc, điều khiển van/bơm từ xa.

### 1.2. Phạm vi triển khai

Dự án được chia thành hai giai đoạn rõ ràng:

- **Giai đoạn 1 (MVP):** Robot tự hành SLAM, tuần tra bể nuôi, cho ăn thông minh, Camera AI phân tích hành vi cá. Hệ dẫn động: Hoverboard 10" + Khung tự chế + Caster + Phanh đĩa fail-safe. Bộ xử lý: Orange Pi 5 (RAM 8GB) chạy Ubuntu + ROS2 — xử lý trung tâm mọi tác vụ AI & SLAM.
- **Giai đoạn 2 (Nâng cấp):** Thêm auto-docking sạc pin, auto-refill thức ăn, giám sát RAS, cảm biến nước. Tối ưu hóa thuật toán AI trên NPU Rockchip.

> 💡 LƯU Ý — **Dự phòng dẫn động:** Nếu board hoverboard không tương thích firmware Emanuel Feru (chip clone GD32 hoặc pinout khác), sẽ chuyển sang Plan B — ODrive/SimpleFOC điều khiển trực tiếp motor BLDC (xem chi tiết tại Section 5.1 và Section 12).

### 1.3. Điểm khác biệt so với giải pháp hiện tại

Bảng 1.1 — So sánh giám sát thủ công và Robot SIFAR

| **Tiêu chí**           | **Giám sát thủ công**                    | **Robot SIFAR**                                     |
|:------------------------|:-----------------------------------------|:----------------------------------------------------|
| Kiểm tra bể            | Người đi từng bể, 2–3 lần/ngày          | Robot tuần tra tự động 24/7, AI phân tích           |
| Phát hiện cá bệnh      | Bằng mắt thường, dễ bỏ sót              | AI nhận diện hành vi bất thường, cảnh báo tức thì   |
| Cho ăn                  | Rải tay hoặc máy cố định                | Robot rải từng bể, AI điều chỉnh lượng theo phản ứng cá |
| Giám sát RAS           | Phải đi kiểm tra thủ công               | Robot tự quét, cảnh báo khi bể lọc bất thường      |
| Hoạt động ban đêm      | Không                                    | Robot tuần tra liên tục, tự sạc pin                 |
| Khả năng mở rộng       | Rất hạn chế                             | Thêm cảm biến, quét QR, điều khiển van/bơm         |

---

## 2. Phân tích môi trường trang trại trong nhà

### 2.1. Đặc điểm môi trường

Trang trại RAS trong nhà đặt ra những yêu cầu thiết kế đặc thù cho robot tự hành. Bảng dưới đây tóm tắt các đặc điểm chính và ảnh hưởng trực tiếp đến thiết kế SIFAR.

Bảng 2.1 — Đặc điểm môi trường trang trại và ảnh hưởng đến thiết kế

| **Đặc điểm**                    | **Điều kiện**                                    | **Ảnh hưởng đến thiết kế**                                     |
|:---------------------------------|:-------------------------------------------------|:---------------------------------------------------------------|
| Bể nuôi cao ~1.2m               | Bể composite/nhựa, miệng mở hoặc có lưới        | Robot cần cột mast camera cao ≥ 1.3m để nhìn xuống bể         |
| Không gian hẹp                  | Lối đi giữa các dãy bể rộng ~1–1.5m             | Robot chiều rộng ≤ 65cm, quay tại chỗ (differential drive)    |
| Nền phẳng                       | Bê tông/gạch/epoxy, phẳng, có thể ẩm ướt        | Bánh hoverboard 10" (D=254mm) bám tốt, vượt ngưỡng cửa/rãnh dễ |
| Môi trường ẩm + hóa chất       | Độ ẩm cao, chlorine, muối (bể mặn)              | Khung nhôm/nhựa chống ăn mòn, mạch IP54, cáp chống ẩm        |
| Ánh sáng nhân tạo              | Đèn LED trang trại (có thể tắt ban đêm)         | Camera cần hoạt động cả điều kiện sáng lẫn thiếu sáng         |
| Hệ thống RAS                   | Bể lắng, bể lọc sinh học, UV, máy bơm           | Robot cần giám sát trực quan: mức nước, bọt, độ trong         |
| Trạm cố định                   | Có vị trí đặt trạm sạc + trạm nạp thức ăn      | Robot cần cơ cấu docking chính xác (cảm biến IR)              |

### 2.2. Bài toán cần giải quyết

> Chế tạo robot tự hành cho trang trại trong nhà (< 10 triệu VNĐ GĐ1), tận dụng motor brushless từ hoverboard cũ làm hệ dẫn động (motor BLDC 350W, pin 36V Li-ion, điều khiển FOC), kết hợp khung tự chế + 2 caster + phanh đĩa fail-safe, tích hợp Lidar SLAM + Camera AI, di chuyển trong lối đi giữa các dãy bể cao ~1.2m, giám sát sức khỏe cá từng bể, cho ăn thông minh, và mở rộng chức năng (auto-docking, giám sát RAS) — phù hợp cho trang trại RAS quy mô vừa và nhỏ.

---

## 3. Yêu cầu kỹ thuật

### 3.1. Yêu cầu chức năng (Functional Requirements)

Bảng 3.1 — Yêu cầu chức năng

| **ID** | **Yêu cầu**                       | **Ưu tiên** | **Giải thích**                                                                                      |
|:------:|:-----------------------------------|:-----------:|:------------------------------------------------------------------------------------------------------|
| FR-01  | Di chuyển tự hành trong trang trại | Bắt buộc   | SLAM Lidar lập bản đồ, robot tự di chuyển giữa các bể                                               |
| FR-02  | Điều khiển từ xa qua WiFi         | Bắt buộc   | Người dùng có thể can thiệp thủ công từ Laptop/phone                                                |
| FR-03  | Tuần tra bể nuôi (Patrol Mode)    | Bắt buộc   | Robot đi dọc từng dãy bể, Camera AI nhìn xuống từng bể                                              |
| FR-04  | Cho ăn tự động từng bể            | Bắt buộc   | Dừng tại mỗi bể, rải thức ăn đúng lượng. Bao gồm phát hiện kẹt phễu (đo dòng motor) và recovery tự động (đảo chiều auger) |
| FR-05  | Camera AI phân tích hành vi cá    | Bắt buộc   | Nhận diện cá bệnh, đánh giá phản ứng ăn mồi. Độ chính xác target ≥80% sau giai đoạn fine-tune (cuối GĐ1). Dùng YOLOv8n + optical flow |
| FR-06  | Tránh vật cản (Lidar)             | Bắt buộc   | Lidar quét 360° phát hiện người/vật cản → dừng/chờ                                                  |
| FR-07  | Cảnh báo thời gian thực           | Bắt buộc   | Gửi Telegram khi phát hiện bất thường                                                                |
| FR-08  | Cảnh báo pin yếu                  | Bắt buộc   | Thông báo khi pin sắp hết                                                                            |
| FR-09  | Hẹn giờ tuần tra + cho ăn         | Bắt buộc   | Đến giờ → Robot tự khởi động nhiệm vụ                                                               |
| FR-10  | Giám sát hệ thống RAS             | GĐ2        | Camera kiểm tra bể lọc: mức nước, bọt, trạng thái thiết bị                                         |
| FR-11  | Tự động sạc pin (Auto-Docking)    | GĐ2        | Pin < 20% → robot tự tìm trạm sạc, căn chỉnh, nạp điện                                             |
| FR-12  | Tự động nạp thức ăn (Auto-Refill) | GĐ2        | Hết thức ăn → robot tự tới trạm nạp, mở phễu, nhận thức ăn                                         |
| FR-13  | Quét mã QR định danh bể           | GĐ2+       | Đọc mã QR trên mỗi bể để xác định chính xác bể nào                                                 |
| FR-14  | Phát hiện cá bệnh / cá chết       | Nên có     | AI phân tích hành vi bơi bất thường, cá nổi, đổi màu. Transfer learning từ YOLOv8n pre-trained, fine-tune với dataset trang trại thực tế |


#### 3.1.1. Kế hoạch Dataset AI & Thuật toán nhận diện cá (FR-05, FR-14)

**A. Kế hoạch thu thập Dataset**

Bảng 3.2 — Kế hoạch thu thập Dataset

| **Hạng mục**          | **Chi tiết**                                                                                          |
|:----------------------|:------------------------------------------------------------------------------------------------------|
| Số lượng tối thiểu    | 500 ảnh/class × 6 class = **3.000 ảnh** (trước augmentation)                                         |
| Các class cần label   | `healthy_fish`, `sick_fish`, `dead_fish`, `normal_feeding`, `active_feeding`, `no_feeding`             |
| Điều kiện chụp        | Góc top-down từ mast 1.35m, qua mặt nước, dưới đèn LED trang trại, cả ban ngày lẫn ban đêm          |
| Công cụ label         | LabelImg hoặc Roboflow (export YOLO format)                                                          |
| Thu thập khi nào      | Giai đoạn 2 (Tuần 5–8), mỗi ngày chụp ~100 ảnh tại trang trại thực tế                               |

> ⚠️ CẢNH BÁO — **License dataset:** Ưu tiên dataset có license CC BY 4.0 hoặc CC0 (public domain). Kiểm tra license trên Roboflow trước khi download. Nếu dataset có NonCommercial restriction → chỉ dùng cho prototype, phải thay thế bằng data tự thu thập trước khi thương mại hóa.

**B. Transfer Learning Strategy**

Bảng 3.3 — Chiến lược Transfer Learning

| **Bước**                                  | **Chi tiết**                                                                                            |
|:------------------------------------------|:--------------------------------------------------------------------------------------------------------|
| Model gốc                                | YOLOv8n (nano) pre-trained trên COCO → convert sang RKNN cho NPU                                       |
| Fine-tune                                | Freeze backbone (CSPDarknet53), chỉ train head detection trên dataset trang trại                        |
| Augmentation phù hợp ảnh qua mặt nước    | Motion blur (kernel 5–15px), Random reflection (flip + brightness ±30%), Barrel distortion (k1=0.1–0.3), Color jitter (hue ±10°, saturation ±20%) |
| Epochs                                   | 100–200 epochs, batch size 16, learning rate 0.001 → 0.0001                                            |
| Validation                               | 80/20 train-val split, đánh giá mAP@0.5                                                                |

**C. Thuật toán `feeding_analyzer.py` (Optical Flow)**

```python
# Input: 2 frame liên tiếp từ camera (t và t+1)
# 1. Detect vùng ROI chứa thức ăn vừa rải (tọa độ từ feeding waypoint)
# 2. cv2.goodFeaturesToTrack() → tìm feature points trên cá
# 3. cv2.calcOpticalFlowPyrLK(frame_t, frame_t1, points) → tính vector di chuyển
# 4. Đếm số vector hướng VỀ PHÍA thức ăn (cosine similarity > 0.7)
# 5. Tính tỷ lệ: ratio = vectors_toward_food / total_vectors
#
# Ngưỡng quyết định:
#   ratio > 0.6   → ACTIVE_FEEDING  → Rải đủ lượng cài đặt
#   0.3 < ratio ≤ 0.6 → REDUCED_FEEDING → Giảm 50% lượng, ghi log
#   ratio ≤ 0.3  → NO_FEEDING → Dừng rải, cảnh báo Telegram
#
# Cập nhật mỗi 2 giây (5 frame pair), lấy trung bình trượt 3 lần đo.
```

### 3.2. Yêu cầu phi chức năng (Non-Functional Requirements)

Bảng 3.4 — Yêu cầu phi chức năng

| **ID**  | **Yêu cầu**                       | **Chỉ tiêu**                                     |
|:-------:|:-----------------------------------|:--------------------------------------------------|
| NFR-01  | Thời lượng pin                    | ≥ 3 giờ hoạt động liên tục                       |
| NFR-02  | Tải trọng phễu thức ăn           | ≥ 2 kg thức ăn viên                              |
| NFR-03  | Tốc độ di chuyển                  | 0.2 – 0.5 m/s (tốc độ thấp cho indoor)           |
| NFR-04  | Phạm vi WiFi                     | Phủ toàn bộ trang trại (có repeater nếu cần)     |
| NFR-05  | Khả năng chống ẩm                | IP54 (chống bụi, chống nước bắn)                  |
| NFR-06  | Chiều rộng robot                  | ≤ 65 cm (vừa lối đi ≥ 80cm)                      |
| NFR-07  | Chiều cao tổng (có cột mast)     | ≥ 1.3m (nhìn được vào bể 1.2m)                   |
| NFR-08  | Chi phí GĐ1                      | ≤ 10.000.000 VNĐ                                 |
| NFR-09  | Độ phân giải Camera AI           | ≥ 1080p, góc rộng ≥ 120°                         |
| NFR-10  | Tốc độ xử lý AI                  | ≥ 10 FPS inference                                |
| NFR-11  | Độ chính xác SLAM                | ≥ 360° quét, ≥ 5.5m phạm vi                      |
| NFR-12  | Độ chính xác docking             | ≤ 2cm sai lệch khi vào trạm sạc                  |
| NFR-13  | Thời gian phản hồi cảnh báo     | ≤ 5 giây từ khi phát hiện đến gửi thông báo      |

---

## 4. Kiến trúc hệ thống

### 4.1. Sơ đồ trạng thái (State Machine)

Robot vận hành dựa trên máy trạng thái (State Machine) để đảm bảo tính tự động hóa cao. Mỗi trạng thái có điều kiện chuyển tiếp rõ ràng, cho phép robot phản ứng linh hoạt với các tình huống khác nhau trong quá trình vận hành.

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

Sơ đồ dưới đây mô tả kiến trúc phần cứng ba tầng: Điều khiển (Brain & Low-level), Chấp hành, và Nguồn điện.

```
┌────────────────────────────────────────────────────────────────────┐
│                  TẦNG ĐIỀU KHIỂN (BRAIN & LOW-LEVEL)               │
│  ┌───────────────────┐  USB/UART  ┌──────────────────────────────┐ │
│  │ 🍊 Orange Pi 5    │◄─────────►│    ESP32 (Vi điều khiển)     │ │
│  │   (Master Node)   │            │    (Low-level Slave)         │ │
│  │   - ROS2 Humble   │            │    - Nhận lệnh vận tốc (vel)│ │
│  │   - SLAM & Nav2   │            │    - PID Motor Control       │ │
│  │   - YOLOv8 (NPU)  │            │    - Servo camera/ống rải   │ │
│  │   - Web Dashboard │            │    - Solenoid phanh đĩa     │ │
│  │   - Camera USB/CSI│            │    - Safety Monitor          │ │
│  │   - Lidar Driver  │            │    - Docking sensor read     │ │
│  │   - odom_publisher│            │    - Đọc RPM Hall sensor     │ │
│  │                   │            │    - MPU6050 (IMU 6-axis)    │ │
│  └─┬───────┬─────────┘            └──────────┬───────────────────┘ │
│    │ USB   │ USB/CSI                         │ UART (FOC cmd)      │
│  ┌─▼─────┐ ┌▼────────┐                     ┌─▼─────────┐          │
│  │RPLidar│ │Camera   │                     │Board      │          │
│  │A1     │ │Wide/CSI │                     │Hoverboard │          │
│  └───────┘ └─────────┘                     └───────────┘          │
├────────────────────────────────────────────────────────────────────┤
│   TẦNG CHẤP HÀNH (Hoverboard + Khung tự chế + Phanh đĩa)        │
│     ┌──────────────────────────────────┼─────────────────┐        │
│     │           │          │           │         │       │        │
│ ┌───▼─────┐ ┌───▼──────┐   │         ┌▼──────┐ ┌▼────┐  │        │
│ │Board    │ │Motor rải │   │         │Phanh  │ │IR   │  │        │
│ │Hoverboard│ │thức ăn   │   │         │đĩa    │ │Dock │  │        │
│ │(STM32   │ │+Servo nắp│   │         │+Solo- │ │Sen- │  │        │
│ │FOC,UART)│ │phễu      │   │         │noid   │ │sor  │  │        │
│ │2x BLDC  │ │          │   │         │fail-  │ │     │  │        │
│ │350W,36V │ └──────────┘   │         │safe   │ └─────┘  │        │
│ │+Hall    │               │         └───────┘          │        │
│ └─────────┘               │                            │        │
├───────────────────────────┼────────────────────────────┼────────┤
│        TẦNG NGUỒN ĐIỆN   │                            │         │
│  Pin Hoverboard 36V ──────┴──► Buck 5V-10A ──► OPi5 + Lidar    │
│  (Li-ion ~4.4Ah,          ───► Buck 5V-5A  ──► ESP32 + Servo   │
│   nâng cấp 6-10Ah)        ───► Trực tiếp 36V ──► Board HB     │
│                                                                 │
│  [TRẠM SẠC] ──► Tiếp điểm đồng ──► Mạch sạc 42V               │
│                  (tận dụng bộ sạc hoverboard sẵn có)            │
└─────────────────────────────────────────────────────────────────┘
```

#### 4.2.1. Luồng Odometry cho ROS2 Nav2

Nav2 yêu cầu topic `/odometry/filtered` (nav_msgs/Odometry) sau khi fuse IMU và Wheel Odom để giảm drift. Dưới đây là luồng dữ liệu đầy đủ từ hardware đến navigation stack.

```
Board Hoverboard (STM32 FOC) → Hall Sensor RPM
  → [UART] → ESP32 (Đọc RPM + MPU6050 IMU @100Hz + Timestamp MS)
  → [USB Serial] → Orange Pi 5
  → [odom_publisher_node] (Tính wheel odom dùng timestamp ESP32) → /odom
  → [imu_publisher_node] (Xử lý raw IMU) → /imu/data
  → [robot_localization EKF] (Fusion odom + imu + ArUco) → /odometry/filtered
  → [Nav2 stack] (Dẫn đường tự động)
```

#### 4.2.2. Luồng ROS2 Topic tích hợp ArUco

Cơ chế reset drift bằng landmark nhân tạo khi robot đi qua ArUco marker. Khi camera nhận dạng được marker ID, `aruco_detector_node` tính toán vị trí robot trong Map frame (dựa trên file `aruco_map.yaml` — xem Section 9) và gửi dữ liệu correction về EKF. Do marker có tọa độ cố định ("Anchor"), nó giúp triệt tiêu sai số tích lũy từ bánh xe và IMU.

```
┌──────────────┐
│ Camera USB   │ → /image_raw
└──────┬───────┘
       ▼
┌──────────────────────────┐
│   aruco_detector_node    │
│   (ros2_aruco package)   │
│   + aruco_map.yaml       │
└──────────┬───────────────┘
           ▼
  /aruco_pose_corrected
  (PoseWithCovarianceStamped)
           ▼
┌──────────────────────────────────────┐
│   ekf_node — robot_localization      │
│   Inputs:                            │
│     odom0 : /odom        (wheel)     │
│     imu0  : /imu/data    (MPU6050)   │
│     pose0 : /aruco_pose_corrected    │
└──────────┬───────────────────────────┘
           ▼
  /odometry/filtered (nav_msgs/Odometry)
       ┌───┴───┐
       ▼       ▼
  [Nav2]   [slam_toolbox]
           (map → odom TF)
```

> 💡 LƯU Ý — `aruco_detector_node` publish topic `/aruco_pose_corrected` kiểu `geometry_msgs/PoseWithCovarianceStamped` (KHÔNG phải `PoseArray`). Đây là yêu cầu bắt buộc của `robot_localization` EKF node.

#### 4.2.3. Phân tách TF Responsibility

Để tránh xung đột TF tree giữa các node, mỗi transform chỉ có đúng một publisher duy nhất.

Bảng 4.1 — Phân bổ TF Frame Publisher

| **TF Frame**            | **Publisher**    | **Ghi chú**                        |
|:------------------------|:-----------------|:-----------------------------------|
| odom → base_link        | ekf_node         | Fuse odom + IMU + ArUco            |
| map → odom              | slam_toolbox     | SLAM correction                    |
| base_link → laser       | static_tf        | Vị trí Lidar cố định              |
| base_link → camera      | static_tf        | Vị trí Camera cố định             |

> ⚠️ CẢNH BÁO — `ekf_node` chỉ publish transform `odom → base_link`. Transform `map → odom` do `slam_toolbox` quản lý. Nếu cả hai cùng publish sẽ gây conflict TF tree và Nav2 crash.

#### 4.2.4. Cấu hình EKF (robot_localization)

File `ekf.yaml` cấu hình đầy đủ cho `robot_localization` EKF node:

```yaml
# ─── Tần số và chế độ ───
frequency: 50                    # Hz — tần số fusion output
two_d_mode: true                 # Robot di chuyển 2D trên sàn phẳng

# ─── TF Configuration ───
publish_tf: true                 # Chỉ publish odom → base_link
map_frame: map
odom_frame: odom
base_link_frame: base_link
world_frame: odom                # EKF làm việc trong odom frame

# ─── Nguồn dữ liệu 1: Wheel Odometry ───
odom0: /odom
odom0_config: [true,  true,  false,   # x, y, z
               false, false, true,    # roll, pitch, yaw
               false, false, false,   # vx, vy, vz
               false, false, true,    # vroll, vpitch, vyaw
               false, false, false]   # ax, ay, az

# ─── Nguồn dữ liệu 2: IMU ───
imu0: /imu/data
imu0_config: [false, false, false,    # x, y, z
              false, false, false,    # roll, pitch, yaw
              false, false, false,    # vx, vy, vz
              false, false, true,     # vroll, vpitch, vyaw
              true,  false, false]    # ax, ay, az
imu0_remove_gravitational_acceleration: true

# ─── Nguồn dữ liệu 3: ArUco Correction ───
pose0: /aruco_pose_corrected
pose0_config: [true,  true,  false,   # x, y, z
               false, false, true,    # roll, pitch, yaw
               false, false, false,   # vx, vy, vz
               false, false, false,   # vroll, vpitch, vyaw
               false, false, false]   # ax, ay, az
pose0_differential: false             # Absolute pose từ ArUco marker
pose0_rejection_threshold: 2.0        # Mahalanobis distance — bỏ qua
                                      # outlier nếu marker bị nhận dạng sai

# ─── Covariance khởi tạo ───
# ArUco có covariance nhỏ hơn odom → EKF ưu tiên correction
initial_estimate_covariance: [1e-9, 0, ...]
```


### 4.3. Odometry Công thức

Hệ dẫn động vi sai (differential drive) dựa trên tốc độ bánh trái/phải từ Hall sensor.

Bảng 4.2 — Thông số tính toán Odometry

| **Thông số**           | **Giá trị**                                          |
|:----------------------|:------------------------------------------------------|
| Wheelbase (L)               | **L_MEASURED** (Đo thực tế tâm-tâm bánh)              |
| Wheel radius (R)            | 127 mm (Bánh 10")                                    |
| Gear ratio (N)              | 1 (Direct drive)                                     |
| Hall pulses/round           | 90 (Tùy số cặp cực motor - Firmware Feru config)    |

**Công thức vận tốc tại tâm robot:**

```cpp
V_linear = (V_right + V_left) / 2
V_angular = (V_right - V_left) / L
```

> ✅ ĐẠT — Sử dụng `L_MEASURED` (giá trị đo thực tế trên khung đã lắp ráp) thay vì giá trị thiết kế giúp giảm sai số quay (yaw error) đáng kể trong di chuyển Nav2.

---

## 5. Thiết kế cơ khí

### 5.1. Cấu trúc khung robot — Hoverboard + Khung tự chế + Caster + Phanh đĩa

Robot SIFAR sử dụng motor brushless từ hoverboard cũ làm hệ dẫn động, kết hợp khung tự chế, 2 bánh caster, và phanh đĩa fail-safe. Giải pháp này mang lại chi phí thấp, độ bền cao, và torque dư thừa cho trọng tải robot ~25kg.

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

**Phần BỎ ĐI:** Vỏ nhựa hoverboard, đế đứng, cảm biến nghiêng tự cân bằng, firmware gốc (thay bằng firmware Emanuel Feru FOC).

**Phần TỰ CHẾ / THÊM MỚI:**
- Khung nhôm/thép (~60×70cm) gắn hoverboard ở vị trí trung tâm.
- 2× Bánh caster Φ75–100mm phía trước → tạo 4 điểm tiếp đất, ổn định tĩnh.
- Phanh đĩa fail-safe (×1 hoặc ×2): caliper cơ xe đạp + đĩa Φ120mm gắn trục hub motor + solenoid kéo cáp (lò xo hồi → mất điện = phanh tự đóng).
- Cột mast nhôm 2020 cao ~1.0m (tổng ~1.35m từ mặt đất).
- Orange Pi 5 + Camera USB/CSI + servo xoay trên đỉnh mast.
- RPLidar A1 gắn trên khung (~25cm).
- Phễu + ống rải thức ăn.
- ESP32 (giao tiếp UART với board hoverboard, điều khiển servo/solenoid).

Bảng 5.1 — Thông số khung robot

| **Thông số**           | **Giá trị**                                          |
|:----------------------|:------------------------------------------------------|
| Kích thước đế          | ~60cm (rộng) × 70cm (dài) × 20cm (cao đế)            |
| Chiều cao tổng         | ~1.35m (có cột mast + OPi5)                          |
| Trọng lượng robot      | ~20–25kg (nhẹ hơn xe lăn 10–15kg)                    |
| Tải trọng an toàn      | ≤ 30kg (motor thiết kế cho 100kg người)               |
| Cấu hình bánh xe       | 2 bánh chủ động BLDC 10" (giữa) + 2 caster (trước)  |
| Khả năng quay          | Quay tại chỗ 360° (differential drive)                |
| Phanh an toàn          | Phanh đĩa fail-safe (lò xo đóng khi mất điện)        |

> 💡 LƯU Ý — Motor brushless bền gấp đôi motor brushed (10,000+ giờ), yên tĩnh hơn nhiều cho indoor farm. Firmware Emanuel Feru cung cấp FOC + PID sẵn → ESP32 chỉ gửi lệnh tốc độ qua UART. Chi phí tương đương (~1.6–3 triệu tổng hoverboard + khung + caster + phanh).

#### 5.1.1. Cơ cấu phanh đĩa fail-safe

Hoverboard không có phanh cơ khí, vì vậy cần tự chế phanh fail-safe theo nguyên lý "lò xo đóng — solenoid mở":

- **Có điện (robot chạy):** Solenoid 12V kéo cáp phanh → nhả phanh → bánh quay tự do.
- **Mất điện (an toàn):** Lò xo hồi trong solenoid đẩy → cáp phanh thắt → caliper ép đĩa → bánh KHÓA.

Bảng 5.2 — Linh kiện phanh đĩa fail-safe

| **Linh kiện**                            | **Mô tả**                              |
|:-----------------------------------------|:----------------------------------------|
| Caliper phanh đĩa cơ xe đạp            | Loại kéo cáp, Shopee ~80–100k          |
| Đĩa phanh Φ120mm                       | Gắn vào trục hub motor                  |
| Solenoid kéo 12V (pull type, có lò xo)  | Hành trình ~10–15mm, ~50–80k            |
| Bracket gắn caliper                      | Tự chế nhôm L                           |

#### 5.1.2. Checklist kiểm tra board Hoverboard TRƯỚC khi mua

Bảng 5.3 — Checklist kiểm tra board Hoverboard

| **Bước** | **Kiểm tra**           | **Cách thực hiện**                                                                                     | **Đạt** |
|:--------:|:-----------------------|:--------------------------------------------------------------------------------------------------------|:-------:|
| 1        | Nhận dạng chip chính  | Mở nắp board, đọc marking: `STM32F103RCT6` = OK. `GD32F103`/`CKS32` = **chip clone, rủi ro cao**      | ✅/❌   |
| 2        | Kiểm tra pinout UART  | Tìm 2 pad/header GND + TX + RX trên board                                                              | ✅/❌   |
| 3        | Đọc chip qua ST-Link  | Cắm ST-Link V2, dùng STM32CubeProgrammer → "Connect". Đọc được chip ID = flash được                   | ✅/❌   |
| 4        | Kiểm tra Hall sensor  | Quay tay từng bánh, đo tín hiệu Hall bằng oscilloscope/logic analyzer (3 kênh/motor)                   | ✅/❌   |
| 5        | Pin + BMS              | Đo điện áp pin ≥ 33V (10S đầy ~42V). Kiểm tra BMS có led báo                                          | ✅/❌   |

> ⚠️ CẢNH BÁO — Nếu bước 1 hoặc 3 KHÔNG ĐẠT → Kích hoạt Plan B ngay, không cố flash.

#### 5.1.3. Plan B — Nếu flash firmware thất bại

**Khi nào kích hoạt:** Board hoverboard dùng chip clone (GD32/CKS32), ST-Link không nhận, hoặc flash firmware Feru thành công nhưng motor không quay đúng.

Bảng 5.4 — Phương án thay thế Plan B

| **Phương án**              | **Mô tả**                                                                                  | **Chi phí thêm** | **Thời gian thêm** |
|:---------------------------|:--------------------------------------------------------------------------------------------|:-----------------:|:-------------------:|
| ODrive v3.6 (clone)        | Board FOC chuyên dụng, hỗ trợ 2 motor BLDC. Giao tiếp UART/USB trực tiếp                  | ~500–800k         | ~3–5 ngày           |
| SimpleFOC (Arduino)        | Thư viện FOC mã nguồn mở chạy trên ESP32 + driver MOSFET riêng (L6234 hoặc DRV8302)       | ~300–500k         | ~5–7 ngày           |

Quy trình Plan B:
1. Tháo board hoverboard gốc, giữ lại motor BLDC + Hall sensor + pin.
2. Nối motor trực tiếp vào ODrive/SimpleFOC driver.
3. Cấu hình encoder = Hall sensor (3 kênh), calibrate motor.
4. ESP32 giao tiếp với ODrive qua UART (protocol tương tự, chỉ đổi command format).
5. Firmware ESP32 cần sửa: thay hàm `sendHoverboardCmd()` bằng `sendODriveCmd()`.

> 💡 LƯU Ý — Plan B vẫn sử dụng cùng motor BLDC và pin 36V, chỉ thay board điều khiển. Phần còn lại của kiến trúc (khung, caster, phanh, ESP32, OPi5) không thay đổi.

### 5.2. Cột Mast Camera

Cột mast là yếu tố then chốt cho phép camera AI nhìn xuống bể nuôi từ trên cao, phục vụ cả tuần tra lẫn cho ăn thông minh.

- **Chiều cao:** ~1.1m tính từ đế robot (tổng ~1.35m từ mặt đất).
- **Vật liệu:** Ống nhôm φ25mm hoặc thanh nhôm 2020, cố định bằng ke góc.
- **Đỉnh cột:** Đế xoay (servo nhỏ) gắn Camera USB/CSI (hướng xuống bể), cho phép xoay 180° để nhìn bể ở cả 2 bên lối đi. Orange Pi 5 đặt gần camera hoặc ở đế robot.

Bảng 5.5 — Tính toán moment lật cột mast khi phanh đĩa đóng đột ngột

| **Thông số**                                 | **Giá trị**                                  | **Ghi chú**                          |
|:---------------------------------------------|:----------------------------------------------|:--------------------------------------|
| Khối lượng đỉnh mast (OPi5 + Camera + Servo) | ~500g (0.5 kg)                                |                                       |
| Chiều cao trọng tâm tải đỉnh                | 1.35m từ mặt đất                              |                                       |
| Gia tốc phanh (worst case)                   | 1–2 m/s²                                     | Từ 0.5 m/s về 0 trong 0.25–0.5s      |
| **Moment lật tại đế cột**                   | **M = 0.5 × 1.35 × 2 = 1.35 N·m (max)**      | F = m × a, M = F × h                 |
| Lực ngang tại ke góc nhôm 2020               | F = M / h_ke = 1.35 / 0.2 ≈ **6.75 N**       | h_ke = 0.2m (chiều cao ke góc)       |
| Torque siết ốc M5 tối thiểu                 | **≥ 4 N·m**                                   | Ke góc nhôm 2020 + ốc M5 inox        |

Biện pháp gia cố:
1. **Ke góc kép:** Dùng 2 ke góc nhôm 2020 đối xứng 2 bên tại chân cột mast.
2. **Thanh giằng chéo thứ 2:** Từ điểm 2/3 cột mast (~0.7m) xuống khung đế, tạo tam giác chống xoay. Vật liệu: thanh nhôm 2020 hoặc ống nhôm φ15mm.
3. **Guy wire (tùy chọn):** 2 dây thép inox φ1.5mm từ điểm 3/4 cột mast (~0.8m) neo xuống 2 điểm trên khung đế, tạo hình chữ V ngược. Tăng đơ M4 để căng dây.

> ✅ ĐẠT — Moment lật 1.35 N·m là nhỏ (tương đương lực 675g ở đầu cần 20cm). Ke góc kép + thanh giằng chéo là đủ. Guy wire chỉ cần nếu nền nhà không phẳng gây rung lắc thêm.

### 5.3. Cơ cấu rải thức ăn (cho bể nhỏ)

- **Phễu:** Dung tích 2–3 kg, miệng rộng 15cm, cổ hẹp 5cm. Vật liệu inox 304 (hoặc nhựa HDPE) chống thấm nước, chống ẩm >80%.
- **Cơ chế rải:** Motor DC nhỏ gắn trục vít (auger screw) tại cổ phễu.
- **Ống dẫn linh hoạt:** Thức ăn từ phễu đi qua ống nhựa mềm dài ~30cm, hướng vào miệng bể. Đầu ống có thể xoay/nghiêng bằng servo nhỏ.
- **Nắp phễu tự động (GĐ2):** Servo mở nắp phễu khi ở trạm nạp thức ăn, đóng lại khi di chuyển.

#### 5.3.1. Thiết kế chống kẹt phễu & Recovery

**A. Thiết kế cơ khí chống kẹt:**

Bảng 5.6 — Thông số thiết kế chống kẹt

| **Thông số**            | **Yêu cầu**       | **Lý do**                                       |
|:------------------------|:-------------------|:-------------------------------------------------|
| Góc nghiêng phễu       | ≥ 45°              | Thức ăn viên 3–5mm tự trượt xuống, không đọng    |
| Vật liệu trong phễu    | Inox 304 (hoặc HDPE) | Không thấm nước, chống ẩm >80%, dễ vệ sinh    |
| Đường kính auger screw  | ≥ 20mm            | Đủ lớn cho thức ăn viên 3–5mm, giảm kẹt          |
| Khe hở auger–ống       | 2–3mm              | Tránh nghiền nát thức ăn thành bột                |

**B. Cảm biến phát hiện kẹt:** Đo dòng điện motor rải qua module ACS712 (5A) trên chân ADC ESP32. Dòng bình thường ~0.3–0.5A. Nếu dòng > 1.0A liên tục > 500ms → kẹt cơ học. Phương pháp phụ (tùy chọn): Encoder quang trên trục auger.

**C. Recovery Procedure (Firmware ESP32):**

```
Khi phát hiện kẹt (dòng > 1.0A > 500ms):
  0. Xoay servo hướng ống rải về vị trí NEUTRAL (90° — hướng thẳng xuống)
     để tránh văng thức ăn vào bể lân cận khi đảo chiều. Delay 500ms.
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

Bảng 5.7 — Vị trí lắp đặt thiết bị

| **Thiết bị**                    | **Vị trí**                              | **Vai trò**                                  |
|:-------------------------------|:----------------------------------------|:---------------------------------------------|
| 🍊 Orange Pi 5                | Trên khung robot hoặc gần đỉnh mast    | Brain (ROS2 + AI NPU)                         |
| Camera USB/CSI                 | Đỉnh cột mast (~1.35m), trên đế servo  | Camera AI nhìn xuống bể                       |
| RPLidar A1                     | Trên khung (~25cm), trung tâm           | Quét 360° lập bản đồ, tránh vật cản          |
| Phanh đĩa + Solenoid          | Bracket cạnh hub motor                  | Fail-safe brake: mất điện = phanh tự đóng    |
| Cảm biến IR docking (×2)      | Phía sau robot, 2 bên                   | Căn chỉnh docking (GĐ2)                      |
| Cảm biến mức thức ăn          | Trong phễu                              | Phát hiện khi gần hết thức ăn (GĐ2)          |

### 5.5. Trạm sạc & Trạm nạp thức ăn (GĐ2)

**Trạm sạc:** Tấm tiếp điểm đồng gắn cố định trên tường/chân tường. Robot có 2 tiếp điểm phía sau, lùi vào và tiếp xúc. Beacon IR trên trạm → Cảm biến IR trên robot giúp căn chỉnh ± 2cm.

**Trạm nạp thức ăn:** Phễu lớn cố định trên cao (~1.5m), có van điện solenoid. Robot dừng dưới phễu, mở nắp phễu on-board, mở van trạm → Thức ăn rơi vào phễu robot.

---

## 6. Thiết kế điện – điện tử

### 6.1. Sơ đồ kết nối (Wiring Diagram)

Sơ đồ dưới đây mô tả toàn bộ kết nối điện giữa các module, từ pin hoverboard đến các thiết bị chấp hành.

```
Pin Hoverboard 36V ────┬──► Board Hoverboard STM32 (firmware Feru FOC)
  (Li-ion, sạc 42V)    │     ├──► Motor BLDC trái (36V, hall sensor)
                        │     └──► Motor BLDC phải (36V, hall sensor)
                        │
                        ├──► Mạch Buck 12V ──► Solenoid phanh đĩa (×1 hoặc ×2)
                        │                      └──► Motor rải thức ăn (+ ACS712 đo dòng)
                        │
                        ├──► Mạch Buck 5V-10A ──► ORANGE PI 5 (Nguồn chính)
                        │   [XL4016/MP2307]       └──╸ 2×1000µF 10V tại OPi5
                        │                         └──► RPLidar A1 (qua USB OPi5)
                        │                         └──► Camera (qua USB OPi5)
                        │
                        ├──► Mạch Buck 5V-5A  ──► ESP32 (3.3V onboard)
                        │                         ├──► Servo #1 (Camera Mast)
                        │                         ├──► Servo #2 (Ống rải)
                        │                         ├──► MPU6050 (IMU - I2C)
                        │                         └──► Cảm biến IR docking (GĐ2)
                        │
ORANGE PI 5 ──[USB Cable]──► RPLidar A1 (Dữ liệu scan tốc độ cao)
ORANGE PI 5 ──[USB/UART]──► ESP32 (Gửi CMD_VEL, nhận RPM+IMU+sensor)
ESP32 ──[UART TX/RX]──► Board Hoverboard (gửi speed_left, speed_right)
```

### 6.2. Bảng phân bổ chân GPIO ESP32

ESP32 đóng vai trò Slave Controller, nhận lệnh từ Orange Pi 5 và điều khiển các thiết bị chấp hành.

Bảng 6.1 — Phân bổ chân GPIO ESP32

| **Chân ESP32**    | **Kết nối tới**              | **Chức năng**                                             |
|:------------------|:-----------------------------|:----------------------------------------------------------|
| GPIO 16 (TX2)     | Board Hoverboard (RX)        | UART gửi lệnh tốc độ motor                               |
| GPIO 17 (RX2)     | Board Hoverboard (TX)        | UART nhận phản hồi (pin, dòng, nhiệt, L_RPM, R_RPM)     |
| GPIO 23           | MOSFET gate (H-bridge)       | Motor rải thức ăn (12V) — H-bridge cho đảo chiều          |
| GPIO 22           | MOSFET gate (H-bridge DIR)   | Chiều quay motor rải (LOW=thuận, HIGH=đảo chiều recovery) |
| GPIO 18           | Servo #1 (PWM)               | Xoay camera trên mast                                     |
| GPIO 19           | Servo #2 (PWM)               | Hướng ống rải thức ăn                                     |
| GPIO 27           | MOSFET gate solenoid         | Điều khiển phanh đĩa (HIGH = nhả phanh)                  |
| GPIO 4            | Voltage divider              | Đo điện áp pin 36V (Dự phòng)                            |
| GPIO 34 (ADC)     | ACS712 (5A) output           | Đo dòng motor rải — phát hiện kẹt phễu                    |
| GPIO 32, 33       | IR Sensor #1, #2 (GĐ2)      | Căn chỉnh docking                                         |
| GPIO 21 (SDA)     | MPU6050 IMU                  | Dữ liệu IMU cho EKF Fusion                               |
| GPIO 22 (SCL)     | MPU6050 IMU                  | Dữ liệu IMU cho EKF Fusion                               |
| GPIO 35 (ADC)     | Cảm biến mức thức ăn (GĐ2)  | Đo mức thức ăn trong phễu                                 |
| USB/UART          | Orange Pi 5                  | Nhận lệnh điều khiển, gửi RPM + IMU data                 |

> 📌 THAM KHẢO — Lidar đã được kết nối trực tiếp vào Orange Pi 5 qua USB. Không còn sử dụng UART ESP32 cho Lidar.

### 6.3. Tính toán nguồn điện

Do Orange Pi 5 có transient load cao khi NPU chạy full, việc thiết kế nguồn điện cần đặc biệt chú trọng.

Bảng 6.2 — Tính toán công suất tiêu thụ

| **Thiết bị**               | **Điện áp** | **Dòng (max)** | **Công suất max** | **Ghi chú**                         |
|:---------------------------|:-----------:|:---------------:|:------------------:|:-------------------------------------|
| Orange Pi 5 + Fan          | 5V          | 4.5A            | 22.5W              | NPU full load hút 4–5A transient    |
| RPLidar A1 (qua OPi)      | 5V          | 0.5A            | 2.5W               | Khởi động motor quét cần dòng cao   |
| Camera USB                 | 5V          | 0.5A            | 2.5W               |                                       |
| Hệ thống servo/ESP32      | 5V          | 2.0A            | 10W                | Nuôi 2 servo, ESP32                  |
| **Tổng tải 5V**           |             | **~7.5A**       | **37.5W**          | Tăng do transient load               |
| 2x Motor BLDC (Board)     | 36V         | 2A              | 72W                | Tải nhẹ                              |
| Solenoid + Motor rải       | 12V         | 2A              | 24W                | Dùng Buck 12V-3A riêng              |

> ⚠️ CẢNH BÁO — Phải sử dụng Mạch Buck 5V dòng tối thiểu **5A** (tốt nhất **10A**) cấp riêng cho cụm OPi5 + Lidar + Camera. ESP32 và Servo nên dùng chung nguồn 5V nhưng có tụ lọc lớn hoặc Buck riêng.

**Yêu cầu kỹ thuật mạch Buck 5V cho OPi5:**

Bảng 6.3 — Spec mạch Buck cho OPi5

| **Thông số**        | **Yêu cầu tối thiểu**      | **Lý do**                                      |
|:--------------------|:----------------------------|:------------------------------------------------|
| Ripple voltage      | ≤ 50mV peak-to-peak        | OPi5 nhạy với ripple, crash nếu >100mV         |
| Transient response  | ≤ 100mV khi load step 0→3A | NPU idle→full load gây đột biến dòng            |
| Hiệu suất          | ≥ 85% tại 3A output        | Giảm nhiệt, kéo dài pin                         |
| Dòng output max     | ≥ 10A                      | Headroom cho OPi5 + Lidar + Camera               |

Bảng 6.4 — Module Buck đề xuất

| **Module**          | **Chip**    | **Dòng max**    | **Giá (ước)** | **Ghi chú**                               |
|:--------------------|:------------|:---------------:|:-------------:|:-------------------------------------------|
| XL4016 based        | XL4016E1    | 8A (12A peak)   | ~200–250k     | **Khuyến nghị**, ripple thấp, input 8–40V |
| MP2307 based        | MP2307DN    | 3A (5A peak)    | ~150–200k     | Nhỏ gọn, hiệu suất 95%, cần 2 module     |
| LM2596S             | LM2596      | 3A              | ~80–100k      | Ripple ~200mV — **KHÔNG DÙNG cho OPi5**   |

**Tụ lọc đầu ra (bắt buộc):** 2× tụ điện phân 1000µF 10V mắc song song ngay tại chân nguồn OPi5 + 1× tụ gốm 100nF song song lọc nhiễu cao tần. Mục đích: xử lý transient load khi NPU chuyển idle → full load (0→4.5A trong <1ms).

**Thời lượng pin:**
- Pin gốc ~4.4Ah: 36V × 4.4Ah = 158Wh ÷ 37.5W ≈ **~4 giờ**.
- Nâng cấp 6Ah: 36V × 6Ah = 216Wh ÷ 37.5W ≈ **~5.7 giờ**.
- Nâng cấp 10Ah: 36V × 10Ah = 360Wh ÷ 37.5W ≈ **~9.6 giờ**.
- Với auto-docking (GĐ2): hoạt động **liên tục 24/7**.

**Ghi chú cân chỉnh (Calibration):**

1. **Wheelbase L:** Sau khi lắp khung, dùng thước đo chính xác khoảng cách giữa tâm 2 bánh chủ động. Tuyệt đối không dùng giá trị lý thuyết để tránh sai số quay (yaw drift).
2. **Wheel Radius R:** Điều khiển robot đi thẳng 2m trên sàn trang trại. Đo quãng đường thực tế và điều chỉnh hệ số trong code để sai số < 1%.
3. **IMU Bias:** Khi robot khởi động, giữ đứng yên 5 giây để ESP32 lấy giá trị trung bình Gyro Offset.
4. **IMU Orientation:** Đảm bảo trục X của MPU6050 trùng với hướng tiến của robot.


---

## 7. Thiết kế phần mềm

### 7.1. Firmware ESP32 (Arduino C++) — Low Level Control

ESP32 đóng vai trò Slave, thực hiện các lệnh từ Orange Pi 5 và quản lý toàn bộ tương tác với phần cứng cấp thấp.

Các chức năng chính:
- Nhận lệnh vận tốc (`cmd_vel`) từ ROS2 (qua UART/USB Serial).
- Gửi lệnh điều khiển xuống Board Hoverboard.
- Điều khiển Servo, Solenoid, Motor rải.
- Gửi dữ liệu cảm biến (Pin, IR, trạng thái phanh) ngược lại OPi5.
- **Đọc MPU6050 IMU:** Dữ liệu 6 trục (accel, gyro) qua I2C mỗi 10ms (100Hz). Dùng thư viện `Adafruit_MPU6050`.
- **Quản lý Timestamp:** Mỗi packet gửi lên OPi5 đính kèm `TIMESTAMP_MS` (từ `millis()`). Odom publisher dùng giá trị này để tính `dt` chính xác, triệt tiêu jitter do Linux. Xử lý rollover `millis()` bằng so sánh hiệu số unsigned long.
- **Phát hiện & Recovery kẹt phễu:** Trước khi đảo chiều, xoay servo 90° (Neutral). Đọc ADC ACS712 kiểm tra dòng motor rải.

Tần suất gửi data: IMU (100Hz), RPM Odometry (50Hz), Sensor data (10Hz).

### 7.2. Phần mềm trên Orange Pi 5 (Ubuntu + ROS2)

Hệ thống chạy trên nền tảng ROS2 Humble, tận dụng sức mạnh cộng đồng và tính ổn định công nghiệp. Mỗi node thực hiện một chức năng riêng biệt, giao tiếp qua hệ thống topic và service chuẩn ROS2.

Bảng 7.1 — Kiến trúc ROS2 Nodes

| **Node Name**            | **Chức năng**                                                             | **Package**                |
|:-------------------------|:--------------------------------------------------------------------------|:---------------------------|
| `odom_publisher_node`    | Đọc Serial Packet, tính odometry dùng timestamp ESP32, publish `/odom`    | Tự viết (Python)           |
| `imu_publisher_node`     | Parse packet IMU, publish `sensor_msgs/Imu` lên `/imu/data`              | Tự viết (Python)           |
| `ekf_node`               | Fuse `/odom` + `/imu/data` + `/aruco_pose_corrected` → `/odometry/filtered` | `robot_localization`     |
| `serial_bridge_node`     | Giao tiếp Serial với ESP32, publish `/cmd_vel` xuống                      | Tự viết (Python)           |
| `sllidar_node`           | Driver RPLidar A1, publish `/scan`                                        | `sllidar_ros2`             |
| `usb_cam_node`           | Driver Camera, publish `/image_raw`                                       | `usb_cam`                  |
| `slam_toolbox`           | SLAM, tạo bản đồ `/map`, publish TF `map → odom`                         | `slam_toolbox`             |
| `nav2_bringup`           | Dẫn đường tự động, tránh vật cản (yêu cầu `/odometry/filtered`)          | `nav2`                     |
| `yolo_detect_node`       | AI Vision nhận diện cá/bọt khí (NPU RKNN)                                | `rknn_yolo_python`         |
| `feeding_analyzer_node`  | Optical flow phân tích hành vi ăn mồi (Section 3.1)                      | Tự viết (Python)           |
| `aruco_detector_node`    | Phát hiện ArUco marker, publish `/aruco_pose_corrected` (`PoseWithCovarianceStamped`) về EKF | `ros2_aruco` |
| `sifar_brain_node`       | Logic chính: Tuần tra, Cho ăn, Xử lý sự kiện                             | Tự viết (Python)           |

**AI Vision (YOLOv8 + Rockchip NPU):**

Model YOLOv8 được convert sang định dạng `.rknn` để chạy trên NPU 6 TOPS của RK3588S — tốc độ dự kiến 30–60 FPS, so với 5–10 FPS trên CPU. Độ trễ cực thấp, không ảnh hưởng đến CPU chạy SLAM.

**Kế hoạch Dataset thực tế (Bootstrap & Fine-tune):**

Do công sức label lớn (~3000 ảnh), việc thu thập được chia làm 2 giai đoạn:

1. **GĐ Bootstrap (Tuần 1–4):** Tìm kiếm và label ~300 ảnh từ public datasets (Roboflow Universe — keywords: "fish aquaculture top-down"). Mục tiêu: Đủ để train model chạy demo (Accuracy ≥50%).

> ⚠️ CẢNH BÁO — **License dataset:** Ưu tiên dataset có license CC BY 4.0 hoặc CC0 (public domain). Kiểm tra license trên Roboflow trước khi download. Nếu dataset có NonCommercial restriction → chỉ dùng cho prototype, phải thay thế bằng data tự thu thập trước khi thương mại hóa.

2. **GĐ Fine-tune (Tuần 9–12):** Robot đã hoạt động tuần tra → tự động chụp ảnh mỗi 10s. Thu thập ~1000 ảnh thực tế, label 50 ảnh/ngày. Model đạt Production-ready (Accuracy ≥80%).

**Chiến lược SLAM cho môi trường bể lặp lại:**

Bể nuôi có hình dạng lặp lại hoàn toàn, gây nguy cơ perceptual aliasing — slam_toolbox nhầm lẫn vị trí vì các bể trông giống hệt nhau.

**A. Landmark nhân tạo — ArUco Marker:** Đặt ít nhất 4 ArUco marker (6×6, kích thước 15×15cm) tại 4 góc trang trại. Thêm marker tại mỗi đầu dãy bể (mỗi 5–10 bể đặt 1 marker trên tường/cột). Marker phải có ID duy nhất, in trên vật liệu chống nước (nhựa PP/acrylic).

**B. Cấu hình slam_toolbox:**

Bảng 7.2 — Cấu hình slam_toolbox

| **Tham số**                           | **Giá trị** | **Lý do**                                            |
|:--------------------------------------|:------------|:------------------------------------------------------|
| Mode                                 | `localization` / `mapping` | Không dùng `lifelong` — tránh aliasing       |
| `loop_search_maximum_distance`        | 3.0m        | Giới hạn phạm vi loop closure, tránh match nhầm bể xa|
| `loop_match_minimum_response_coarse`  | 0.7         | Yêu cầu match score cao hơn default (0.35)           |
| `loop_match_minimum_response_fine`    | 0.7         | Tương tự (default 0.45)                               |
| `minimum_travel_distance`             | 0.5m        | Chỉ thêm scan mới khi đã di chuyển đủ xa            |
| `do_loop_closing`                     | false       | Tắt loop closure tự động, dùng ArUco thay thế        |

### 7.3. Triết lý phát triển

- **Standardization:** Dùng chuẩn ROS2 — dễ tích hợp, debug, nâng cấp.
- **Hardware Acceleration:** Tận dụng tối đa NPU, GPU của Orange Pi 5.
- **Modular:** Tách biệt rõ ràng Low-level (ESP32) và High-level (OPi5).

### 7.4. Giao thức giao tiếp (Serial Packet)

Giao tiếp giữa OPi5 và ESP32 qua cổng USB Serial (baudrate 115200):

- **OPi5 gửi:** `<CMD, VAL1, VAL2>` — Ví dụ: `<VEL, 0.2, 0.1>` cho vận tốc dài 0.2m/s, góc 0.1rad/s.
- **ESP32 gửi:**
  - `<FB, TIMESTAMP_MS, BAT_V, L_RPM, R_RPM, FEEDER_I>` — Phản hồi pin, tốc độ bánh trái/phải, dòng motor rải.
  - `<IMU, TIMESTAMP_MS, AX, AY, AZ, GX, GY, GZ>` — Dữ liệu gia tốc và vận tốc góc raw.

---

## 8. Danh sách vật tư & Dự toán (BOM)

### 8.1. Giai đoạn 1 — MVP (Smart Robot với Orange Pi 5)

Bảng 8.1 — BOM Giai đoạn 1

| **STT** | **Linh kiện**                              | **SL** | **Đơn giá (VNĐ)** | **Thành tiền** | **Vai trò**                        |
|:-------:|:-------------------------------------------|:------:|-------------------:|---------------:|:-----------------------------------|
| 1       | Orange Pi 5 (8GB RAM)                     | 1      | 2.500.000          | 2.500.000      | Brain (ROS2 + AI NPU)              |
| 2       | Tản nhiệt + Quạt cho OPi5                | 1      | 150.000            | 150.000        | Làm mát CPU/NPU                    |
| 3       | Camera USB góc rộng (120°)                | 1      | 300.000            | 300.000        | Vision AI                          |
| 4       | Thẻ nhớ MicroSD 64GB Class 10            | 1      | 150.000            | 150.000        | Cài OS Ubuntu                      |
| 5       | Hoverboard cũ 10" (motor, board, pin)     | 1      | 1.500.000          | 1.500.000      | Hệ dẫn động                        |
| 6       | ST-Link V2                               | 1      | 100.000            | 100.000        | Flash firmware                      |
| 7       | ESP32 Dev Kit V1                          | 1      | 150.000            | 150.000        | Low-level Controller                |
| 8       | RPLidar A1 (Slamtec)                      | 1      | 2.800.000          | 2.800.000      | SLAM & Avoidance                    |
| 9       | Mạch Buck DC-DC 5V 10A (XL4016)          | 1      | 250.000            | 250.000        | Nguồn cho OPi5 & Lidar             |
| 10      | Servo SG90                               | 2      | 50.000             | 100.000        | Xoay cam, ống rải                   |
| 11      | Nhôm 2020 + Khung tự chế + Caster        | 1 bộ   | 1.000.000          | 1.000.000      | Cơ khí robot                        |
| 12      | Hệ thống phanh (Caliper + Solenoid)      | 1 bộ   | 300.000            | 300.000        | Phanh Fail-safe                     |
| 13      | Hệ thống rải (Motor + Phễu inox 304)     | 1 bộ   | 900.000            | 900.000        | Bộ cho ăn (phễu inox)              |
| 14      | Mạch Buck 12V + MOSFET + Dây             | 1 bộ   | 300.000            | 300.000        | Phụ kiện điện                       |
| 15      | Tụ lọc 1000µF 10V + 100nF gốm           | 1 bộ   | 20.000             | 20.000         | Tụ lọc transient cho OPi5           |
| 16      | Module ACS712 (5A)                        | 1      | 30.000             | 30.000         | Đo dòng motor rải — phát hiện kẹt   |
| 17      | ArUco Marker (in A4 laminate ×8)          | 8      | 6.000              | 50.000         | Landmark cho SLAM                   |
| 18      | Module MPU6050 (IMU)                      | 1      | 40.000             | 40.000         | IMU fusion cho odometry             |
|         | **TỔNG CỘNG GĐ1**                        |        |                    | **~10.610.000** |                                    |

> 💡 LƯU Ý — Chi phí GĐ1 tăng ~3 triệu so với phương án dùng điện thoại cũ, nhưng đổi lại hệ thống chuẩn công nghiệp (ROS2), ổn định 24/7, không lo sự cố pin điện thoại, và AI mạnh gấp 10 lần. Đây là sự đầu tư xứng đáng cho dài hạn.

### 8.2. Giai đoạn 2 — Nâng cấp (tùy chọn)

Bảng 8.2 — BOM Giai đoạn 2

| **STT** | **Linh kiện**                                            | **Giá (VNĐ)** | **Lợi ích**                   |
|:-------:|:---------------------------------------------------------|---------------:|:-------------------------------|
| 1       | Màn hình 5-7 inch HDMI (gắn lên robot)                  | 800.000        | Hiển thị trực quan tại chỗ    |
| 2       | Trạm sạc (tiếp điểm đồng + beacon IR + mạch sạc 42V)   | 800.000        | Auto-docking sạc pin           |
| 3       | Cảm biến IR (cho docking) × 2                           | 60.000         | Căn chỉnh docking ± 2cm       |
| 4       | Trạm nạp thức ăn (phễu lớn + van solenoid)              | 600.000        | Auto-refill thức ăn            |
| 5       | Camera hồng ngoại IR (Vision ban đêm)                    | 800.000        | Thay camera thường nếu cần    |
| 6       | Pin Li-ion 36V 10Ah+                                    | 1.500.000      | Tăng thời gian hoạt động      |
|         | **TỔNG CỘNG GĐ2 (thêm)**                               |                | **~4.560.000**                 |


---

## 9. Quy trình chế tạo & Tích hợp

### 9.1. Giai đoạn 1: Flash firmware Hoverboard + Dựng khung Robot (Tuần 1–4)

Bảng 9.1 — Quy trình GĐ1

| **Bước** | **Công việc**                                                                                              | **Tiêu chí hoàn thành**                                    |
|:--------:|:-----------------------------------------------------------------------------------------------------------|:------------------------------------------------------------|
| 1.1      | Mua hoverboard cũ 10", kiểm tra theo checklist Section 5.1.2 (chip STM32, pinout UART, ST-Link)           | Motor quay đều, pin giữ điện, chip STM32F103 xác nhận      |
| 1.2      | Tháo vỏ nhựa, đế đứng, cảm biến nghiêng                                                                  | Giữ lại board + motor + pin + sạc                           |
| 1.3      | Flash firmware Emanuel Feru qua ST-Link V2. Nếu thất bại → kích hoạt Plan B (Section 5.1.3)               | Board nhận lệnh UART, motor quay được                       |
| 1.4      | Chế khung nhôm/thép (~60×70cm), gắn hoverboard vào giữa                                                   | Khung chắc, 4 điểm tiếp đất ổn định                        |
| 1.5      | Gắn 2 bánh caster phía trước + phanh đĩa fail-safe                                                        | Phanh tự đóng khi ngắt điện, nhả khi cấp điện              |
| 1.6      | Nối ESP32 → Board HB (UART), test điều khiển từ OPi5 qua WiFi                                             | Điều khiển W/A/S/D từ OPi5 qua WiFi                        |
| 1.7      | Dựng cột mast nhôm 2020 (~1.0m), gắn servo + Camera USB/CSI                                               | Cột không rung, camera nhìn rõ vào bể                       |
| 1.8      | Test di chuyển trong lối đi hẹp ~80cm                                                                      | Robot quay tại chỗ, không va bể                             |

### 9.2. Giai đoạn 2: SLAM + Camera AI (Tuần 5–8)

Bảng 9.2 — Quy trình GĐ2

| **Bước** | **Công việc**                                                                              | **Tiêu chí hoàn thành**                              |
|:--------:|:-------------------------------------------------------------------------------------------|:------------------------------------------------------|
| 2.1      | Gắn RPLidar A1 (→ OPi5 USB), cài đặt `sllidar_ros2`                                     | Bản đồ 2D trang trại hiển thị đúng                    |
| 2.1a     | Quy trình Mapping (xem chi tiết bên dưới)                                                 | ArUco detect OK, TF map→odom được sửa lỗi liên tục   |
| 2.2      | Test Camera USB/CSI trên mast, servo xoay                                                  | Camera nhìn rõ vào bể ở cả 2 bên                     |
| 2.3      | Train model Bootstrap từ public data (Tuần 1-4)                                           | Accuracy ≥50% (đủ để demo flow)                       |
| 2.4      | Viết `state_machine.py` + `patrol_manager.py`                                             | Robot tuần tra tự động qua các bể                     |
| 2.5      | Viết `alert_system.py` (Telegram + loa)                                                    | Gửi cảnh báo + phát âm thanh khi bất thường          |

**Quy trình Mapping chi tiết (Bước 2.1a):**

1. Đặt 6–10 ArUco marker (ghi tọa độ/ID) tại 4 góc + đầu mỗi dãy bể.
2. Chạy slam_toolbox ở chế độ `mapping`.
3. Teleop robot đi hết trang trại 1 vòng.
4. Lưu bản đồ: `ros2 service call /slam_toolbox/serialize_map`.

**Bước 3b — Lưu tọa độ ArUco markers vào file `aruco_map.yaml`:**

Sau khi bản đồ được lưu, cần ghi lại tọa độ tuyệt đối của từng ArUco marker trong map frame để `aruco_detector_node` sử dụng khi tính toán vị trí robot.

Quy trình:
1. Chuyển slam_toolbox sang chế độ `localization`.
2. Điều khiển robot đứng trước từng marker, ghi lại pose từ topic `/tf` (map → base_link) tại thời điểm camera thấy marker.
3. Lưu vào file `aruco_map.yaml` theo format sau:

```yaml
# aruco_map.yaml — Tọa độ tuyệt đối các ArUco marker trong map frame
markers:
  - id: 0
    x: 1.23
    y: 0.45
    yaw: 0.0
  - id: 1
    x: 5.67
    y: 0.45
    yaw: 0.0
  - id: 2
    x: 5.67
    y: 8.90
    yaw: 3.14
  # ... thêm marker theo thứ tự ID
```

4. `aruco_detector_node` load file này khi khởi động để biết tọa độ tuyệt đối của từng marker trong map frame.
5. Khi camera phát hiện marker, node tính ngược pose robot: `robot_pose = marker_pose ⊖ camera_to_marker_tf`, rồi publish lên `/aruco_pose_corrected`.
6. Chuyển slam_toolbox sang chế độ `localization` cho vận hành hàng ngày.

### 9.3. Giai đoạn 3: Cho ăn thông minh + Thử nghiệm (Tuần 9–12)

Bảng 9.3 — Quy trình GĐ3

| **Bước** | **Công việc**                                                  | **Tiêu chí hoàn thành**                  |
|:--------:|:---------------------------------------------------------------|:------------------------------------------|
| 3.1      | Lắp phễu + ống rải + servo hướng, code FeederControl          | Rải đúng vào miệng bể, không rơi ra ngoài|
| 3.2      | Viết `feeding_analyzer.py`: AI quan sát → điều chỉnh lượng    | Robot tự giảm/dừng khi cá bỏ ăn          |
| 3.3      | Code `scheduler.py` (hẹn giờ tuần tra + cho ăn)               | Robot hoạt động tự động đúng lịch         |
| 3.4      | Viết `dashboard.py` (web UI trên OPi5)                        | Bản đồ SLAM + video AI + log             |
| 3.5      | Thử nghiệm 5 ngày tại trang trại                              | Vận hành ổn định, SLAM chính xác          |

### 9.4. Giai đoạn 4: Docking + RAS + Mở rộng (Tuần 13–18, GĐ2)

Bảng 9.4 — Quy trình GĐ4

| **Bước** | **Công việc**                                                            | **Tiêu chí hoàn thành**              |
|:--------:|:-------------------------------------------------------------------------|:--------------------------------------|
| 4.1      | Chế tạo trạm sạc 42V, gắn cảm biến IR, code `docking_manager.py`       | Robot tự lùi vào trạm, sạc tự động   |
| 4.2      | Chế tạo trạm nạp thức ăn, code auto-refill                              | Robot tự nạp đầy phễu                |
| 4.3      | Viết `ras_monitor.py`: AI giám sát bể lọc RAS                           | Phát hiện mức nước thấp, bọt nhiều   |
| 4.4      | (Tùy chọn) Nâng cấp Camera CSI chất lượng cao hơn                       | AI mạnh hơn, multi-camera            |
| 4.5      | Thử nghiệm tích hợp 7 ngày                                              | Toàn bộ hệ thống hoạt động ổn định   |

---

## 10. Kế hoạch thử nghiệm & Nghiệm thu

### 10.1. Bài test chức năng

Bảng 10.1 — Bài test chức năng

| **TC** | **Tên test**                              | **Điều kiện đạt**                                                 |
|:------:|:------------------------------------------|:-------------------------------------------------------------------|
| TC-01  | Di chuyển trong lối đi hẹp               | Không va chạm bể, quay đầu mượt                                   |
| TC-02  | Đi thẳng 5 mét                           | Sai lệch ngang ≤ 5cm                                              |
| TC-03  | Tránh vật cản (Lidar)                     | Dừng cách vật cản 30–50cm, 10/10 lần                             |
| TC-04  | Bản đồ SLAM trang trại                   | Lập bản đồ chính xác, sai lệch ≤ 10cm                            |
| TC-05  | Camera xoay nhìn vào bể                  | Servo xoay chính xác ± 5°, hình ảnh rõ                            |
| TC-06  | Rải thức ăn vào bể                       | 90% thức ăn rơi vào miệng bể                                      |
| TC-07a | AI nhận diện cá (Bootstrap — Tuần 8)     | ≥ 50% mAP@0.5 với dataset bootstrap, ≥ 10 FPS                    |
| TC-07b | AI nhận diện cá (Fine-tuned — Tuần 16)   | ≥ 80% mAP@0.5 sau fine-tune với data thực, ≥ 30 FPS              |
| TC-08  | Phân tích hành vi cho ăn                  | Phân biệt đúng cá ăn tích cực vs bỏ ăn                           |
| TC-09  | Tuần tra tự động                          | Hoàn thành tất cả bể đúng thứ tự                                  |
| TC-10  | Cảnh báo Telegram                        | Gửi thông báo ≤ 5 giây sau khi phát hiện                          |
| TC-11  | Pin yếu                                  | Cảnh báo khi < 20%, tự dừng khi < 10%                            |
| TC-12  | Mất WiFi                                | Tự dừng sau 10 giây mất kết nối                                   |
| TC-13  | Auto-docking sạc pin (GĐ2)              | Căn chỉnh ≤ 2cm, sạc tự động                                      |
| TC-14  | Auto-refill thức ăn (GĐ2)               | Nạp đầy phễu, không tràn                                          |
| TC-15  | Giám sát RAS (GĐ2)                      | Phát hiện mức nước thấp ≥ 90% trường hợp                         |

### 10.2. Bài test tích hợp tại thực địa

Chạy liên tục 4 giờ trong trang trại thực tế. Tuần tra 20+ bể, cho ăn 3 lần/ngày trong 5 ngày. Ghi nhận: độ chính xác rải thức ăn, false alarm rate, thời gian tuần tra, sự cố.

---

## 11. Vận hành & Bảo trì

### 11.1. Hướng dẫn vận hành hàng ngày

1. **Sạc pin** đầy (hoặc để robot tự sạc tại trạm nếu GĐ2).
2. **Đổ thức ăn** vào phễu (hoặc đổ vào trạm nạp nếu GĐ2).
3. **Bật robot**, kiểm tra WiFi trên Dashboard.
4. **Cài lịch** tuần tra + cho ăn hoặc chạy thủ công.
5. Robot sẽ tự hoạt động theo lịch và gửi cảnh báo khi cần.

### 11.2. Bảo trì định kỳ

Bảng 11.1 — Lịch bảo trì

| **Tần suất** | **Công việc**                                                            |
|:-------------|:-------------------------------------------------------------------------|
| Hàng ngày    | Lau khô phễu, kiểm tra bánh xe (không dính rác)                         |
| Hàng tuần    | Kiểm tra dây nối, siết ốc cột mast, kiểm tra phanh đĩa + cáp phanh    |
| Hàng tháng   | Lau ống kính Lidar + Camera, kiểm tra solenoid phanh                    |
| 6 tháng      | Kiểm tra vòng bi bánh caster, tiếp điểm sạc, cập nhật firmware          |

---

## 12. Phân tích rủi ro kỹ thuật

Bảng 12.1 — Ma trận rủi ro kỹ thuật

| **Rủi ro**                           | **Xác suất** | **Hậu quả**    | **Biện pháp phòng ngừa**                                                                                 |
|:--------------------------------------|:------------:|:---------------:|:-----------------------------------------------------------------------------------------------------------|
| Nước bắn vào mạch điện              | Trung bình   | Nghiêm trọng   | IP54 bọc kín, cáp chống ẩm, đặt mạch trên cao                                                            |
| Pin Li-ion cháy/phồng                | Thấp         | Nguy hiểm       | BMS bảo vệ, không sạc quá 90%, tránh va đập                                                               |
| Mất WiFi trong trang trại            | Thấp         | Trung bình      | Repeater, ESP32 tự dừng + phanh đĩa sau 10s                                                               |
| Motor BLDC quá nhiệt                 | Rất thấp     | Nhẹ             | Robot 30kg vs tải motor 100kg, tốc độ thấp → motor dùng <5% công suất                                     |
| Robot va chạm bể                     | Trung bình   | Nhẹ–TB          | Lidar phát hiện, tốc độ thấp + phanh fail-safe, bumper foam                                               |
| Thức ăn rơi ngoài bể                 | Trung bình   | Nhẹ             | Ống dẫn + servo căn chỉnh hướng, test từng bể                                                             |
| Cột mast rung lắc                    | Trung bình   | Nhẹ             | Ke góc kép + thanh giằng chéo + guy wire. Moment lật max 1.35 N·m (Section 5.2)                           |
| Docking lệch (GĐ2)                  | Trung bình   | Nhẹ             | 2 cảm biến IR, thuật toán PID căn chỉnh                                                                   |
| Ăn mòn do hóa chất/muối             | Trung bình   | TB              | Khung nhôm/thép xử lý chống rỉ, phủ sơn                                                                  |
| Flash firmware hoverboard thất bại   | 20–30%       | Cao             | Checklist 5 bước TRƯỚC mua (Section 5.1.2). Plan B: ODrive/SimpleFOC (+500–800k). Không ảnh hưởng kiến trúc còn lại |
| Kẹt phễu thức ăn                    | Trung bình   | TB              | Phễu inox 304 góc ≥45°, auger ≥20mm. ACS712 phát hiện. Recovery đảo chiều 2s. Telegram alert              |
| Thức ăn văng khi recovery            | Thấp         | Nhẹ             | Xoay servo về Neutral trước khi đảo chiều auger                                                           |
| SLAM aliasing (bể lặp lại)          | Trung bình   | Cao             | ArUco marker ≥4 góc + đầu dãy bể. Localization mode, tắt loop closure tự động                             |
| OPi5 crash do nguồn                 | Thấp         | Cao             | Buck XL4016 (ripple ≤50mV) + 2×1000µF tụ lọc transient. Không dùng LM2596                                |

---

## 13. Lộ trình phát triển

```
GĐ1 — MVP (Nền tảng ROS2 mạnh mẽ)             GĐ2 — Tự động hóa hoàn toàn
───────────────────────────────────           ────────────────────────────
✓ Hoverboard cũ + khung tự chế + Phanh        → Auto-docking sạc pin tự động
✓ Orange Pi 5 (Ubuntu + ROS2 Humble)          → Auto-refill thức ăn
✓ RPLidar A1 (USB) + SLAM Toolbox             → Giám sát bể lọc RAS
✓ Camera AI (YOLOv8 Rockchip NPU)             → Pin Li-ion 36V dung lượng cao
✓ ESP32 Slave Control + IMU MPU6050           → Cảm biến chất lượng nước
✓ EKF Sensor Fusion (odom + IMU + ArUco)
✓ Cho ăn thông minh từng bể
✓ Dashboard ROS2 (Web based)
```

---

## 14. Phụ lục — Hỏi đáp kỹ thuật cho Hội đồng thẩm định

**Q1: Tại sao robot hoạt động trong nhà thay vì ngoài ao?**
> Trang trại RAS trong nhà là xu hướng nuôi trồng thủy sản hiện đại: kiểm soát môi trường tốt hơn, mật độ nuôi cao hơn, ít phụ thuộc thời tiết. Số lượng bể nhiều (hàng chục đến hàng trăm) khiến giám sát thủ công rất tốn công sức. Robot SIFAR giải quyết chính xác bài toán này.

**Q2: Camera gắn trên cột mast có ổn định không?**
> Cột mast bằng nhôm 2020 được cố định bằng ke góc và thanh giằng chéo. Servo xoay camera SG90 nhẹ (9g), không tạo rung lắc đáng kể. Robot di chuyển tốc độ thấp (≤ 0.5m/s) trên nền phẳng nên cột rất ổn định. Moment lật tối đa chỉ 1.35 N·m (Section 5.2).

**Q3: Robot rải thức ăn vào bể nhỏ như thế nào?**
> Thức ăn viên từ phễu đi qua trục vít → ống dẫn nhựa mềm dài ~30cm hướng vào miệng bể. Servo điều chỉnh góc ống. Robot dừng tại waypoint SLAM trước mỗi bể. Cơ chế phát hiện kẹt phễu + recovery tự động đảm bảo độ tin cậy (Section 5.3).

**Q4: Auto-docking hoạt động như thế nào?**
> Trạm sạc có beacon IR. Robot có 2 cảm biến IR phía sau. Khi pin < 20%, robot dùng SLAM di chuyển tới khu vực trạm sạc, dùng tín hiệu IR căn chỉnh ± 2cm và lùi vào tiếp điểm đồng.

**Q5: Hệ thống lọc RAS được giám sát như thế nào?**
> GĐ2: Robot dừng tại các bể lọc (waypoint SLAM), camera AI phân tích: mức nước, bọt/váng, trạng thái máy bơm/UV. Kết quả gửi về Dashboard.

**Q6: Robot có an toàn cho người không?**
> Lidar quét 360° phát hiện người từ 2–5m, robot tự dừng khi vật cản < 50cm. Tốc độ tối đa 0.5m/s. Bumper foam quanh đế. Module SafetyMonitor trên ESP32 xử lý khẩn cấp cục bộ. Phanh đĩa fail-safe tự đóng khi mất điện.

**Q7: Chi phí GĐ1 ~10 triệu có cao không?**
> Mức giá cực kỳ rẻ so với robot công nghiệp (50–200 triệu). Đầu tư thêm ~3 triệu cho Orange Pi 5 là khoản đầu tư cho sự ổn định cốt lõi — nếu dùng điện thoại mà hệ thống chập chờn, chi phí bảo trì và rủi ro vận hành sẽ còn tốn kém hơn.

**Q8: Tại sao chọn Orange Pi 5 + ROS2 thay vì Điện thoại Android?**
> Ba điểm yếu chí mạng của Android trong môi trường công nghiệp: (1) Nhiệt độ & Pin — chạy 24/7 cắm sạc liên tục làm pin phồng, OPi5 dùng nguồn trực tiếp; (2) Băng thông Lidar — truyền qua WiFi gây trễ và rớt gói tin, OPi5 có USB 3.0 trực tiếp; (3) Chuẩn hóa — ROS2 là tiêu chuẩn toàn cầu, tiếp cận hàng ngàn package có sẵn.

---

## 15. Phản hồi Hội đồng Thẩm định

Tài liệu SIFAR đã trải qua 4 vòng thẩm định từ phiên bản v3.0 đến v5.0. Mỗi vòng phản biện đều được ghi nhận và giải quyết toàn bộ trong các phiên bản tiếp theo. Quá trình này giúp tài liệu không ngừng hoàn thiện — từ một bản kế hoạch sơ bộ dùng điện thoại Android (v3.0) thành một thiết kế kỹ thuật hoàn chỉnh, chuẩn công nghiệp với ROS2, EKF sensor fusion, và ArUco visual correction (v5.0).

Bảng 15.1 — Tổng hợp vấn đề phản biện qua 4 vòng thẩm định

| **#** | **Vấn đề**                                  | **Mức độ**   | **Giải pháp đã bổ sung**                                                                                    | **Phiên bản giải quyết** | **Sections** |
|:-----:|:---------------------------------------------|:------------|:-------------------------------------------------------------------------------------------------------------|:------------------------:|:------------|
| 1     | Odometry cho Nav2 — Thiếu `/odom` topic      | CRITICAL    | Công thức odometry từ Hall RPM. Node `odom_publisher_node`. Serial Packet. ESP32 gửi RPM 50Hz                | v4.1                     | 4.2, 7.1, 7.4 |
| 2     | Dataset AI & Thuật toán chưa định nghĩa      | CRITICAL    | Kế hoạch 3000 ảnh/6 class. YOLOv8n + augmentation. `feeding_analyzer.py` optical flow + 3 ngưỡng             | v4.1                     | 3.1, 7.2     |
| 3     | Plan B Hoverboard — 20-30% board clone       | CRITICAL    | Checklist 5 bước. Plan B: ODrive/SimpleFOC. Quy trình chuyển đổi                                            | v4.1                     | 5.1, 12      |
| 4     | SLAM Aliasing — Bể lặp lại                  | QUAN TRỌNG  | ArUco marker ≥4 góc + đầu dãy. slam_toolbox localization mode, tắt loop closure                             | v4.1                     | 7.2, 9       |
| 5     | Nguồn 5V OPi5 — Transient crash             | QUAN TRỌNG  | Buck XL4016, ripple ≤50mV. 2×1000µF + 100nF tụ lọc tại OPi5                                                 | v4.1                     | 6.1, 6.3     |
| 6     | Kẹt phễu thức ăn — Không phát hiện/recovery  | QUAN TRỌNG  | Phễu inox 304, ACS712 đo dòng, recovery đảo chiều, Telegram alert                                           | v4.1                     | 5.3, 7.1     |
| 7     | Moment lật cột mast chưa tính toán          | QUAN TRỌNG  | Moment max 1.35 N·m. Ke góc kép + giằng chéo + guy wire                                                     | v4.1                     | 5.2          |
| 8     | Odometry Drift — Chỉ RPM tích lũy lỗi      | CRITICAL    | Thêm MPU6050 IMU. `robot_localization` EKF fuse Wheel Odom + IMU                                            | v4.2                     | 4.2, 6.2, 7.2 |
| 9     | Nav2 integration — Luồng ArUco chưa rõ      | CRITICAL    | Sơ đồ luồng topic rõ ràng. Quy trình mapping 5 bước                                                        | v4.2                     | 4.2, 7.2, 9 |
| 10    | Dataset timeline — 3000 ảnh quá tải         | QUAN TRỌNG  | Tách 2 pha: Bootstrap (public) + Fine-tune (tự động chụp). TC-07a/b                                         | v4.2                     | 7.2, 10.1    |
| 11    | Timestamp jitter — OPi5 thiếu real-time     | QUAN TRỌNG  | `TIMESTAMP_MS` từ ESP32. EKF dùng timestamp MCU tính dt                                                     | v4.2                     | 7.1, 7.4     |
| 12    | Recovery Safety — Văng thức ăn khi đảo chiều | QUAN TRỌNG  | Xoay servo Neutral (90°) trước khi đảo chiều                                                                | v4.2                     | 5.3, 12      |
| 13    | BOM Consistency                              | NHỎ         | Gộp BOM nhất quán, thêm item IMU MPU6050                                                                    | v4.2                     | 8            |
| 14    | ArUco Topic Type mismatch                    | CRITICAL    | Đổi sang `PoseWithCovarianceStamped`. Cập nhật ekf.yaml với pose0                                           | v5.0                     | 4.2, 7.2     |
| 15    | TF conflict — EKF vs slam_toolbox           | CRITICAL    | Phân tách TF responsibility. Bảng publisher rõ ràng                                                          | v5.0                     | 4.2          |
| 16    | Thiếu aruco_map.yaml                         | QUAN TRỌNG  | Thêm Bước 3b lưu tọa độ marker. Format YAML chuẩn                                                          | v5.0                     | 9            |

Bảng 15.2 — Scorecard so sánh v3.0 → v5.0

| **Tiêu chí**                    | **v3.0 (Android)**  | **v5.0 (OPi5 + ROS2)**                  |
|:---------------------------------|:-------------------:|:-----------------------------------------|
| Bộ xử lý trung tâm              | Điện thoại Android  | Orange Pi 5 (8GB, NPU 6 TOPS)           |
| SLAM Framework                  | Python tự viết      | slam_toolbox (chuẩn công nghiệp)        |
| Navigation                      | Không có             | Nav2 (costmap + planner + controller)    |
| Odometry                        | Không có             | Wheel + IMU + ArUco EKF Fusion           |
| AI Framework                    | TFLite trên Android  | YOLOv8 RKNN (NPU, 30-60 FPS)            |
| Giao tiếp phần cứng            | WiFi → ESP32        | USB/UART trực tiếp                       |
| Độ ổn định 24/7                 | Thấp (phồng pin)    | Cao (nguồn trực tiếp, tản nhiệt)        |
| Anti-drift                      | Không                | EKF + ArUco marker correction            |
| Anti-aliasing SLAM              | Không                | ArUco landmark + tuned parameters        |
| Phanh an toàn                    | Không                | Fail-safe disc brake                     |
| Phát hiện kẹt phễu              | Không                | ACS712 + recovery tự động                |
| Chi phí GĐ1                     | ~7 triệu            | ~10.6 triệu                             |

---

> **Tài liệu tham khảo:** ESP32 Docs (espressif.com) · Arduino Reference · OpenCV · Ultralytics YOLOv8 · RPLidar SDK (Slamtec) · Emanuel Feru Hoverboard Firmware (GitHub) · Recirculating Aquaculture Systems (FAO) · OpenCV ArUco Module · slam_toolbox Documentation (Steve Macenski) · ODrive Documentation · SimpleFOC Library · robot_localization (Tom Moore) · ROS2 Humble Documentation · Nav2 Documentation
