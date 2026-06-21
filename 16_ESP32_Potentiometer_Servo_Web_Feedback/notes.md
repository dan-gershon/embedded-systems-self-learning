// ============================================================
// Experiment 16 – ESP32 Potentiometer Controlled Servo
// with Web Feedback
//
// This project combines analog input, mechanical output, and
// WiFi communication.
//
// A potentiometer is read using the ESP32 ADC. The measured
// analog value is converted to voltage, percent, and servo angle.
// The ESP32 then moves an SG90 servo motor to the calculated
// angle and displays live data on a phone browser.
//
// Main concepts:
// - Potentiometer as voltage divider
// - ESP32 ADC1 measurement with WiFi
// - Raw ADC to voltage conversion
// - Raw ADC to percent conversion
// - Analog input to servo angle mapping
// - Servo PWM signal
// - External 5V supply for motor load
// - Common ground
// - Simple filtering
// - Deadband to reduce jitter
// - ESP32 web server
// - HTTP GET /data request
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
//
// Suggested images for GitHub:
// - images/hardware_setup.jpg
// - images/web_dashboard_low_angle.jpg
// - images/servo_low_angle.jpg
// - images/web_dashboard_mid_angle.jpg
// - images/servo_mid_angle.jpg
// - images/web_dashboard_high_angle.jpg
// - images/servo_high_angle.jpg
// - images/serial_monitor.jpg
// ============================================================
