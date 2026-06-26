#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Stepper.h>

// ================= הגדרות רשת =================
const char* ssid = "Dira 36";
const char* password = "054******6";

WebServer server(80);

// ================= הגדרות חומרה =================
const int trigPin = 5;
const int echoPin = 18;

Servo tiltServo;
const int servoPin = 19;

// הגדרת מנוע צעד (שינינו את סדר הפינים לתיקון הרעידות)
const int stepsPerRevolution = 2048;
Stepper panStepper(stepsPerRevolution, 13, 12, 14, 27);

// ================= משתנים גלובליים =================
float currentDistance = 0;
int panAngle = 0;
int tiltAngle = 90;
int panDirection = 1;

unsigned long lastStepTime = 0;

// ================= דף ה-HTML שיוצג בדפדפן =================
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="he" dir="rtl">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>סורק רדאר חכם</title>
  <style>
    body { font-family: Arial, sans-serif; background-color: #2c3e50; color: white; text-align: center; padding: 20px; }
    .card { background: #34495e; padding: 20px; border-radius: 10px; display: inline-block; margin: 10px; width: 250px; box-shadow: 0px 4px 10px rgba(0,0,0,0.5); }
    h1 { color: #1abc9c; }
    .value { font-size: 2em; font-weight: bold; color: #f1c40f; margin-top: 10px; }
  </style>
  <script>
    // פונקציה שמושכת נתונים מה-ESP32 כל חצי שנייה
    setInterval(function() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('dist').innerText = data.distance + " ס\"מ";
          document.getElementById('pan').innerText = data.pan + " מעלות";
          document.getElementById('tilt').innerText = data.tilt + " מעלות";
        });
    }, 500);
  </script>
</head>
<body>
  <h1>לוח בקרה - סורק חדר</h1>
  <div>
    <div class="card">
      <h3>מרחק נוכחי</h3>
      <div class="value" id="dist">--</div>
    </div>
    <div class="card">
      <h3>זווית אופקית (צעד)</h3>
      <div class="value" id="pan">--</div>
    </div>
    <div class="card">
      <h3>זווית אנכית (סרוו)</h3>
      <div class="value" id="tilt">--</div>
    </div>
  </div>
</body>
</html>
)rawliteral";

// ================= פונקציות השרת =================
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleData() {
  // שליחת הנתונים בפורמט JSON לדפדפן
  String json = "{\"distance\":" + String(currentDistance) + 
                ",\"pan\":" + String(panAngle) + 
                ",\"tilt\":" + String(tiltAngle) + "}";
  server.send(200, "application/json", json);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  tiltServo.setPeriodHertz(50);
  tiltServo.attach(servoPin, 500, 2400);
  tiltServo.write(tiltAngle);
  
  panStepper.setSpeed(15); // מהירות מנוע הצעד (סל"ד)

  // התחברות ל-WiFi
  Serial.println("\nמתחבר ל-WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nחיבור הצליח!");
  Serial.print("כתובת ה-IP של האתר שלך היא: ");
  Serial.println(WiFi.localIP());

  // הגדרת נתיבי השרת
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

// ================= LOOP =================
void loop() {
  // 1. טיפול בבקשות של לקוחות מהדפדפן
  server.handleClient();

  // 2. מדידת מרחק (פעם ב-100 מילי-שניות כדי לא לתקוע את המעבד)
  if (millis() - lastStepTime > 100) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 20000); // 20ms timeout
    if (duration > 0) {
      currentDistance = duration * 0.034 / 2;
    } else {
      currentDistance = -1; // לא נמצא אובייקט בטווח
    }

    // 3. תנועת המנועים
    // מנוע הצעד עושה 5 צעדים בכל מחזור
    panStepper.step(5 * panDirection);
    panAngle += (panDirection * 1); // הערכה גסה לזווית
    
    // שינוי כיוון אם הגענו לקצה (סריקת מטוטלת)
    if (panAngle >= 90 || panAngle <= -90) {
      panDirection *= -1; // הפיכת כיוון
      
      // משחקים קצת עם הסרוו כשמשנים כיוון
      if (tiltAngle == 90) tiltAngle = 100;
      else tiltAngle = 90;
      tiltServo.write(tiltAngle);
    }
    
    lastStepTime = millis();
  }
}
