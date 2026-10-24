/* SIFAR RAW CAPTURE v1 - receive only, no servo, Wi-Fi, or UART writes.
 * Before uploading: disconnect IO42 from the hoverboard; IO41 disconnected too.
 * USB CDC On Boot: Enabled. USB Serial Monitor: 115200 baud.
 * ? = status; c = capture RX for 3 seconds, then print all bytes in HEX.
 * The LEDC servo controller must not run on IO41 during capture.
 * RX wiring/voltage must be checked separately before connecting the vehicle.
 * Software timestamps describe UART reads, not logic-analyzer edge timings.
 */
#include <Arduino.h>

HardwareSerial HoverUart(1);
constexpr int RX_PIN = 41;
constexpr int DISCONNECTED_TX_PIN = 42;
constexpr uint32_t HOVER_BAUD = 9600;
constexpr uint32_t CAPTURE_MS = 3000;
constexpr size_t CAPACITY = 4096;
uint8_t capturedBytes[CAPACITY];
uint32_t readTimesUs[CAPACITY];

void showStatus() {
  Serial.println("SIFAR RAW CAPTURE v1 | RX IO41 | 9600 8N1 | NO UART TRANSMISSION");
  Serial.println("?: status | c: record 3 seconds, then print ALL received bytes");
  Serial.println("TX IO42 must remain physically disconnected from hoverboard.");
}

void capture() {
  while (HoverUart.available() > 0) HoverUart.read();
  Serial.println("CAPTURE: 3 seconds. Keep the board position unchanged.");
  Serial.flush();
  const uint32_t startMs = millis();
  const uint32_t startUs = micros();
  size_t count = 0;
  size_t dropped = 0;
  // No USB printing during collection, to reduce timing disturbances.
  while (millis() - startMs < CAPTURE_MS) {
    while (HoverUart.available() > 0) {
      const int value = HoverUart.read();
      if (value < 0) break;
      const uint32_t relativeUs = micros() - startUs;
      if (count < CAPACITY) {
        capturedBytes[count] = static_cast<uint8_t>(value);
        readTimesUs[count] = relativeUs;
        ++count;
      } else {
        ++dropped;
      }
      if (millis() - startMs >= CAPTURE_MS) break;
    }
    delay(1);
  }

  Serial.printf("CAPTURE END: bytes=%u, storage_dropped=%u\n",
                static_cast<unsigned int>(count), static_cast<unsigned int>(dropped));
  Serial.println("RAW: [first-read us .. last-read us] HEX; rows are NOT protocol frames.");
  for (size_t first = 0; first < count; first += 16) {
    const size_t end = (first + 16 < count) ? first + 16 : count;
    Serial.printf("[%lu..%lu us] ", static_cast<unsigned long>(readTimesUs[first]),
                  static_cast<unsigned long>(readTimesUs[end - 1]));
    for (size_t i = first; i < end; ++i) {
      Serial.printf("%02X ", static_cast<unsigned int>(capturedBytes[i]));
    }
    Serial.println();
  }

  size_t candidates = 0;
  uint32_t previousUs = 0;
  uint32_t minimumUs = UINT32_MAX;
  uint32_t maximumUs = 0;
  uint64_t sumUs = 0;
  // Scan overlapping windows: do not lose a possible start after a bad window.
  for (size_t i = 0; i + 3 < count; ++i) {
    if (capturedBytes[i] != 0xB5 || capturedBytes[i + 3] != 0xE7) continue;
    const uint32_t timestamp = readTimesUs[i];
    if (candidates > 0) {
      const uint32_t interval = timestamp - previousUs;
      if (interval < minimumUs) minimumUs = interval;
      if (interval > maximumUs) maximumUs = interval;
      sumUs += interval;
    }
    previousUs = timestamp;
    ++candidates;
  }
  Serial.printf("B5 ?? ?? E7 candidates=%u (pattern matches, NOT validated packets)\n",
                static_cast<unsigned int>(candidates));
  if (candidates > 1) {
    Serial.printf("Approx candidate interval us: min=%lu mean=%lu max=%lu\n",
                  static_cast<unsigned long>(minimumUs),
                  static_cast<unsigned long>(sumUs / (candidates - 1)),
                  static_cast<unsigned long>(maximumUs));
  }
  Serial.println("DONE. Send ? for status or c for another capture.");
}

void setup() {
  Serial.begin(115200);
  HoverUart.setRxBufferSize(4096);
  HoverUart.begin(HOVER_BAUD, SERIAL_8N1, RX_PIN, DISCONNECTED_TX_PIN);
}

void loop() {
  if (Serial.available() == 0) return;
  const char command = static_cast<char>(Serial.read());
  if (command == '?') showStatus();
  else if (command == 'c' || command == 'C') capture();
}
