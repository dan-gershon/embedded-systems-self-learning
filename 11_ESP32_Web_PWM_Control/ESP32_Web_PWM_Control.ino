#include <WiFi.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const int ledPin = 18;

int pwmValue = 0;

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  ledcAttach(ledPin, 5000, 8);
  ledcWrite(ledPin, pwmValue);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop()
{
  WiFiClient client = server.available();

  if (!client)
  {
    return;
  }

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("GET /set?pwm=") >= 0)
  {
    int startIndex = request.indexOf("pwm=") + 4;
    int endIndex = request.indexOf(" ", startIndex);

    String pwmString = request.substring(startIndex, endIndex);
    pwmValue = pwmString.toInt();

    if (pwmValue < 0) pwmValue = 0;
    if (pwmValue > 255) pwmValue = 255;

    ledcWrite(ledPin, pwmValue);

    float dutyPercent = (pwmValue / 255.0) * 100.0;

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{\"pwm\":");
    client.print(pwmValue);
    client.print(",\"duty_percent\":");
    client.print(dutyPercent, 1);
    client.print(",\"status\":\"ok\"}");
    return;
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 PWM Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial;
      text-align: center;
      background-color: #111827;
      color: white;
      margin-top: 40px;
    }
    .box {
      background-color: #1f2937;
      padding: 25px;
      border-radius: 18px;
      width: 85%;
      max-width: 420px;
      margin: auto;
    }
    input[type=range] {
      width: 90%;
    }
    button {
      font-size: 18px;
      padding: 12px 20px;
      margin: 8px;
      border: none;
      border-radius: 10px;
      background-color: #38bdf8;
      color: #0f172a;
      font-weight: bold;
    }
    pre {
      text-align: left;
      background-color: #020617;
      padding: 12px;
      border-radius: 10px;
      overflow-x: auto;
    }
  </style>
</head>

<body>
  <div class="box">
    <h2>ESP32 PWM Control</h2>

    <p>PWM Value: <span id="pwmText">0</span></p>
    <p>Duty Cycle: <span id="dutyText">0</span>%</p>

    <input type="range" min="0" max="255" value="0" id="slider" oninput="setPWM(this.value)">

    <br><br>

    <button onclick="setPWM(0)">OFF</button>
    <button onclick="setPWM(128)">50%</button>
    <button onclick="setPWM(255)">MAX</button>

    <h3>ESP32 JSON Response</h3>
    <pre id="jsonBox">{}</pre>
  </div>

  <script>
    function setPWM(value) {
      fetch("/set?pwm=" + value)
        .then(response => response.json())
        .then(data => {
          document.getElementById("pwmText").innerHTML = data.pwm;
          document.getElementById("dutyText").innerHTML = data.duty_percent;
          document.getElementById("jsonBox").innerHTML = JSON.stringify(data, null, 2);
          document.getElementById("slider").value = data.pwm;
        });
    }
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
