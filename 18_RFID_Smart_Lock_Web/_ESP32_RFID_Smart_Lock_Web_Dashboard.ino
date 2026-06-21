#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ============================================================
// Experiment 18 – ESP32 RFID Smart Lock Web Dashboard
//
// This project uses an RC522 RFID reader to identify RFID cards.
// If the scanned UID matches the authorized UID, the ESP32 unlocks
// an SG90 servo lock for a few seconds and then locks it again.
//
// A WiFi web dashboard shows:
// - Lock state
// - Last scanned UID
// - Access result
// - Scan counter
// - Servo angle
// - JSON response
//
// Main concepts:
// - RFID UID reading
// - SPI communication
// - Access control logic
// - Servo actuator control
// - External 5V supply for motor load
// - ESP32 WiFi web server
// - HTTP GET /data request
// - JSON response
// - Embedded system state management
//
// RC522 Connections:
// - SDA / SS  -> GPIO21
// - SCK       -> GPIO18
// - MOSI      -> GPIO23
// - MISO      -> GPIO19
// - RST       -> GPIO22
// - IRQ       -> Not connected
// - GND       -> ESP32 GND
// - 3.3V      -> ESP32 3.3V
//
// Servo Connections:
// - Servo Brown / Black        -> External 5V GND
// - Servo Red                  -> External 5V +
// - Servo Orange / Yellow      -> ESP32 GPIO27
// - External 5V GND            -> ESP32 GND
// - Optional capacitor +       -> External 5V +
// - Optional capacitor -       -> External 5V GND
//
// Important:
// - RC522 must be powered from 3.3V, not 5V.
// - Servo should be powered from external 5V.
// - ESP32 and external 5V supply must share GND.
// ============================================================

// ============================================================
// WiFi Settings
// ============================================================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ============================================================
// RFID Settings
// ============================================================
const int RFID_SS_PIN = 21;
const int RFID_RST_PIN = 22;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// Authorized card UID:
byte authorizedUid[] = { 0x24, 0x84, 0x01, 0x07 };
byte authorizedUidSize = 4;

// ============================================================
// Servo Settings
// ============================================================
const int SERVO_PIN = 27;

const int LOCKED_ANGLE = 10;
const int UNLOCKED_ANGLE = 90;

const int MIN_PULSE_US = 600;
const int MAX_PULSE_US = 2400;

Servo lockServo;

// ============================================================
// Lock Timing
// ============================================================
const unsigned long UNLOCK_TIME_MS = 3000;

bool lockIsOpen = false;
unsigned long unlockStartTime = 0;

// ============================================================
// System State
// ============================================================
String lastUidString = "None";
String lastAccessResult = "Waiting";
String lockStateText = "Locked";

unsigned long totalScans = 0;
unsigned long grantedScans = 0;
unsigned long deniedScans = 0;

int currentServoAngle = LOCKED_ANGLE;

WiFiServer server(80);

// ============================================================
// Helper: Print UID into String
// ============================================================
String uidToString(byte *uid, byte uidSize)
{
  String result = "";

  for (byte i = 0; i < uidSize; i++)
  {
    if (uid[i] < 0x10)
    {
      result += "0";
    }

    result += String(uid[i], HEX);

    if (i < uidSize - 1)
    {
      result += " ";
    }
  }

  result.toUpperCase();
  return result;
}

// ============================================================
// Helper: Compare scanned UID with authorized UID
// ============================================================
bool isAuthorized(byte *uid, byte uidSize)
{
  if (uidSize != authorizedUidSize)
  {
    return false;
  }

  for (byte i = 0; i < uidSize; i++)
  {
    if (uid[i] != authorizedUid[i])
    {
      return false;
    }
  }

  return true;
}

// ============================================================
// Servo control
// ============================================================
void moveLockServo(int angle)
{
  currentServoAngle = angle;
  lockServo.write(angle);
}

void lockDoor()
{
  moveLockServo(LOCKED_ANGLE);
  lockIsOpen = false;
  lockStateText = "Locked";

  Serial.println("Door locked");
}

void unlockDoor()
{
  moveLockServo(UNLOCKED_ANGLE);
  lockIsOpen = true;
  unlockStartTime = millis();
  lockStateText = "Unlocked";

  Serial.println("Door unlocked");
}

