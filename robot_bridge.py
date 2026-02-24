import serial
import time

# Cấu hình cổng Serial (Thay đổi COM port phù hợp với máy của bạn)
SERIAL_PORT = 'COM3' 
BAUD_RATE = 115200

def init_bridge():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2) # Chờ khởi động
        print(f"Đã kết nối thành công tới {SERIAL_PORT}")
        return ser
    except Exception as e:
        print(f"Lỗi kết nối: {e}")
        return None

def send_command(ser, cmd):
    if ser:
        ser.write(f"{cmd}\n".encode())
        print(f"Đã gửi lệnh: {cmd}")

if __name__ == "__main__":
    bridge = init_bridge()
    if bridge:
        # Ví dụ: Gửi lệnh 'FORWARD' mỗi 2 giây
        while True:
            send_command(bridge, "FORWARD")
            time.sleep(2)
            send_command(bridge, "STOP")
            time.sleep(2)
