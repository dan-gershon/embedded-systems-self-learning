#include <WiFi.h>
#include <ESP32Servo.h>

// ============================================================
// Experiment 16 – ESP32 Potentiometer Controlled Servo
// with Web Feedback
//
// This project reads a potentiometer using the ESP32 ADC,
// maps the analog value to a servo angle, moves an SG90 servo,
// and displays live data on a phone browser using a WiFi
// web dashboard.
//
// Main concepts:
// - Potentiometer as voltage divider
// - ESP32 ADC measurement
// - ADC1 usage with WiFi
// - Mapping analog input to servo angle
// - Servo PWM control
// - External 5V supply for motor load
// - Common ground
// - JSON response
// - Live web dashboard
//
// Hardware:
// - ESP32 Development Board
// - Potentiometer
// - SG90 Servo Motor
// - External 5V power supply
// - 100uF capacitor
//
// Connections:
// Potentiometer:
// - Side pin   -> ESP32 3.3V
// - Middle pin -> ESP32 GPIO34
// - Side pin   -> ESP32 GND
//
// Servo:
// - Servo Brown / Black        -> External 5V GND
// - Servo Red                  -> External 5V +
// - Servo Orange / Yellow      -> ESP32 GPIO18
// - External 5V GND            -> ESP32 GND
// - Capacitor +                -> External 5V +
// - Capacitor -                -> External 5V GND
// ============================================================

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// Pin Settings
// ============================================================
const int POT_PIN = 34;      // ADC1 pin, safe to use with WiFi
const int SERVO_PIN = 18;

// ============================================================
// ADC Settings
// ============================================================
const int ADC_MAX = 4095;
const float VREF = 3.3;

// ============================================================
// Servo Settings
// ============================================================
const int MIN_ANGLE = 10;
const int MAX_ANGLE = 170;

const int MIN_PULSE_US = 600;
const int MAX_PULSE_US = 2400;

// ============================================================
// Filtering Settings
// ============================================================
// Simple smoothing filter.
// Higher alpha = faster response, lower alpha = smoother signal.
const float FILTER_ALPHA = 0.20;

// Minimum angle change needed before commanding the servo.
// This prevents tiny ADC noise from constantly moving the servo.
const int ANGLE_DEADBAND = 1;

// ============================================================
// Global Variables
// ============================================================
int rawValue = 0;
float filteredRaw = 0.0;
float voltageValue = 0.0;
float percentValue = 0.0;

