# Experiment 17 – ESP32 Ultrasonic Distance Web Dashboard

## Overview

In this experiment, I built a live distance measurement system using an ESP32 and an HC-SR04 ultrasonic sensor.

The ESP32 triggers the ultrasonic sensor, measures the echo return time, calculates the distance to an object, and displays the result on a web dashboard. A phone or computer browser connects to the ESP32 over WiFi and receives live distance data through JSON responses.

This experiment is different from the previous ADC-based potentiometer experiment because the measured quantity is not voltage. Instead, the ESP32 measures time. The distance is calculated using the time it takes for an ultrasonic sound wave to travel to an object and return.

---

## Project Goals

- Use an HC-SR04 ultrasonic sensor with the ESP32
- Generate a trigger pulse from the ESP32
- Measure echo pulse duration
- Calculate distance using time-of-flight
- Understand the speed of sound relation
- Protect ESP32 input pins using a voltage divider
- Build a live WiFi web dashboard
- Return measurement data using JSON
- Display distance, filtered distance, echo time, and measurement status

---

## Hardware Used

- ESP32 Development Board
- HC-SR04 Ultrasonic Sensor
- 1kΩ resistor
- 2kΩ resistor, or two 1kΩ resistors in series
- Breadboard
- Jumper wires
- Smartphone / browser
- USB cable

---

## Circuit Connections

The HC-SR04 ultrasonic sensor has four pins:

VCC

TRIG

ECHO

GND

Final connections used in this experiment:

HC-SR04 VCC → ESP32 VIN / 5V

HC-SR04 GND → ESP32 GND

HC-SR04 TRIG → ESP32 GPIO18

HC-SR04 ECHO → voltage divider → ESP32 GPIO35

GPIO18 was used for TRIG because it is a regular output-capable GPIO pin and was easy to access on my ESP32 board.

GPIO35 was used for ECHO because it is an input pin, which is suitable for reading the echo pulse.

---

## Important ESP32 Voltage Note

The HC-SR04 is usually powered from 5V.

Its ECHO pin can output a 5V signal.

ESP32 GPIO pins are not 5V tolerant.

Because of this, the ECHO signal must not be connected directly to the ESP32.

A voltage divider was used to reduce the ECHO signal from about 5V to about 3.3V.

---

## Echo Voltage Divider

The voltage divider connection:

HC-SR04 Echo → 1kΩ resistor → ESP32 GPIO35

ESP32 GPIO35 → 2kΩ resistor → GND

Diagram:

HC-SR04 Echo  
↓  
1kΩ resistor  
↓  
GPIO35  
↓  
2kΩ resistor  
↓  
GND

This reduces the voltage approximately according to:

Vout = Vin × R2 / (R1 + R2)

With:

R1 = 1kΩ

R2 = 2kΩ

If Vin = 5V:

Vout = 5 × 2 / (1 + 2)

Vout ≈ 3.33V

This is safe for the ESP32 input.

If a 2kΩ resistor is not available, two 1kΩ resistors can be connected in series to create 2kΩ.

---

## Image: Hardware Setup

Add hardware setup image here:

![Hardware Setup](images/hardware_setup.jpg)

---

## How the HC-SR04 Works

The HC-SR04 ultrasonic sensor measures distance using sound waves.

The ESP32 sends a short trigger pulse to the TRIG pin.

The sensor then emits a short ultrasonic burst at about 40kHz.

This sound wave travels through the air, hits an object, and reflects back to the sensor.

The sensor outputs a pulse on the ECHO pin.

The width of the ECHO pulse represents how long the sound wave traveled.

The ESP32 measures this pulse duration and calculates the distance.

---

## Time-of-Flight Measurement

This experiment is based on time-of-flight measurement.

The measured time includes both directions:

Sensor → object

Object → sensor

Because the sound wave travels to the object and back, the calculated distance must be divided by 2.

Distance formula:

distance_cm = duration_us × 0.0343 / 2

Where:

duration_us = echo pulse width in microseconds

0.0343 cm/us = approximate speed of sound in air

The division by 2 is needed because the echo time is the round-trip travel time.

---

## Example Calculation

If the ECHO pulse duration is:

duration_us = 1000 us

Then:

distance_cm = 1000 × 0.0343 / 2

distance_cm = 17.15 cm

So an echo duration of 1000 us corresponds to an object about 17.15 cm away.

