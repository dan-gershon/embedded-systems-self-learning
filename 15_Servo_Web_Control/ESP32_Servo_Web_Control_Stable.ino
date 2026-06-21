#include <WiFi.h>
#include <ESP32Servo.h>

// ============================================================
// Experiment 15 – ESP32 Servo Web Control Stable Version
//
// This project controls an SG90 servo motor using an ESP32
// web server. A phone browser sends angle commands to the ESP32
// using HTTP GET requests, and the ESP32 returns JSON responses.
//
// Stable version improvements:
// - Servo powered from external 5V supply is recommended
// - WiFi sleep disabled
// - Safe servo angle range
// - Safe servo pulse range
// - Slider requests are throttled in JavaScript
// - ESP32 ignores repeated identical servo commands
// - Short client timeout
// - HTTP connection is closed after each response
//
// Hardware:
// - ESP32 Development Board
// - SG90 Servo Motor
// - External 5V supply for servo
// - 100uF capacitor across servo 5V and GND, optional but recommended
//
// Connections:
// - Servo Brown / Black        -> External 5V GND
// - Servo Red                  -> External 5V +
// - Servo Orange / Yellow      -> ESP32 GPIO18
// - External 5V GND            -> ESP32 GND
// - Capacitor +                -> External 5V +
// - Capacitor -                -> External 5V GND
//
// Important:
// The ESP32 and servo power supply must share the same GND.
// Do NOT connect external 5V to ESP32 3V3.
// ============================================================

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// Servo Settings
// ============================================================
const int SERVO_PIN = 18;

// Safer range for SG90.
// Many cheap SG90 servos buzz or hit mechanical limits at 0 and 180.
const int MIN_ANGLE = 10;
const int MAX_ANGLE = 170;

// Safer pulse range.
// Typical full range can be 500 to 2500 us.
// This safer range reduces edge stress and current spikes.
const int MIN_PULSE_US = 600;
const int MAX_PULSE_US = 2400;

int currentAngle = 90;
int currentPulseUs = 1500;

Servo myServo;
WiFiServer server(80);

