/*
 * =========================================================================================
 *   DỰ ÁN ROBOT TỰ HÀNH SIFAR - ĐIỀU KHIỂN HOVERBOARD 10 INCH QUA ESP32-S3 (MẠCH TÍM)
 *   Bo mạch ESP32: XH-S3E-AI V1.0 (ESP32-S3-N16R8)
 *   Mã nguồn tích hợp: Phát WiFi AP + Web Remote HTML5 Joystick + Điều khiển Servo bẻ góc
 * =========================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>

// --- THÔNG SỐ WIFI ACCESS POINT ---
const char* ssid = "ROBOT_HOVERBOARD";
const char* password = ""; // Khong dat mat khau de ket noi nhanh

WebServer server(80);

// --- CẤU HÌNH NGOẠI VI TRÊN MẠCH TÍM ESP32-S3 ---
#define PIN_SERVO          41    // Chân IO41 (Chân 5 trên hàng 8 chân) nối dây tín hiệu Servo (Cam/Vàng)
#define PIN_HOVER_RX       41    // Chân RX đọc dữ liệu Hoverboard (khi dùng chế độ UART)
#define PIN_HOVER_TX       42    // Chân IO42 (Chân 6) nối vào RX xe (Dây 2 cáp trắng)
#define HOVER_BAUD         9600

#define PWM_FREQ           50    // 50Hz chuẩn động cơ Servo RC
#define PWM_RESOLUTION     12    // Độ phân giải 12-bit (0 - 4095)

HardwareSerial HoverSerial(1);

// Biến lưu trạng thái góc Servo
// 90 độ = Nằm phẳng (ĐỨNG YÊN)
// 110 độ = Nghiêng tới trước 20 độ (TIẾN)
// 70 độ  = Nghiêng về sau 20 độ (LÙI)
int currentAngle = 90;
unsigned long lastCommandTime = 0;

// Hàm điều khiển góc quay của Servo qua PWM nội tại ESP32-S3
void setServoAngle(int angle) {
  // map góc từ 0 - 180 độ sang duty cycle tương ứng (205 ~ 1000us, 307 ~ 1500us, 410 ~ 2000us)
  int duty = map(angle, 0, 180, 205, 410);
  ledcWrite(PIN_SERVO, duty);
}

// Hàm gửi gói tin 4 byte sang bo Hoverboard LKS32MC
void sendHoverPacket(uint8_t b1, uint8_t b2) {
  uint8_t pkt[4] = {0xB5, b1, b2, 0xE7};
  HoverSerial.write(pkt, 4);
}

// --- GIAO DIỆN WEB ĐIỀU KHIỂN TRÊN ĐIỆN THOẠI (HTML5 + UTF-8) ---
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>SIFAR Robot Remote</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #121212; color: white; margin: 0; padding: 15px; touch-action: none; }
    h2 { color: #00e676; margin-bottom: 5px; }
    #msg { font-size: 16px; color: #ffeb3b; min-height: 25px; margin-bottom: 15px; font-weight: bold; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; max-width: 320px; margin: 0 auto; }
    .btn { background: #29b6f6; color: black; border: none; padding: 16px; font-size: 18px; border-radius: 10px; font-weight: bold; }
    .btn-stop { background: #ff1744; color: white; padding: 18px; font-size: 22px; border-radius: 12px; font-weight: bold; border: none; margin-top: 15px; width: 100%; max-width: 320px; }
    #zone_joystick { width: 220px; height: 220px; background: #2a2a2a; border-radius: 50%; margin: 15px auto; position: relative; border: 3px solid #555; }
    #stick { width: 70px; height: 70px; background: #00e676; border-radius: 50%; position: absolute; top: 75px; left: 75px; }
  </style>
</head>
<body>
  <h2>ROBOT SIFAR</h2>
  <div id="msg">Sẵn sàng điều khiển</div>

  <div class="btn-grid">
    <button class="btn" onclick="sendCmd('forward', 'TIẾN THẲNG')">⬆️ TIẾN</button>
    <button class="btn" onclick="sendCmd('backward', 'LÙI LẠI')">⬇️ LÙI</button>
  </div>

  <div id="zone_joystick">
    <div id="stick"></div>
  </div>

  <button class="btn-stop" onclick="sendCmd('stop', 'DỪNG KHẨN CẤP')">⏹ DỪNG LẠI</button>

  <script>
    function sendCmd(action, text) {
      document.getElementById('msg').innerText = text;
      fetch('/cmd?action=' + action).catch(e => {});
    }

    const zone = document.getElementById('zone_joystick');
    const stick = document.getElementById('stick');
    const radius = 75;
    let active = false;

    function handleMove(touchX, touchY) {
      const zRect = zone.getBoundingClientRect();
      let dx = touchX - (zRect.left + zRect.width / 2);
      let dy = touchY - (zRect.top + zRect.height / 2);
      let dist = Math.sqrt(dx * dx + dy * dy);

      if (dist > radius) {
        dx = (dx / dist) * radius;
        dy = (dy / dist) * radius;
      }

      stick.style.left = (75 + dx) + 'px';
      stick.style.top = (75 + dy) + 'px';

      let speed = Math.round((-dy / radius) * 100);
      fetch('/drive?speed=' + speed).catch(e => {});
    }

    zone.addEventListener('touchstart', (e) => { active = true; handleMove(e.touches[0].clientX, e.touches[0].clientY); });
    window.addEventListener('touchmove', (e) => { if(active) handleMove(e.touches[0].clientX, e.touches[0].clientY); });
    window.addEventListener('touchend', () => { 
      if(active) { 
        active = false; 
        stick.style.left = '75px'; 
        stick.style.top = '75px'; 
        sendCmd('stop', 'ĐÃ DỪNG'); 
      } 
    });
  </script>
</body>
</html>
)rawliteral";

void handleRoot() { 
  server.send(200, "text/html", INDEX_HTML); 
}

// Xử lý các nút bấm Tiến / Lùi / Dừng
void handleCmd() {
  if (server.hasArg("action")) {
    String act = server.arg("action");
    if (act == "forward") {
      currentAngle = 110; // Xoay servo nghiêng tới
      setServoAngle(currentAngle);
      sendHoverPacket(0x28, 0xBD); // Đồng thời gửi mã UART nghiêng tới
      lastCommandTime = millis();
      Serial.println(">>> [REMOTE] Lệnh: TIẾN (Servo 110°)");
    } else if (act == "backward") {
      currentAngle = 70;  // Xoay servo nghiêng lui
      setServoAngle(currentAngle);
      sendHoverPacket(0xA8, 0xFF); // Gửi mã UART nghiêng lui
      lastCommandTime = millis();
      Serial.println(">>> [REMOTE] Lệnh: LÙI (Servo 70°)");
    } else if (act == "stop") {
      currentAngle = 90;  // Về vị trí phẳng
      setServoAngle(currentAngle);
      sendHoverPacket(0xA8, 0xBC); // Gửi mã UART đứng yên
      lastCommandTime = 0;
      Serial.println(">>> [REMOTE] Lệnh: DỪNG (Servo 90°)");
    }
    server.send(200, "text/plain", "OK");
  }
}

// Xử lý cần gạt Virtual Joystick tỉ lệ
void handleDrive() {
  if (server.hasArg("speed")) {
    int spd = server.arg("speed").toInt(); // -100 đến 100
    // Đổi tỉ lệ tốc độ sang góc nghiêng Servo từ 70 độ đến 110 độ
    int angle = map(spd, -100, 100, 70, 110);
    currentAngle = angle;
    setServoAngle(currentAngle);
    lastCommandTime = millis();
    server.send(200, "text/plain", "OK");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // 1. Khởi động xung PWM điều khiển Servo
  ledcAttach(PIN_SERVO, PWM_FREQ, PWM_RESOLUTION);
  setServoAngle(90); // Mặc định ở giữa (Phẳng)

  // 2. Khởi động UART giao tiếp với bo xe Hoverboard
  HoverSerial.begin(HOVER_BAUD, SERIAL_8N1, PIN_HOVER_RX, PIN_HOVER_TX);

  // 3. Khởi động WiFi Access Point & Web Server
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/drive", handleDrive);
  server.begin();

  Serial.println("\n========================================================");
  Serial.println("  ROBOT SIFAR - ESP32-S3 CONTROLLER ĐÃ SẴN SÀNG!");
  Serial.println("  WiFi: ROBOT_HOVERBOARD | Truy cập: http://192.168.4.1");
  Serial.println("========================================================");
}

void loop() {
  server.handleClient();

  // Tính năng an toàn Fail-safe: Nếu mất sóng WiFi hoặc buông tay quá 600ms -> Tự trả về phẳng để dừng xe
  if (lastCommandTime > 0 && (millis() - lastCommandTime > 600)) {
    currentAngle = 90;
    setServoAngle(90);
    sendHoverPacket(0xA8, 0xBC);
    lastCommandTime = 0;
  }
}