int targetAngle = 90;
int currentServoAngle = 90;
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
// Convert angle to servo pulse width
// ============================================================
int angleToPulseUs(int angle)
{
  angle = clampValue(angle, MIN_ANGLE, MAX_ANGLE);
  return map(angle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
}

// ============================================================
// Read ADC, filter value, calculate voltage, percent and angle
// ============================================================
void updateMeasurement()
{
  rawValue = analogRead(POT_PIN);

  filteredRaw = (FILTER_ALPHA * rawValue) + ((1.0 - FILTER_ALPHA) * filteredRaw);

  voltageValue = filteredRaw * VREF / ADC_MAX;
  percentValue = filteredRaw * 100.0 / ADC_MAX;

  targetAngle = map((int)filteredRaw, 0, ADC_MAX, MIN_ANGLE, MAX_ANGLE);
  targetAngle = clampValue(targetAngle, MIN_ANGLE, MAX_ANGLE);
}

// ============================================================
// Move servo if target angle changed enough
// ============================================================
void updateServo()
{
  int angleDifference = abs(targetAngle - currentServoAngle);

  if (angleDifference >= ANGLE_DEADBAND)
  {
    currentServoAngle = targetAngle;
    currentPulseUs = angleToPulseUs(currentServoAngle);
    myServo.writeMicroseconds(currentPulseUs);
  }
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

  client.print("{\"raw\":");
  client.print(rawValue);

  client.print(",\"filtered_raw\":");
  client.print(filteredRaw, 1);

  client.print(",\"voltage\":");
  client.print(voltageValue, 3);

  client.print(",\"percent\":");
  client.print(percentValue, 1);

  client.print(",\"angle\":");
  client.print(currentServoAngle);

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
  <title>ESP32 Pot Servo Dashboard</title>
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
      max-width: 460px;
      margin: auto;
      padding: 24px;
      border-radius: 22px;
      box-shadow: 0 0 30px rgba(0,0,0,0.35);
    }

    h1 {
      margin-bottom: 4px;
      font-size: 27px;
    }

    .subtitle {
      color: #cbd5e1;
      margin-bottom: 22px;
    }

    .angleValue {
      font-size: 54px;
      font-weight: bold;
      color: #38bdf8;
      margin: 12px;
    }

    .label {
      color: #cbd5e1;
      font-size: 15px;
    }

    .barOuter {
      width: 90%;
      height: 26px;
      background: #1f2937;
      border-radius: 20px;
      margin: 18px auto;
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

    .grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
      margin-top: 18px;
    }

    .card {
      background: #1f2937;
      border-radius: 14px;
      padding: 12px 6px;
    }

    .smallValue {
      font-size: 21px;
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
    <h1>ESP32 Potentiometer Servo</h1>
    <div class="subtitle">Physical input controls mechanical output</div>

    <div class="label">Servo Angle</div>
    <div class="angleValue"><span id="angleText">90</span>&deg;</div>

    <div class="barOuter">
      <div class="barInner" id="angleBar"></div>
    </div>

    <div class="grid">
      <div class="card">
        <div class="label">Raw ADC</div>
        <div class="smallValue" id="rawText">0</div>
      </div>

      <div class="card">
        <div class="label">Voltage</div>
        <div class="smallValue"><span id="voltageText">0.000</span> V</div>
      </div>

      <div class="card">
        <div class="label">Potentiometer</div>
        <div class="smallValue"><span id="percentText">0.0</span>%</div>
      </div>

      <div class="card">
        <div class="label">Pulse Width</div>
        <div class="smallValue"><span id="pulseText">1500</span> us</div>
      </div>
    </div>

    <h3>ESP32 JSON Response</h3>
    <pre id="jsonBox">{}</pre>
  </div>

  <script>
    function updateDashboard() {
      fetch("/data")
        .then(response => response.json())
        .then(data => {
          document.getElementById("rawText").innerHTML = data.raw;
          document.getElementById("voltageText").innerHTML = data.voltage.toFixed(3);
          document.getElementById("percentText").innerHTML = data.percent.toFixed(1);
          document.getElementById("angleText").innerHTML = data.angle;
          document.getElementById("pulseText").innerHTML = data.pulse_us;

          let anglePercent = (data.angle - data.min_angle) * 100.0 /
                             (data.max_angle - data.min_angle);

          document.getElementById("angleBar").style.width = anglePercent + "%";

          document.getElementById("jsonBox").innerHTML =
            JSON.stringify(data, null, 2);
        })
        .catch(error => {
          document.getElementById("jsonBox").innerHTML = "Connection error";
        });
    }

    setInterval(updateDashboard, 300);
    updateDashboard();
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

  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  rawValue = analogRead(POT_PIN);
  filteredRaw = rawValue;

  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, MIN_PULSE_US, MAX_PULSE_US);

  updateMeasurement();
  currentServoAngle = targetAngle;
  currentPulseUs = angleToPulseUs(currentServoAngle);
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
  updateMeasurement();
  updateServo();

  WiFiClient client = server.available();

  if (client)
  {
    client.setTimeout(50);

    String request = client.readStringUntil('\r');
    client.flush();

    if (request.length() > 0)
    {
      Serial.println(request);

      if (request.indexOf("GET /data") >= 0)
      {
        sendJsonResponse(client);
      }
      else
      {
        sendHtmlPage(client);
      }
    }

    client.stop();
  }

  static unsigned long lastPrintTime = 0;

  if (millis() - lastPrintTime >= 500)
  {
    lastPrintTime = millis();

    Serial.print("Raw: ");
    Serial.print(rawValue);

    Serial.print(" | Voltage: ");
    Serial.print(voltageValue, 3);

    Serial.print(" V | Percent: ");
    Serial.print(percentValue, 1);

    Serial.print(" % | Angle: ");
    Serial.print(currentServoAngle);

    Serial.print(" deg | Pulse: ");
    Serial.print(currentPulseUs);

    Serial.println(" us");
  }

  delay(10);
}
