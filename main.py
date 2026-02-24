from core.state_machine import SIFARStateMachine, RobotState
from core.navigation import NavigationManager
import time

def main():
    print("--- SIFAR Integration Simulation ---")
    sm = SIFARStateMachine()
    nav = NavigationManager()
    
    # 1. Start Patrol
    print("\n[HÀNH ĐỘNG] Bắt đầu tuần tra các bể...")
    sm.transition_to(RobotState.PATROL_TANKS)
    nav.set_destination("TANK_1")
    
    # Simulate movement
    while not nav.move_step():
        time.sleep(0.1)
    
    # 2. Check RAS
    print("\n[HÀNH ĐỘNG] Chuyển sang kiểm tra hệ thống RAS...")
    sm.transition_to(RobotState.MONITOR_RAS)
    nav.set_destination("RAS_SYSTEM")
    while not nav.move_step():
        time.sleep(0.1)

    # 3. Low Battery event
    print("\n[SỰ KIỆN] Pin yếu (15%)!")
    sm.battery_level = 15
    sm.update() # This will trigger transition to GO_TO_CHARGER
    
    if sm.state == RobotState.GO_TO_CHARGER:
        nav.set_destination("CHARGER")
        while not nav.move_step():
            time.sleep(0.1)
        
        sm.transition_to(RobotState.CHARGING)
        print("Đang sạc...")
        while sm.state == RobotState.CHARGING:
            sm.update()
            time.sleep(0.1)

    print("\n--- Hoàn tất mô phỏng ---")
    print(f"Trạng thái cuối cùng: {sm.state}")

if __name__ == "__main__":
    main()
