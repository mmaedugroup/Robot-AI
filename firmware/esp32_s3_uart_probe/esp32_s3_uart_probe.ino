/* SIFAR UART probe v1 - manual, one-shot transmission only.
 * This is a protocol diagnostic, NOT a motor controller or emergency stop.
 * Disconnect IO42 from the hoverboard while uploading/checking USB commands.
 * IO41 stays physically disconnected. No servo or Wi-Fi is initialized.
 * USB CDC On Boot: Enabled. USB monitor: 115200 baud.
 * ?: help/status; a: arm for 5 seconds; p: send ONE captured packet, then disarm;
 * x: cancel arming only (does not send a stop command to the hoverboard).
 * B5 A8 BC E7 was observed with the hoverboard upside down.
 * Its meaning is UNKNOWN. It must not be called neutral/stop/safe.
 */
#include <Arduino.h>

HardwareSerial HoverUart(1);
constexpr int UNUSED_RX_PIN = 41;
constexpr int TX_PIN = 42;
constexpr uint32_t HOVER_BAUD = 9600;
constexpr uint32_t ARM_WINDOW_MS = 5000;
constexpr uint8_t CAPTURED_PACKET[] = {0xB5, 0xA8, 0xBC, 0xE7};
bool armed = false;
uint32_t armedAt = 0;

void showStatus() {
  Serial.println("SIFAR UART PROBE v1 | IO42 TX | 9600 8N1 | no automatic transmission");
  Serial.println("?: status | a: arm 5s | p: send ONE captured packet | x: cancel arming");
  Serial.println("Packet B5 A8 BC E7: meaning UNKNOWN; not a confirmed stop command.");
  Serial.println("x/timeout only prevent new packets; they do NOT stop a motor.");
  Serial.println(armed ? "STATE: ARMED" : "STATE: DISARMED");
}

void setup() {
  Serial.begin(115200);
  // RX is initialized on a known pin but remains physically disconnected.
  // Avoid TX=-1/RX=-1 fallback pin assignments across Arduino core versions.
  HoverUart.begin(HOVER_BAUD, SERIAL_8N1, UNUSED_RX_PIN, TX_PIN);
}

void loop() {
  if (armed && millis() - armedAt >= ARM_WINDOW_MS) {
    armed = false;
    Serial.println("DISARMED: arming expired; no packet sent.");
  }
  if (Serial.available() == 0) return;
  const char command = static_cast<char>(Serial.read());
  if (command == '\r' || command == '\n' || command == ' ') return;

  if (command == '?') {
    showStatus();
  } else if (command == 'x') {
    armed = false;
    Serial.println("DISARMED: no stop command sent.");
  } else if (command == 'a') {
    armedAt = millis();
    armed = true;
    Serial.println("ARMED for 5s. p sends one packet of UNKNOWN meaning.");
  } else if (command == 'p') {
    if (!armed || millis() - armedAt >= ARM_WINDOW_MS) {
      armed = false;
      Serial.println("BLOCKED: not armed; no packet sent.");
      return;
    }
    armed = false;
    const size_t sent = HoverUart.write(CAPTURED_PACKET, sizeof(CAPTURED_PACKET));
    HoverUart.flush();
    if (sent == sizeof(CAPTURED_PACKET)) {
      Serial.println("TX: B5 A8 BC E7 | ONE packet | STATE: DISARMED");
    } else {
      Serial.println("ERROR: incomplete UART write | STATE: DISARMED");
    }
  } else {
    armed = false;
    Serial.println("Unknown command; DISARMED. Send ? for help.");
  }
}
