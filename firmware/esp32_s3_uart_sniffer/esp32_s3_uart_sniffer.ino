/* SIFAR: receive-only UART capture. No servo, Wi-Fi, or motor commands.
 * ESP32 GPIO41 <- hoverboard TX; GND <-> GND.
 * Leave ESP32 TX disconnected. Never connect the cable's 15V to ESP32.
 * Arduino IDE: ESP32-S3, USB CDC On Boot enabled; monitor 115200 baud.
 */
#include <Arduino.h>

HardwareSerial HoverSerial(1);
constexpr int HOVER_RX_PIN = 41;
constexpr uint32_t HOVER_BAUD = 9600;
constexpr size_t CAPTURE_SIZE = 32;
uint8_t capture[CAPTURE_SIZE];
size_t captureLength = 0;
uint32_t captureStarted = 0;
uint32_t lastByteTime = 0;

void printCapture() {
  if (captureLength == 0) return;
  if (Serial) {
    Serial.printf("[%lu ms] ", static_cast<unsigned long>(captureStarted));
    for (size_t i = 0; i < captureLength; ++i) {
      Serial.printf("%02X ", static_cast<unsigned int>(capture[i]));
    }
    Serial.println();
  }
  captureLength = 0;
}

void setup() {
  Serial.begin(115200);
  // Explicitly choose a TX pin, but do not physically connect it.
  // This sketch never calls HoverSerial.write/print.
  HoverSerial.setRxBufferSize(2048);
  HoverSerial.begin(HOVER_BAUD, SERIAL_8N1, HOVER_RX_PIN, 42);
}

void loop() {
  while (HoverSerial.available() > 0) {
    const int value = HoverSerial.read();
    if (value < 0) break;
    const uint32_t now = millis();
    if (captureLength > 0 && now - lastByteTime >= 10) printCapture();
    if (captureLength == 0) captureStarted = now;
    capture[captureLength++] = static_cast<uint8_t>(value);
    lastByteTime = now;
    if (captureLength == CAPTURE_SIZE) printCapture();
  }
  if (captureLength > 0 && millis() - lastByteTime >= 10) printCapture();
}
