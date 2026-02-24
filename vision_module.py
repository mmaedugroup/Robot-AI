import cv2
import time

class SIFARVision:
    def __init__(self):
        self.mode = "TANK_MONITORING" # Options: TANK_MONITORING, RAS_MONITORING

    def process_frame(self, frame):
        """Placeholder for AI processing logic"""
        if self.mode == "TANK_MONITORING":
            # logic for detecting fish, health, food reaction in 1.2m tanks
            cv2.putText(frame, "Mode: Tank Monitoring - Detecting Fish...", (10, 30), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        elif self.mode == "RAS_MONITORING":
            # logic for detecting filter status, water level, foam
            cv2.putText(frame, "Mode: RAS Monitoring - Checking Filters...", (10, 30), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 0), 2)
        
        return frame

    def start_vision(self):
        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print("Không thể mở Camera.")
            return

        print(f"SIFAR Vision khởi động... Chế độ: {self.mode}")
        print("Nhấn 't' để đổi chế độ Tank, 'r' để đổi chế độ RAS, 'q' để thoát.")

        while True:
            ret, frame = cap.read()
            if not ret: break

            # Xử lý frame
            processed_frame = self.process_frame(frame)

            # Hiển thị
            cv2.imshow('SIFAR Intelligent Vision', processed_frame)

            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                break
            elif key == ord('t'):
                self.mode = "TANK_MONITORING"
            elif key == ord('r'):
                self.mode = "RAS_MONITORING"

        cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    vision = SIFARVision()
    vision.start_vision()
