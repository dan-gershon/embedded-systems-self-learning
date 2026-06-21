#include <WiFi.h>

// ============================================================
// Experiment 19 – ESP32 Stepper Motor Web Controller
//
// This project controls a 28BYJ-48 stepper motor using an
// ESP32, a ULN2003 driver module, and a WiFi web dashboard.
//
// A phone browser can control:
// - Start / Stop
// - Direction
// - Speed
//
// Main concepts:
// - Stepper motor control
// - ULN2003 motor driver
// - Half-step sequence
// - Direction control
// - Speed control
// - Non-blocking timing using micros()
// - ESP32 WiFi web server
// - HTTP GET requests
// - Query parameters
// - JSON response
//
// Hardware:
// - ESP32 Development Board
// - 28BYJ-48 5V Stepper Motor
// - ULN2003 Stepper Driver Module
// - External 5V power supply
//
// Connections:
// - ULN2003 IN1 -> ESP32 GPIO25
// - ULN2003 IN2 -> ESP32 GPIO26
// - ULN2003 IN3 -> ESP32 GPIO27
// - ULN2003 IN4 -> ESP32 GPIO14
// - ULN2003 VCC -> External 5V +
// - ULN2003 GND -> External 5V GND
// - ESP32 GND   -> External 5V GND
//
// Important:
// The stepper motor must NOT be powered directly from ESP32 GPIO.
// ESP32 gives control signals only.
// ULN2003 + external 5V supply power the motor.
// ESP32 GND and external 5V GND must be connected together.
// ============================================================

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// Stepper Pins
// ============================================================
const int IN1_PIN = 25;
const int IN2_PIN = 26;
const int IN3_PIN = 27;
const int IN4_PIN = 14;

// ============================================================
// Stepper Settings
// ============================================================
// 28BYJ-48 is commonly treated as 4096 half-steps per output shaft revolution.
const int STEPS_PER_REVOLUTION = 4096;

// Half-step sequence for 28BYJ-48 + ULN2003
const int STEP_COUNT = 8;

