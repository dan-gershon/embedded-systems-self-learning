#include <WiFi.h>

// ============================================================
// Experiment 13 – ESP32 ADC Web Dashboard
//
// This project reads an analog voltage from a potentiometer
// using the ESP32 ADC and displays the measured value on a
// phone browser through a WiFi web dashboard.
//
// Main concepts:
// - ADC reading
// - Analog voltage measurement
// - Potentiometer as voltage divider
// - ESP32 ADC1 with WiFi
// - HTTP GET request
// - JSON response
// - Live web dashboard
//
// Hardware:
// - Potentiometer 10k
// - Left pin   -> 3.3V
// - Middle pin -> GPIO34
// - Right pin  -> GND
// ============================================================

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// ADC Settings
// ============================================================
const int POT_PIN = 34;

// ESP32 ADC is 12-bit by default, but we define it clearly.
const int ADC_MAX = 4095;
const float VREF = 3.3;

// Current measurement values
int rawValue = 0;
float voltageValue = 0.0;
float percentValue = 0.0;

WiFiServer server(80);

// ============================================================
// Read ADC and calculate voltage + percent
// ============================================================
void updateAdcMeasurement()
{
  rawValue = analogRead(POT_PIN);

  voltageValue = rawValue * VREF / ADC_MAX;
  percentValue = rawValue * 100.0 / ADC_MAX;
}

// ============================================================
// Send JSON response
// ============================================================
void sendJsonResponse(WiFiClient& client)
{
  updateAdcMeasurement();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  client.print("{\"raw\":");
  client.print(rawValue);

  client.print(",\"voltage\":");
  client.print(voltageValue, 3);

  client.print(",\"percent\":");
  client.print(percentValue, 1);

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
  <title>ESP32 ADC Web Dashboard</title>
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
      max-width: 430px;
      margin: auto;
      padding: 22px;
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

    .value {
      font-size: 38px;
      font-weight: bold;
      margin: 8px;
      color: #38bdf8;
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
      margin: 22px auto;
      overflow: hidden;
      border: 1px solid #334155;
    }

    .barInner {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, #22c55e, #38bdf8, #a855f7);
      border-radius: 20px;
      transition: width 0.15s;
    }

    .grid {
      display: grid;
      grid-template-columns: 1fr 1fr 1fr;
      gap: 10px;
      margin-top: 18px;
    }

    .card {
      background: #1f2937;
      border-radius: 14px;
      padding: 12px 6px;
    }

    .smallValue {
      font-size: 20px;
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
    <h1>ESP32 ADC Dashboard</h1>
    <div class="subtitle">Potentiometer live measurement</div>

    <div class="label">Position</div>
    <div class="value"><span id="percentText">0.0</span>%</div>

    <div class="barOuter">
      <div class="barInner" id="bar"></div>
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
        <div class="label">Percent</div>
        <div class="smallValue"><span id="percentText2">0.0</span>%</div>
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
          document.getElementById("percentText2").innerHTML = data.percent.toFixed(1);

          document.getElementById("bar").style.width = data.percent + "%";

          document.getElementById("jsonBox").innerHTML =
            JSON.stringify(data, null, 2);
        })
        .catch(error => {
          document.getElementById("jsonBox").innerHTML =
            "Connection error";
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

  WiFi.mode(WIFI_STA);
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

  String request = client.readStringUntil('\r');
  client.flush();

  Serial.println(request);

  if (request.indexOf("GET /data") >= 0)
  {
    sendJsonResponse(client);

    Serial.print("Raw: ");
    Serial.print(rawValue);

    Serial.print(" | Voltage: ");
    Serial.print(voltageValue, 3);

    Serial.print(" V | Percent: ");
    Serial.print(percentValue, 1);

    Serial.println(" %");

    return;
  }

  sendHtmlPage(client);
}
