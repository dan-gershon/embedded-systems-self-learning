#include <WiFi.h>

// ============================================================
// Experiment 17 – ESP32 Ultrasonic Distance Web Dashboard
//
// This project measures distance using an HC-SR04 ultrasonic
// sensor and displays live distance data on a phone browser.
//
// Main concepts:
// - Ultrasonic distance measurement
// - Trigger and Echo pins
// - Time-of-flight measurement
// - Speed of sound
// - Pulse duration measurement
// - Voltage divider for ESP32 input protection
// - ESP32 WiFi web server
// - HTTP GET /data request
// - JSON response
// - Live web dashboard
//
// Hardware:
// - ESP32 Development Board
// - HC-SR04 Ultrasonic Sensor
// - 1kΩ resistor
// - 2kΩ resistor, or two 1kΩ resistors in series
//
// Connections:
// - HC-SR04 VCC  -> ESP32 VIN / 5V
// - HC-SR04 GND  -> ESP32 GND
// - HC-SR04 TRIG -> ESP32 GPIO5
// - HC-SR04 ECHO -> Voltage divider -> ESP32 GPIO35
//
// Echo voltage divider:
// - Echo -> 1kΩ -> GPIO35
// - GPIO35 -> 2kΩ -> GND
// ============================================================

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// Pin Settings
// ============================================================
const int TRIG_PIN = 5;
const int ECHO_PIN = 35;

// ============================================================
// Measurement Settings
// ============================================================
const float SPEED_OF_SOUND_CM_PER_US = 0.0343;

// Timeout limits maximum measurable distance.
// 30000 us is about 5 meters round trip, but HC-SR04 is usually reliable up to around 400 cm.
const unsigned long ECHO_TIMEOUT_US = 30000;

float distanceCm = 0.0;
float filteredDistanceCm = 0.0;
unsigned long durationUs = 0;
bool measurementValid = false;

// Simple smoothing filter
const float FILTER_ALPHA = 0.25;

WiFiServer server(80);

// ============================================================
// Measure distance using HC-SR04
// ============================================================
void updateDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  durationUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);

  if (durationUs == 0)
  {
    measurementValid = false;
    return;
  }

  distanceCm = durationUs * SPEED_OF_SOUND_CM_PER_US / 2.0;

  if (distanceCm < 2.0 || distanceCm > 400.0)
  {
    measurementValid = false;
    return;
  }

  measurementValid = true;

  filteredDistanceCm =
    (FILTER_ALPHA * distanceCm) + ((1.0 - FILTER_ALPHA) * filteredDistanceCm);
}

// ============================================================
// Send JSON response
// ============================================================
void sendJsonResponse(WiFiClient& client)
{
  updateDistance();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();

  client.print("{\"distance_cm\":");
  client.print(distanceCm, 1);

  client.print(",\"filtered_distance_cm\":");
  client.print(filteredDistanceCm, 1);

  client.print(",\"duration_us\":");
  client.print(durationUs);

  client.print(",\"valid\":");
  client.print(measurementValid ? "true" : "false");

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
  <title>ESP32 Ultrasonic Dashboard</title>
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

    .distanceValue {
      font-size: 56px;
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
      height: 28px;
      background: #1f2937;
      border-radius: 20px;
      margin: 18px auto;
      overflow: hidden;
      border: 1px solid #334155;
    }

    .barInner {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, #ef4444, #facc15, #22c55e);
      border-radius: 20px;
      transition: width 0.15s;
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

    .statusGood {
      color: #22c55e;
    }

    .statusBad {
      color: #ef4444;
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
    <h1>ESP32 Ultrasonic Sensor</h1>
    <div class="subtitle">Live distance measurement</div>

    <div class="label">Distance</div>
    <div class="distanceValue"><span id="distanceText">0.0</span> cm</div>

    <div class="barOuter">
      <div class="barInner" id="distanceBar"></div>
    </div>

    <div class="grid">
      <div class="card">
        <div class="label">Filtered</div>
        <div class="smallValue"><span id="filteredText">0.0</span> cm</div>
      </div>

      <div class="card">
        <div class="label">Echo Time</div>
        <div class="smallValue"><span id="durationText">0</span> us</div>
      </div>

      <div class="card">
        <div class="label">Status</div>
        <div class="smallValue" id="validText">---</div>
      </div>

      <div class="card">
        <div class="label">Range</div>
        <div class="smallValue">2-400 cm</div>
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
          document.getElementById("distanceText").innerHTML =
            data.distance_cm.toFixed(1);

          document.getElementById("filteredText").innerHTML =
            data.filtered_distance_cm.toFixed(1);

          document.getElementById("durationText").innerHTML =
            data.duration_us;

          let barPercent = data.filtered_distance_cm * 100.0 / 200.0;

          if (barPercent < 0) barPercent = 0;
          if (barPercent > 100) barPercent = 100;

          document.getElementById("distanceBar").style.width =
            barPercent + "%";

          if (data.valid) {
            document.getElementById("validText").innerHTML = "Valid";
            document.getElementById("validText").className = "smallValue statusGood";
          } else {
            document.getElementById("validText").innerHTML = "No Echo";
            document.getElementById("validText").className = "smallValue statusBad";
          }

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

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  delay(500);

  updateDistance();
  filteredDistanceCm = distanceCm;

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
// Main Loop
// ============================================================
void loop()
{
  static unsigned long lastMeasureTime = 0;
  static unsigned long lastPrintTime = 0;

  if (millis() - lastMeasureTime >= 100)
  {
    lastMeasureTime = millis();
    updateDistance();
  }

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

  if (millis() - lastPrintTime >= 500)
  {
    lastPrintTime = millis();

    Serial.print("Distance: ");
    Serial.print(distanceCm, 1);

    Serial.print(" cm | Filtered: ");
    Serial.print(filteredDistanceCm, 1);

    Serial.print(" cm | Echo: ");
    Serial.print(durationUs);

    Serial.print(" us | Valid: ");
    Serial.println(measurementValid ? "yes" : "no");
  }
}