const int stepSequence[STEP_COUNT][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

// ============================================================
// Motor State
// ============================================================
bool motorRunning = false;

// direction = 1 means forward, direction = -1 means reverse
int direction = 1;

int stepIndex = 0;
long totalSteps = 0;

// Speed in steps per second
int stepsPerSecond = 300;

// Calculated delay between steps
unsigned long stepDelayUs = 1000000UL / 300;

unsigned long lastStepTimeUs = 0;

WiFiServer server(80);

// ============================================================
// Helper: Clamp value
// ============================================================
int clampValue(int value, int minValue, int maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

// ============================================================
// Helper: Extract integer parameter from HTTP request
// Example: GET /set?speed=400 HTTP/1.1
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
// Apply one step state to ULN2003 inputs
// ============================================================
void applyStep(int index)
{
  digitalWrite(IN1_PIN, stepSequence[index][0]);
  digitalWrite(IN2_PIN, stepSequence[index][1]);
  digitalWrite(IN3_PIN, stepSequence[index][2]);
  digitalWrite(IN4_PIN, stepSequence[index][3]);
}

// ============================================================
// Disable all motor coils
// This reduces heating when motor is stopped.
// ============================================================
void releaseMotor()
{
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}

// ============================================================
// Update stepper motor using non-blocking timing
// ============================================================
void updateStepper()
{
  if (!motorRunning)
  {
    return;
  }

  unsigned long nowUs = micros();

  if (nowUs - lastStepTimeUs >= stepDelayUs)
  {
    lastStepTimeUs = nowUs;

    stepIndex += direction;

    if (stepIndex >= STEP_COUNT)
    {
      stepIndex = 0;
    }

    if (stepIndex < 0)
    {
      stepIndex = STEP_COUNT - 1;
    }

    applyStep(stepIndex);

    totalSteps += direction;
  }
}

// ============================================================
// Set motor speed
// ============================================================
void setMotorSpeed(int newStepsPerSecond)
{
  // Safe practical range for 28BYJ-48
  stepsPerSecond = clampValue(newStepsPerSecond, 50, 900);

  stepDelayUs = 1000000UL / stepsPerSecond;
}

// ============================================================
// Estimate RPM
// ============================================================
float calculateRpm()
{
  return (stepsPerSecond * 60.0) / STEPS_PER_REVOLUTION;
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

  client.print("{\"running\":");
  client.print(motorRunning ? "true" : "false");

  client.print(",\"direction\":\"");
  client.print(direction == 1 ? "CW" : "CCW");

  client.print("\",\"steps_per_second\":");
  client.print(stepsPerSecond);

  client.print(",\"step_delay_us\":");
  client.print(stepDelayUs);

  client.print(",\"rpm_estimate\":");
  client.print(calculateRpm(), 2);

  client.print(",\"step_index\":");
  client.print(stepIndex);

  client.print(",\"total_steps\":");
  client.print(totalSteps);

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
  <title>ESP32 Stepper Controller</title>
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
      max-width: 470px;
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

    .statusBig {
      font-size: 42px;
      font-weight: bold;
      margin: 16px 0;
    }

    .running {
      color: #22c55e;
    }

    .stopped {
      color: #ef4444;
    }

    button {
      font-size: 16px;
      padding: 11px 15px;
      margin: 6px;
      border: none;
      border-radius: 10px;
      background: #38bdf8;
      color: #020617;
      font-weight: bold;
    }

    .stopButton {
      background: #ef4444;
      color: white;
    }

    .startButton {
      background: #22c55e;
      color: #020617;
    }

    input[type=range] {
      width: 90%;
      margin: 18px 0;
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
      padding: 12px 8px;
    }

    .label {
      color: #cbd5e1;
      font-size: 14px;
    }

    .smallValue {
      font-size: 20px;
      font-weight: bold;
      margin-top: 6px;
      word-break: break-word;
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
    <h1>ESP32 Stepper Motor</h1>
    <div class="subtitle">28BYJ-48 + ULN2003 Web Controller</div>

    <div id="runState" class="statusBig stopped">Stopped</div>

    <button class="startButton" onclick="sendCommand('start')">START</button>
    <button class="stopButton" onclick="sendCommand('stop')">STOP</button>

    <br>

    <button onclick="sendDirection('cw')">CW</button>
    <button onclick="sendDirection('ccw')">CCW</button>

    <p>Speed: <span id="speedText">300</span> steps/sec</p>
    <input type="range" min="50" max="900" value="300" id="speedSlider"
           oninput="requestSpeed(this.value)">

    <div class="grid">
      <div class="card">
        <div class="label">Direction</div>
        <div class="smallValue" id="directionText">CW</div>
      </div>

      <div class="card">
        <div class="label">RPM Estimate</div>
        <div class="smallValue" id="rpmText">0.00</div>
      </div>

      <div class="card">
        <div class="label">Step Delay</div>
        <div class="smallValue"><span id="delayText">0</span> us</div>
      </div>

      <div class="card">
        <div class="label">Total Steps</div>
        <div class="smallValue" id="stepsText">0</div>
      </div>
    </div>

    <h3>ESP32 JSON Response</h3>
    <pre id="jsonBox">{}</pre>
  </div>

  <script>
    let pendingSpeed = 300;
    let lastSentSpeed = -1;
    let speedTimer = null;
    let requestBusy = false;

    function sendCommand(command) {
      fetch("/set?cmd=" + command)
        .then(response => response.json())
        .then(data => updateDashboardFromData(data))
        .catch(error => {
          document.getElementById("jsonBox").innerHTML = "Connection error";
        });
    }

    function sendDirection(dir) {
      fetch("/set?dir=" + dir)
        .then(response => response.json())
        .then(data => updateDashboardFromData(data))
        .catch(error => {
          document.getElementById("jsonBox").innerHTML = "Connection error";
        });
    }

    function requestSpeed(speed) {
      pendingSpeed = parseInt(speed);
      document.getElementById("speedText").innerHTML = pendingSpeed;

      if (speedTimer !== null) {
        clearTimeout(speedTimer);
      }

      speedTimer = setTimeout(sendLatestSpeed, 120);
    }

    function sendLatestSpeed() {
      if (requestBusy) {
        speedTimer = setTimeout(sendLatestSpeed, 120);
        return;
      }

      if (pendingSpeed === lastSentSpeed) {
        return;
      }

      requestBusy = true;
      lastSentSpeed = pendingSpeed;

      fetch("/set?speed=" + pendingSpeed)
        .then(response => response.json())
        .then(data => updateDashboardFromData(data))
        .catch(error => {
          document.getElementById("jsonBox").innerHTML = "Connection error";
        })
        .finally(() => {
          requestBusy = false;
        });
    }

    function updateStatusOnly() {
      fetch("/data")
        .then(response => response.json())
        .then(data => updateDashboardFromData(data))
        .catch(error => {
          document.getElementById("jsonBox").innerHTML = "Connection error";
        });
    }

    function updateDashboardFromData(data) {
      const runState = document.getElementById("runState");

      if (data.running) {
        runState.innerHTML = "Running";
        runState.className = "statusBig running";
      } else {
        runState.innerHTML = "Stopped";
        runState.className = "statusBig stopped";
      }

      document.getElementById("directionText").innerHTML = data.direction;
      document.getElementById("speedText").innerHTML = data.steps_per_second;
      document.getElementById("speedSlider").value = data.steps_per_second;
      document.getElementById("rpmText").innerHTML = data.rpm_estimate.toFixed(2);
      document.getElementById("delayText").innerHTML = data.step_delay_us;
      document.getElementById("stepsText").innerHTML = data.total_steps;

      document.getElementById("jsonBox").innerHTML =
        JSON.stringify(data, null, 2);
    }

    setInterval(updateStatusOnly, 500);
    updateStatusOnly();
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
// Handle HTTP command request
// ============================================================
void handleSetRequest(String request)
{
  if (request.indexOf("cmd=start") >= 0)
  {
    motorRunning = true;
    lastStepTimeUs = micros();
  }

  if (request.indexOf("cmd=stop") >= 0)
  {
    motorRunning = false;
    releaseMotor();
  }

  if (request.indexOf("dir=cw") >= 0)
  {
    direction = 1;
  }

  if (request.indexOf("dir=ccw") >= 0)
  {
    direction = -1;
  }

  if (request.indexOf("speed=") >= 0)
  {
    int requestedSpeed = getParamValue(request, "speed", stepsPerSecond);
    setMotorSpeed(requestedSpeed);
  }
}

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);

  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(IN3_PIN, OUTPUT);
  pinMode(IN4_PIN, OUTPUT);

  releaseMotor();
  setMotorSpeed(stepsPerSecond);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println("ESP32 Stepper Motor Web Controller");
  Serial.println("Initializing...");

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
  updateStepper();

  WiFiClient client = server.available();

  if (client)
  {
    client.setTimeout(50);

    String request = client.readStringUntil('\r');
    client.flush();

    if (request.length() > 0)
    {
      Serial.println(request);

      if (request.indexOf("GET /set?") >= 0)
      {
        handleSetRequest(request);
        sendJsonResponse(client);
      }
      else if (request.indexOf("GET /data") >= 0)
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

  if (millis() - lastPrintTime >= 1000)
  {
    lastPrintTime = millis();

    Serial.print("Motor: ");
    Serial.print(motorRunning ? "Running" : "Stopped");

    Serial.print(" | Direction: ");
    Serial.print(direction == 1 ? "CW" : "CCW");

    Serial.print(" | Speed: ");
    Serial.print(stepsPerSecond);

    Serial.print(" steps/sec | RPM est: ");
    Serial.print(calculateRpm(), 2);

    Serial.print(" | Total steps: ");
    Serial.println(totalSteps);
  }
}