// ============================================================
// Helper: Clamp value to range
// ============================================================
int clampValue(int value, int minValue, int maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

// ============================================================
// Helper: Extract integer parameter from HTTP request
//
// Example:
// GET /set?angle=90 HTTP/1.1
// ============================================================
int getParamValue(String request, String key, int defaultValue)
{
  String pattern = key + "=";
  int startIndex = request.indexOf(pattern);

  if (startIndex < 0)
  {
    return defaultValue;
  }

  startIndex += pattern.length();

  int endIndex = request.indexOf("&", startIndex);
  int spaceIndex = request.indexOf(" ", startIndex);

  if (endIndex < 0 || (spaceIndex > 0 && spaceIndex < endIndex))
  {
    endIndex = spaceIndex;
  }

  if (endIndex < 0)
  {
    return defaultValue;
  }

  String valueString = request.substring(startIndex, endIndex);
  return valueString.toInt();
}

// ============================================================
// Convert angle to pulse width in microseconds
// ============================================================
int angleToPulseUs(int angle)
{
  angle = clampValue(angle, MIN_ANGLE, MAX_ANGLE);
  return map(angle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
}

// ============================================================
// Move servo only if the angle changed
// ============================================================
void setServoAngle(int angle)
{
  angle = clampValue(angle, MIN_ANGLE, MAX_ANGLE);

  if (angle == currentAngle)
  {
    return;
  }

  currentAngle = angle;
  currentPulseUs = angleToPulseUs(currentAngle);

  myServo.writeMicroseconds(currentPulseUs);
}

// ============================================================
// Send JSON response
// ============================================================
void sendJsonResponse(WiFiClient& client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();

  client.print("{\"angle\":");
  client.print(currentAngle);

  client.print(",\"pulse_us\":");
  client.print(currentPulseUs);

  client.print(",\"min_angle\":");
  client.print(MIN_ANGLE);

  client.print(",\"max_angle\":");
  client.print(MAX_ANGLE);

  client.print(",\"status\":\"ok\"}");
}

// ============================================================
// Send HTML page
// ============================================================
void sendHtmlPage(WiFiClient& client)
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Servo Web Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: #0f172a;
      color: white;
      margin: 0;
      padding: 24px;
    }

    .box {
      background: #111827;
      max-width: 440px;
      margin: auto;
      padding: 24px;
      border-radius: 22px;
      box-shadow: 0 0 30px rgba(0,0,0,0.35);
    }

    h1 {
      margin-bottom: 4px;
      font-size: 28px;
    }

    .subtitle {
      color: #cbd5e1;
      margin-bottom: 22px;
    }

    .angleValue {
      font-size: 50px;
      font-weight: bold;
      color: #38bdf8;
      margin: 14px;
    }

    input[type=range] {
      width: 90%;
      margin: 18px 0;
    }

    button {
      font-size: 16px;
      padding: 10px 14px;
      margin: 6px;
      border: none;
      border-radius: 10px;
      background: #38bdf8;
      color: #020617;
      font-weight: bold;
    }

    .barOuter {
      width: 90%;
      height: 24px;
      background: #1f2937;
      border-radius: 20px;
      margin: 16px auto;
      overflow: hidden;
      border: 1px solid #334155;
    }

    .barInner {
      height: 100%;
      width: 50%;
      background: linear-gradient(90deg, #22c55e, #38bdf8, #a855f7);
      border-radius: 20px;
      transition: width 0.12s;
    }

    .card {
      background: #1f2937;
      border-radius: 14px;
      padding: 12px;
      margin-top: 14px;
    }

    .smallLabel {
      color: #cbd5e1;
      font-size: 14px;
    }

    .smallValue {
      font-size: 22px;
      font-weight: bold;
      margin-top: 6px;
    }

    pre {
      text-align: left;
      background: #020617;
      color: #d1fae5;
      padding: 12px;
      border-radius: 12px;
      overflow-x: auto;
      font-size: 14px;
    }
  </style>
</head>

<body>
  <div class="box">
    <h1>ESP32 Servo Control</h1>
    <div class="subtitle">Stable web control with throttled requests</div>

    <div class="angleValue"><span id="angleText">90</span>&deg;</div>

    <input type="range" min="10" max="170" value="90" id="angleSlider"
           oninput="requestAngle(this.value)">

    <div class="barOuter">
      <div class="barInner" id="bar"></div>
    </div>

    <button onclick="requestAngle(10)">10&deg;</button>
    <button onclick="requestAngle(45)">45&deg;</button>
    <button onclick="requestAngle(90)">90&deg;</button>
    <button onclick="requestAngle(135)">135&deg;</button>
    <button onclick="requestAngle(170)">170&deg;</button>

    <div class="card">
      <div class="smallLabel">Pulse Width</div>
      <div class="smallValue"><span id="pulseText">1500</span> us</div>
    </div>

    <h3>ESP32 JSON Response</h3>
    <pre id="jsonBox">{}</pre>
  </div>

  <script>
    let pendingAngle = 90;
    let lastSentAngle = -1;
    let requestTimer = null;
    let requestBusy = false;

    function requestAngle(angle) {
      pendingAngle = parseInt(angle);

      updateLocalDisplay(pendingAngle, null);

      if (requestTimer !== null) {
        clearTimeout(requestTimer);
      }

      requestTimer = setTimeout(sendLatestAngle, 120);
    }

    function sendLatestAngle() {
      if (requestBusy) {
        requestTimer = setTimeout(sendLatestAngle, 120);
        return;
      }

      if (pendingAngle === lastSentAngle) {
        return;
      }

      requestBusy = true;
      lastSentAngle = pendingAngle;

      fetch("/set?angle=" + pendingAngle)
        .then(response => response.json())
        .then(data => {
          updateLocalDisplay(data.angle, data.pulse_us);

          document.getElementById("jsonBox").innerHTML =
            JSON.stringify(data, null, 2);
        })
        .catch(error => {
          document.getElementById("jsonBox").innerHTML = "Connection error";
        })
        .finally(() => {
          requestBusy = false;

          if (pendingAngle !== lastSentAngle) {
            requestTimer = setTimeout(sendLatestAngle, 120);
          }
        });
    }

    function updateLocalDisplay(angle, pulse) {
      document.getElementById("angleText").innerHTML = angle;
      document.getElementById("angleSlider").value = angle;

      let percent = (angle - 10) * 100.0 / (170 - 10);
      document.getElementById("bar").style.width = percent + "%";

      if (pulse !== null) {
        document.getElementById("pulseText").innerHTML = pulse;
      }
    }

    requestAngle(90);
  </script>
</body>
</html>
)rawliteral";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.println(html);
}

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, MIN_PULSE_US, MAX_PULSE_US);

  currentPulseUs = angleToPulseUs(currentAngle);
  myServo.writeMicroseconds(currentPulseUs);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 40)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connected");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
    Serial.println("Web server started");
    Serial.println("Open the IP address in your phone browser");
  }
  else
  {
    Serial.println("WiFi connection failed");
    Serial.println("Check SSID, password, and 2.4GHz WiFi network");
  }
}

// ============================================================
// Main loop
// ============================================================
void loop()
{
  WiFiClient client = server.available();

  if (!client)
  {
    return;
  }

  client.setTimeout(50);

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.length() == 0)
  {
    client.stop();
    return;
  }

  Serial.println(request);

  if (request.indexOf("GET /set?") >= 0)
  {
    int requestedAngle = getParamValue(request, "angle", currentAngle);
    setServoAngle(requestedAngle);

    Serial.print("Servo angle: ");
    Serial.print(currentAngle);

    Serial.print(" deg | Pulse width: ");
    Serial.print(currentPulseUs);

    Serial.println(" us");

    sendJsonResponse(client);
    client.stop();
    return;
  }

  sendHtmlPage(client);
  client.stop();
}