// ============================================================
// RFID Scan Handling
// ============================================================
void checkRfid()
{
  if (!rfid.PICC_IsNewCardPresent())
  {
    return;
  }

  if (!rfid.PICC_ReadCardSerial())
  {
    return;
  }

  totalScans++;

  lastUidString = uidToString(rfid.uid.uidByte, rfid.uid.size);

  Serial.println("RFID card detected");
  Serial.print("UID: ");
  Serial.println(lastUidString);

  if (isAuthorized(rfid.uid.uidByte, rfid.uid.size))
  {
    grantedScans++;
    lastAccessResult = "Access Granted";

    Serial.println("ACCESS GRANTED");
    unlockDoor();
  }
  else
  {
    deniedScans++;
    lastAccessResult = "Access Denied";

    Serial.println("ACCESS DENIED");
  }

  Serial.println("--------------------------------");

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(300);
}

// ============================================================
// Auto-lock after unlock time
// ============================================================
void updateLockTimer()
{
  if (lockIsOpen && (millis() - unlockStartTime >= UNLOCK_TIME_MS))
  {
    lockDoor();
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

  client.print("{\"lock_state\":\"");
  client.print(lockStateText);

  client.print("\",\"last_uid\":\"");
  client.print(lastUidString);

  client.print("\",\"access_result\":\"");
  client.print(lastAccessResult);

  client.print("\",\"servo_angle\":");
  client.print(currentServoAngle);

  client.print(",\"total_scans\":");
  client.print(totalScans);

  client.print(",\"granted_scans\":");
  client.print(grantedScans);

  client.print(",\"denied_scans\":");
  client.print(deniedScans);

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
  <title>ESP32 RFID Smart Lock</title>
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
      font-size: 44px;
      font-weight: bold;
      margin: 16px 0;
    }

    .locked {
      color: #ef4444;
    }

    .unlocked {
      color: #22c55e;
    }

    .waiting {
      color: #38bdf8;
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
      font-size: 19px;
      font-weight: bold;
      margin-top: 6px;
      word-break: break-word;
    }

    .uidBox {
      background: #020617;
      border-radius: 12px;
      padding: 14px;
      margin-top: 14px;
      font-size: 22px;
      font-weight: bold;
      color: #facc15;
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
    <h1>ESP32 RFID Smart Lock</h1>
    <div class="subtitle">RC522 + SPI + Servo Lock + Web Dashboard</div>

    <div id="lockState" class="statusBig waiting">Waiting</div>

    <div class="label">Last UID</div>
    <div id="uidText" class="uidBox">None</div>

    <div class="grid">
      <div class="card">
        <div class="label">Access Result</div>
        <div class="smallValue" id="accessText">Waiting</div>
      </div>

      <div class="card">
        <div class="label">Servo Angle</div>
        <div class="smallValue"><span id="angleText">10</span>&deg;</div>
      </div>

      <div class="card">
        <div class="label">Total Scans</div>
        <div class="smallValue" id="totalText">0</div>
      </div>

      <div class="card">
        <div class="label">Granted / Denied</div>
        <div class="smallValue">
          <span id="grantedText">0</span> / <span id="deniedText">0</span>
        </div>
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
          const lockStateElement = document.getElementById("lockState");

          lockStateElement.innerHTML = data.lock_state;

          if (data.lock_state === "Locked") {
            lockStateElement.className = "statusBig locked";
          } else if (data.lock_state === "Unlocked") {
            lockStateElement.className = "statusBig unlocked";
          } else {
            lockStateElement.className = "statusBig waiting";
          }

          document.getElementById("uidText").innerHTML = data.last_uid;
          document.getElementById("accessText").innerHTML = data.access_result;
          document.getElementById("angleText").innerHTML = data.servo_angle;

          document.getElementById("totalText").innerHTML = data.total_scans;
          document.getElementById("grantedText").innerHTML = data.granted_scans;
          document.getElementById("deniedText").innerHTML = data.denied_scans;

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
  delay(1000);

  Serial.println();
  Serial.println("ESP32 RFID Smart Lock Web Dashboard");
  Serial.println("Initializing...");

  SPI.begin(18, 19, 23, 21);
  rfid.PCD_Init();

  Serial.println("RC522 initialized");

  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, MIN_PULSE_US, MAX_PULSE_US);
  lockDoor();

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

  Serial.println("Place RFID card/tag near the reader...");
}

// ============================================================
// Main Loop
// ============================================================
void loop()
{
  checkRfid();
  updateLockTimer();

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
}
