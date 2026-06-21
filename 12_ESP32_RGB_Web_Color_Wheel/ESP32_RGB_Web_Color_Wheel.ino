#include <WiFi.h>

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// RGB LED Settings
// ============================================================
// Recommended pins for ESP32:
const int RED_PIN   = 25;
const int GREEN_PIN = 26;
const int BLUE_PIN  = 27;

// Change to true if your RGB LED is Common Anode.
// Common Cathode: common pin -> GND
// Common Anode:   common pin -> 3.3V
const bool COMMON_ANODE = false;

// PWM settings
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;   // 8-bit: 0 to 255

// Current color values
int redValue = 255;
int greenValue = 0;
int blueValue = 0;
int brightness = 100;           // 0 to 100 percent

WiFiServer server(80);

// ============================================================
// Helper: extract integer parameter from HTTP request
// Example: /set?r=255&g=80&b=10&br=100
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
// Helper: limit values
// ============================================================
int clampValue(int value, int minValue, int maxValue)
{
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

// ============================================================
// Apply RGB values to LED
// ============================================================
void applyColor()
{
  int r = (redValue   * brightness) / 100;
  int g = (greenValue * brightness) / 100;
  int b = (blueValue  * brightness) / 100;

  if (COMMON_ANODE)
  {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }

  ledcWrite(RED_PIN, r);
  ledcWrite(GREEN_PIN, g);
  ledcWrite(BLUE_PIN, b);
}

// ============================================================
// Send JSON response
// ============================================================
void sendJsonResponse(WiFiClient& client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  client.print("{\"r\":");
  client.print(redValue);
  client.print(",\"g\":");
  client.print(greenValue);
  client.print(",\"b\":");
  client.print(blueValue);
  client.print(",\"brightness\":");
  client.print(brightness);
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
  <title>ESP32 RGB Color Wheel</title>
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
      font-size: 28px;
    }

    .subtitle {
      color: #cbd5e1;
      margin-bottom: 18px;
    }

    canvas {
      width: 300px;
      height: 300px;
      max-width: 90vw;
      border-radius: 50%;
      touch-action: none;
      cursor: crosshair;
    }

    #preview {
      width: 160px;
      height: 42px;
      margin: 18px auto;
      border-radius: 10px;
      border: 2px solid white;
      background: rgb(255,0,0);
    }

    .values {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin-top: 10px;
      flex-wrap: wrap;
    }

    .valueBox {
      background: #1f2937;
      padding: 10px 14px;
      border-radius: 12px;
      min-width: 72px;
    }

    .r { color: #ff4d4d; }
    .g { color: #4dff4d; }
    .b { color: #4da3ff; }

    input[type=range] {
      width: 85%;
      margin-top: 8px;
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
    <h1>ESP32 RGB LED</h1>
    <div class="subtitle">Touch the color wheel</div>

    <canvas id="colorWheel" width="300" height="300"></canvas>

    <div id="preview"></div>

    <div class="values">
      <div class="valueBox"><b class="r">R</b><br><span id="rText">255</span></div>
      <div class="valueBox"><b class="g">G</b><br><span id="gText">0</span></div>
      <div class="valueBox"><b class="b">B</b><br><span id="bText">0</span></div>
    </div>

    <p>Brightness: <span id="brText">100</span>%</p>
    <input type="range" min="0" max="100" value="100" id="brightnessSlider"
           oninput="setBrightness(this.value)">

    <br><br>

    <button onclick="setPreset(255,0,0)">Red</button>
    <button onclick="setPreset(0,255,0)">Green</button>
    <button onclick="setPreset(0,0,255)">Blue</button>
    <button onclick="setPreset(255,255,255)">White</button>
    <button onclick="setPreset(255,255,0)">Yellow</button>
    <button onclick="setPreset(255,0,255)">Purple</button>
    <button onclick="setPreset(0,255,255)">Cyan</button>
    <button onclick="setPreset(0,0,0)">Off</button>

    <h3>ESP32 JSON Response</h3>
    <pre id="jsonBox">{}</pre>
  </div>

  <script>
    const canvas = document.getElementById("colorWheel");
    const ctx = canvas.getContext("2d");

    let currentR = 255;
    let currentG = 0;
    let currentB = 0;
    let currentBrightness = 100;

    function drawColorWheel() {
      const radius = canvas.width / 2;
      const image = ctx.createImageData(canvas.width, canvas.height);
      const data = image.data;

      for (let y = 0; y < canvas.height; y++) {
        for (let x = 0; x < canvas.width; x++) {
          const dx = x - radius;
          const dy = y - radius;
          const distance = Math.sqrt(dx * dx + dy * dy);
          const index = (y * canvas.width + x) * 4;

          if (distance <= radius) {
            let angle = Math.atan2(dy, dx);
            let hue = (angle * 180 / Math.PI + 360) % 360;
            let saturation = distance / radius;
            let rgb = hsvToRgb(hue, saturation, 1);

            data[index] = rgb.r;
            data[index + 1] = rgb.g;
            data[index + 2] = rgb.b;
            data[index + 3] = 255;
          } else {
            data[index + 3] = 0;
          }
        }
      }

      ctx.putImageData(image, 0, 0);
    }

    function hsvToRgb(h, s, v) {
      let c = v * s;
      let x = c * (1 - Math.abs((h / 60) % 2 - 1));
      let m = v - c;

      let r = 0, g = 0, b = 0;

      if (h < 60) {
        r = c; g = x; b = 0;
      } else if (h < 120) {
        r = x; g = c; b = 0;
      } else if (h < 180) {
        r = 0; g = c; b = x;
      } else if (h < 240) {
        r = 0; g = x; b = c;
      } else if (h < 300) {
        r = x; g = 0; b = c;
      } else {
        r = c; g = 0; b = x;
      }

      return {
        r: Math.round((r + m) * 255),
        g: Math.round((g + m) * 255),
        b: Math.round((b + m) * 255)
      };
    }

    function getColorFromEvent(event) {
      const rect = canvas.getBoundingClientRect();

      let clientX;
      let clientY;

      if (event.touches && event.touches.length > 0) {
        clientX = event.touches[0].clientX;
        clientY = event.touches[0].clientY;
      } else {
        clientX = event.clientX;
        clientY = event.clientY;
      }

      const scaleX = canvas.width / rect.width;
      const scaleY = canvas.height / rect.height;

      const x = (clientX - rect.left) * scaleX;
      const y = (clientY - rect.top) * scaleY;

      const radius = canvas.width / 2;
      const dx = x - radius;
      const dy = y - radius;
      const distance = Math.sqrt(dx * dx + dy * dy);

      if (distance > radius) {
        return;
      }

      let angle = Math.atan2(dy, dx);
      let hue = (angle * 180 / Math.PI + 360) % 360;
      let saturation = distance / radius;

      let rgb = hsvToRgb(hue, saturation, 1);
      setColor(rgb.r, rgb.g, rgb.b);
    }

    function setColor(r, g, b) {
      currentR = r;
      currentG = g;
      currentB = b;

      updateScreen();
      sendColor();
    }

    function setBrightness(value) {
      currentBrightness = parseInt(value);
      updateScreen();
      sendColor();
    }

    function setPreset(r, g, b) {
      setColor(r, g, b);
    }

    function updateScreen() {
      document.getElementById("rText").innerHTML = currentR;
      document.getElementById("gText").innerHTML = currentG;
      document.getElementById("bText").innerHTML = currentB;
      document.getElementById("brText").innerHTML = currentBrightness;

      const previewR = Math.round(currentR * currentBrightness / 100);
      const previewG = Math.round(currentG * currentBrightness / 100);
      const previewB = Math.round(currentB * currentBrightness / 100);

      document.getElementById("preview").style.background =
        "rgb(" + previewR + "," + previewG + "," + previewB + ")";
    }

    function sendColor() {
      fetch("/set?r=" + currentR + "&g=" + currentG + "&b=" + currentB + "&br=" + currentBrightness)
        .then(response => response.json())
        .then(data => {
          document.getElementById("jsonBox").innerHTML = JSON.stringify(data, null, 2);
        });
    }

    canvas.addEventListener("mousedown", getColorFromEvent);
    canvas.addEventListener("mousemove", function(event) {
      if (event.buttons === 1) {
        getColorFromEvent(event);
      }
    });

    canvas.addEventListener("touchstart", function(event) {
      event.preventDefault();
      getColorFromEvent(event);
    });

    canvas.addEventListener("touchmove", function(event) {
      event.preventDefault();
      getColorFromEvent(event);
    });

    drawColorWheel();
    updateScreen();
    sendColor();
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

  ledcAttach(RED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(GREEN_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(BLUE_PIN, PWM_FREQ, PWM_RESOLUTION);

  applyColor();

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
// Main Loop
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

  if (request.indexOf("GET /set?") >= 0)
  {
    redValue = getParamValue(request, "r", redValue);
    greenValue = getParamValue(request, "g", greenValue);
    blueValue = getParamValue(request, "b", blueValue);
    brightness = getParamValue(request, "br", brightness);

    redValue = clampValue(redValue, 0, 255);
    greenValue = clampValue(greenValue, 0, 255);
    blueValue = clampValue(blueValue, 0, 255);
    brightness = clampValue(brightness, 0, 100);

    applyColor();

    Serial.print("Color -> R:");
    Serial.print(redValue);
    Serial.print(" G:");
    Serial.print(greenValue);
    Serial.print(" B:");
    Serial.print(blueValue);
    Serial.print(" Brightness:");
    Serial.print(brightness);
    Serial.println("%");

    sendJsonResponse(client);
    return;
  }

  sendHtmlPage(client);
}