---

## Measurement Range

The HC-SR04 is commonly used for distances around:

2 cm to 400 cm

Very close objects may not be measured correctly.

Very far objects may produce weak or missing echoes.

Soft, angled, or irregular surfaces may also give unstable readings because the sound wave may not reflect directly back to the sensor.

---

## Filtering

Raw ultrasonic measurements can jump slightly because of noise, reflections, and object surface shape.

The code uses a simple smoothing filter:

filtered = alpha × new_measurement + (1 - alpha) × previous_filtered

This makes the displayed distance more stable.

In this experiment:

FILTER_ALPHA = 0.25

A higher value reacts faster but is less smooth.

A lower value is smoother but reacts more slowly.

---

## Web Dashboard

The ESP32 runs a local web server.

The phone or computer browser connects to the ESP32 IP address.

Example:

http://192.168.1.23

The dashboard displays:

- Distance in cm
- Filtered distance in cm
- Echo pulse duration in us
- Measurement validity
- Distance bar
- JSON response

The dashboard updates periodically by requesting new data from the ESP32.

---

## HTTP Request

The browser sends this request repeatedly:

GET /data

This asks the ESP32 for the latest distance measurement.

The ESP32 responds with JSON data.

---

## JSON Response

Example JSON response:

{
  "distance_cm": 25.4,
  "filtered_distance_cm": 24.9,
  "duration_us": 1481,
  "valid": true,
  "status": "ok"
}

The JSON includes:

distance_cm:

The latest calculated distance.

filtered_distance_cm:

The smoothed distance value.

duration_us:

The measured ECHO pulse width.

valid:

Whether the measurement was considered valid.

status:

General response status.

---

## Why This Experiment Is Important

Previous experiments measured analog voltage using the ADC.

This experiment measures time.

This is an important difference.

Many embedded systems measure physical quantities by measuring time, pulse width, frequency, or delay.

Examples:

- Ultrasonic distance sensors
- Tachometers
- Encoders
- PWM input signals
- Communication protocols
- Time-of-flight sensors
- Frequency counters

This experiment introduces the idea that a microcontroller is not only a voltage-measuring device. It can also measure events in time.

---

## Image: Close Object

Web dashboard with close object:

![Web Dashboard Close Object](images/web_dashboard_close_object.jpg)

Sensor setup with close object:

![Sensor Close Object](images/sensor_close_object.jpg)

---

## Image: Medium Object

Web dashboard with medium object:

![Web Dashboard Medium Object](images/web_dashboard_medium_object.jpg)

Sensor setup with medium object:

![Sensor Medium Object](images/sensor_medium_object.jpg)

---

## Image: Far Object

Web dashboard with far object:

![Web Dashboard Far Object](images/web_dashboard_far_object.jpg)

Sensor setup with far object:

![Sensor Far Object](images/sensor_far_object.jpg)

---

## Image: Serial Monitor

Add Serial Monitor image here:

![Serial Monitor](images/serial_monitor.jpg)

---

## How The System Works

ESP32 sends 10 us trigger pulse on GPIO18  
↓  
HC-SR04 emits ultrasonic sound burst  
↓  
Sound wave travels to object  
↓  
Sound wave reflects back to sensor  
↓  
HC-SR04 outputs ECHO pulse  
↓  
ESP32 reads pulse duration on GPIO35  
↓  
ESP32 calculates distance  
↓  
ESP32 filters the result  
↓  
Browser sends GET /data request  
↓  
ESP32 returns JSON response  
↓  
Web dashboard updates live  

---

## What I Learned

In this experiment I learned how to measure distance using time-of-flight.

Main topics:

- HC-SR04 ultrasonic sensor
- Trigger and Echo signals
- Pulse duration measurement
- Time-of-flight calculation
- Speed of sound
- Why the distance calculation is divided by 2
- ESP32 GPIO output for trigger
- ESP32 GPIO input for echo
- Voltage divider for 5V to 3.3V protection
- WiFi web server
- HTTP GET request
- JSON response
- Live web dashboard
- Filtering noisy measurements

---

## Result

The experiment worked successfully.

The ESP32 triggered the HC-SR04 ultrasonic sensor, measured the echo pulse duration, calculated the distance to an object, and displayed the live result on a web dashboard.

This experiment added a new type of embedded measurement to the project series: time-based sensing.
