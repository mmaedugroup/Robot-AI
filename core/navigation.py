class NavigationManager:
    def __init__(self):
        # Coordinates for different locations in the indoor farm
        self.waypoints = {
            "HOME": (0, 0),
            "CHARGER": (1, 1),
            "REFILLER": (-1, 1),
            "RAS_SYSTEM": (5, 0),
            "TANK_1": (2, 2),
            "TANK_2": (2, 4),
            "TANK_3": (2, 6),
        }
        self.current_position = (0, 0)
        self.target_waypoint = "HOME"

    def set_destination(self, waypoint_name):
        if waypoint_name in self.waypoints:
            self.target_waypoint = waypoint_name
            print(f"Đã thiết lập điểm đến: {waypoint_name} tại {self.waypoints[waypoint_name]}")
        else:
            print(f"Không tìm thấy điểm đến: {waypoint_name}")

    def get_distance_to_target(self):
        import math
        target = self.waypoints[self.target_waypoint]
        dist = math.sqrt((target[0] - self.current_position[0])**2 + 
                         (target[1] - self.current_position[1])**2)
        return dist

    def move_step(self):
        """Simulate a movement step towards the target"""
        if self.get_distance_to_target() < 0.1:
            print(f"Đã đến {self.target_waypoint}")
            return True
        
        # Simple linear interpolation for simulation
        target = self.waypoints[self.target_waypoint]
        dx = target[0] - self.current_position[0]
        dy = target[1] - self.current_position[1]
        
        step_size = 0.5
        self.current_position = (
            self.current_position[0] + (dx * 0.1),
            self.current_position[1] + (dy * 0.1)
        )
        print(f"Vị trí hiện tại: {self.current_position}")
        return False

if __name__ == "__main__":
    nav = NavigationManager()
    nav.set_destination("TANK_1")
    while not nav.move_step():
        pass
