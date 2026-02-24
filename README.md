# SIFAR - Smart Indoor Farm Assistant Robot

Robot trợ lý thông minh cho trang trại nuôi trồng thủy sản trong nhà (Indoor Farm).

## Chức năng
- **Tuần tra tự động:** Giám sát các bể nuôi cao 1.2m và hệ thống lọc RAS.
- **Cho ăn thông minh:** Tự động rải thức ăn và điều chỉnh lượng dựa trên AI.
- **Quản lý năng lượng:** Tự hành tìm trạm sạc và trạm nạp thức ăn.
- **Phân tích AI:** Nhận diện hành vi cá, trạng thái hệ thống lọc và cảnh báo bất thường.

## Cấu trúc thư mục
- `/core`: Mã nguồn điều khiển trung tâm (State Machine, Navigation).
- `vision_module.py`: Xử lý thị giác máy tính và AI.
- `robot_bridge.py`: Giao tiếp với phần cứng (MCU/ESP32).
- `PLAN.md`: Kế hoạch chi tiết của dự án.

## Bắt đầu
1. Cài đặt thư viện: `pip install -r requirements.txt` (Sắp có)
2. Chạy module vision: `python vision_module.py`
3. Chạy bridge điều khiển: `python robot_bridge.py`
