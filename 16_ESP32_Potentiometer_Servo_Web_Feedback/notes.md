# Experiment 16 – ESP32 Potentiometer Controlled Servo with Web Feedback

## Overview

In this experiment, I built a small embedded system that combines analog input, mechanical output, and WiFi feedback.

A potentiometer is connected to the ESP32 and used as an analog input. The ESP32 reads the potentiometer using its ADC, converts the reading into voltage and percentage values, maps the value to a servo angle, and moves an SG90 servo motor.

At the same time, the ESP32 runs a local web server. A phone browser connects to the ESP32 and displays a live dashboard showing the raw ADC value, measured voltage, potentiometer percentage, servo angle, pulse width, and JSON response.

This experiment combines concepts from previous experiments: ADC measurement, servo control, external servo power supply, HTTP communication, JSON responses, and live web dashboards.

---

## Project Goals

- Read a potentiometer using the ESP32 ADC
- Use the potentiometer as a physical control input
- Convert ADC raw values into voltage and percentage
- Map analog input to servo angle
- Control an SG90 servo motor using PWM
- Use an external 5V supply for the servo
- Use common ground between ESP32 and external supply
- Add a capacitor to reduce voltage dips
- Display live system data on a phone browser
- Return sensor and actuator data as JSON
- Understand a complete input-processing-output system

---

## Hardware Used

- ESP32 Development Board
- Potentiometer
- SG90 Servo Motor
- External 5V power supply
- 100uF capacitor
- Breadboard
- Jumper wires
- USB cable
- Smartphone / browser
- Optional: Multimeter

---

## Circuit Connections

### Potentiometer

The potentiometer has three pins.

Connections:

ESP32 3.3V → one side pin

ESP32 GPIO34 → middle pin

ESP32 GND → other side pin

The middle pin is the wiper. It outputs a voltage between 0V and 3.3V depending on the rotation position.

Important:

The potentiometer must be connected to 3.3V, not 5V, because ESP32 GPIO pins are not 5V tolerant.

---

### Servo Motor

The SG90 servo has three wires:

Brown / Black → GND

Red → 5V

Orange / Yellow → Signal

Final stable connections:

External 5V + → Servo Red

External 5V GND → Servo Brown / Black

ESP32 GPIO18 → Servo Orange / Yellow

External 5V GND → ESP32 GND

The ESP32 remains powered from USB.

Important:

The external 5V supply and the ESP32 must share the same GND. Without a common ground, the servo signal does not have a proper reference.

---

### Capacitor

A 100uF capacitor was connected across the servo power supply.

Capacitor + → External 5V +

Capacitor - → External 5V GND

The capacitor helps reduce short voltage dips when the servo starts moving.

If the capacitor is electrolytic, polarity is important. The stripe on the capacitor usually marks the negative side.

---

## Image: Hardware Setup

Add hardware setup image here:

![Hardware Setup](images/hardware_setup.jpg)

---

## Why GPIO34 Was Used

GPIO34 was used for the potentiometer because it belongs to ADC1.

On ESP32, ADC2 can conflict with WiFi operation. Since this experiment uses WiFi and ADC at the same time, ADC1 is the safer choice.

GPIO34 is also input-only, which is suitable for analog measurement.

---

## Potentiometer as a Voltage Divider

The potentiometer is used as an adjustable voltage divider.

One side is connected to 3.3V.

The other side is connected to GND.

The middle pin outputs a variable voltage between 0V and 3.3V.

Expected values:

At one end:

Voltage ≈ 0V

ADC raw value ≈ 0

At the middle:

Voltage ≈ 1.65V

ADC raw value ≈ 2048

At the other end:

Voltage ≈ 3.3V

ADC raw value ≈ 4095

---

## ADC Measurement

ADC stands for Analog-to-Digital Converter.

The ESP32 ADC converts the analog voltage from the potentiometer into a digital number.

In this project, the ADC is configured as 12-bit.

ADC range:

0 to 4095

Voltage calculation:

voltage = raw × 3.3 / 4095

Percentage calculation:

percent = raw × 100 / 4095

---

## Mapping ADC to Servo Angle

The measured ADC value is mapped to a servo angle.

The servo is controlled in a safe range:

10 degrees to 170 degrees

This avoids the mechanical edges of the SG90 servo.

Mapping idea:

ADC raw value near 0 → servo near 10 degrees

ADC raw value near 2048 → servo near 90 degrees

ADC raw value near 4095 → servo near 170 degrees

This converts a physical knob position into a mechanical servo position.

---

## Servo PWM Signal

Servo control uses PWM, but it is different from LED brightness PWM.

