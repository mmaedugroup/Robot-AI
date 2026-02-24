from enum import Enum, auto

class RobotState(Enum):
    IDLE = auto()
    PATROL_TANKS = auto()
    MONITOR_RAS = auto()
    FEEDING = auto()
    GO_TO_CHARGER = auto()
    CHARGING = auto()
    GO_TO_REFILLER = auto()
    REFILLING = auto()
    ERROR = auto()

class SIFARStateMachine:
    def __init__(self):
        self.state = RobotState.IDLE
        self.battery_level = 100
        self.food_level = 100
        self.is_connected = True

    def update(self):
        """Main loop update for state transitions"""
        print(f"Current State: {self.state}")

        # Check for critical conditions
        if self.battery_level < 20 and self.state not in [RobotState.GO_TO_CHARGER, RobotState.CHARGING]:
            self.transition_to(RobotState.GO_TO_CHARGER)
        
        elif self.food_level < 10 and self.state == RobotState.FEEDING:
            self.transition_to(RobotState.GO_TO_REFILLER)

        # State-specific logic
        if self.state == RobotState.IDLE:
            # Placeholder for scheduled tasks
            pass
        elif self.state == RobotState.CHARGING:
            self.battery_level += 5
            if self.battery_level >= 100:
                self.transition_to(RobotState.IDLE)
        elif self.state == RobotState.REFILLING:
            self.food_level = 100
            self.transition_to(RobotState.IDLE)

    def transition_to(self, new_state: RobotState):
        print(f"Transitioning from {self.state} to {new_state}")
        self.state = new_state

if __name__ == "__main__":
    sm = SIFARStateMachine()
    sm.battery_level = 15
    for _ in range(5):
        sm.update()