For LED brightness, the duty cycle controls the average power.

For a servo, the important value is the pulse width.

Typical servo timing:

Control frequency ≈ 50 Hz

Period ≈ 20 ms

Pulse width ≈ 500 us → about 0 degrees

Pulse width ≈ 1500 us → about 90 degrees

Pulse width ≈ 2500 us → about 180 degrees

In this experiment, a safer pulse range was used:

600 us to 2400 us

This reduces buzzing and current spikes at the edges.

---

## Filtering and Deadband

ADC readings can have small noise. If the servo reacts to every tiny change, it may jitter.

To reduce this, the code uses two methods:

### Simple Filtering

The measured ADC value is smoothed using a simple low-pass filter.

This makes the value more stable.

### Angle Deadband

The servo is updated only when the calculated angle changes enough.

This prevents tiny ADC noise from constantly sending new servo commands.

Together, these methods make the servo movement smoother and more stable.

---

## External 5V Supply

The servo is powered from an external 5V supply instead of directly from the ESP32 board.

This is important because a servo can draw high current when it starts moving. If the servo is powered from the ESP32 USB supply, the voltage may drop and reset the ESP32.

Using an external 5V supply makes the system more stable.

The grounds must still be connected together:

External 5V GND → ESP32 GND

---

## Web Dashboard

The ESP32 runs a local web server.

The phone connects to the ESP32 IP address through a browser.

The dashboard displays:

- Raw ADC value
- Measured voltage
- Potentiometer percentage
- Servo angle
- Servo pulse width
- JSON response
- Live angle progress bar

The dashboard updates every 300 ms.

---

## HTTP Request

The browser periodically sends:

GET /data

This asks the ESP32 for the latest system state.

Unlike the previous servo web control experiment, the browser is not commanding the servo. The potentiometer controls the servo physically, while the browser only displays live feedback.

---

## JSON Response

The ESP32 responds with JSON data.

Example:

{
  "raw": 2048,
  "filtered_raw": 2047.5,
  "voltage": 1.650,
  "percent": 50.0,
  "angle": 90,
  "pulse_us": 1500,
  "min_angle": 10,
  "max_angle": 170,
  "status": "ok"
}

This JSON response contains both input data and output data.

Input data:

- raw
- voltage
- percent

Output data:

- angle
- pulse_us

This makes the dashboard useful for understanding the whole system.

---

## Image: Low Angle

Web dashboard at low angle:

![Web Dashboard Low Angle](images/web_dashboard_low_angle.jpg)

Servo at low angle:

![Servo Low Angle](images/servo_low_angle.jpg)

---

## Image: Middle Angle

Web dashboard at middle angle:

![Web Dashboard Middle Angle](images/web_dashboard_mid_angle.jpg)

Servo at middle angle:

![Servo Middle Angle](images/servo_mid_angle.jpg)

---

## Image: High Angle

Web dashboard at high angle:

![Web Dashboard High Angle](images/web_dashboard_high_angle.jpg)

Servo at high angle:

![Servo High Angle](images/servo_high_angle.jpg)

---

## Image: Serial Monitor

Add Serial Monitor image here:

![Serial Monitor](images/serial_monitor.jpg)

---

## How The System Works

Potentiometer rotation  
↓  
Analog voltage changes  
↓  
ESP32 reads voltage using ADC on GPIO34  
↓  
ADC value is filtered  
↓  
Filtered value is converted to voltage and percent  
↓  
Filtered value is mapped to servo angle  
↓  
ESP32 sends servo PWM signal on GPIO18  
↓  
SG90 servo moves to the calculated angle  
↓  
Phone browser sends GET /data  
↓  
ESP32 returns JSON response  
↓  
Dashboard updates live  

---

## What I Learned

In this experiment I learned how to build a complete embedded system that uses input, processing, output, and communication.

Main topics:

- Potentiometer voltage divider
- ESP32 ADC measurement
- ADC1 usage with WiFi
- Raw ADC to voltage conversion
- Raw ADC to percentage conversion
- Mapping input range to output range
- Servo motor control
- Servo PWM pulse width
- External power supply for motor loads
- Common ground
- Capacitor usage for voltage stability
- Filtering noisy analog signals
- Deadband to reduce servo jitter
- ESP32 web server
- HTTP GET requests
- JSON responses
- Live web dashboard

---

## Result

The experiment worked successfully.

The potentiometer controlled the SG90 servo angle in real time. The ESP32 measured the analog input, calculated the corresponding servo angle, moved the servo, and displayed all live values on a phone browser through WiFi.

This experiment demonstrated a complete embedded system flow:

Input → Processing → Output → Communication
