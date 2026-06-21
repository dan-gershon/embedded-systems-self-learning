# Experiment 13 – ESP32 ADC Web Dashboard

## Overview

In this experiment, I built a live analog measurement dashboard using an ESP32 and a potentiometer.

Unlike the previous experiments, this project does not control an LED or another output device. The main purpose of this experiment is measurement. The ESP32 reads an analog voltage from a potentiometer using its ADC, converts the reading into voltage and percentage values, and displays the results on a web dashboard.

The dashboard is opened from a phone browser over WiFi. The browser periodically requests new data from the ESP32, and the ESP32 responds with JSON data.

This experiment focuses on the input side of embedded systems: reading a real physical signal, converting it to digital data, and presenting it through a simple web interface.

---

## Project Goals

- Read an analog voltage using the ESP32 ADC
- Use a potentiometer as a voltage divider
- Convert raw ADC values into voltage
- Convert raw ADC values into percentage
- Build a live web dashboard
- Send sensor data using JSON
- Use HTTP requests to refresh measurements
- Understand the difference between output control and input measurement

---

## Hardware Used

- ESP32 Development Board
- Potentiometer
- Breadboard
- Jumper wires
- Smartphone / web browser

---

## Circuit Connections

The potentiometer has three pins.

Connections:

3.3V → one side pin of the potentiometer

GPIO34 → middle pin of the potentiometer

GND → other side pin of the potentiometer

If the percentage goes down when turning the potentiometer in the direction I expected it to go up, the two side pins can simply be swapped.

Important note:

The potentiometer must be connected to 3.3V, not 5V, because ESP32 GPIO pins are not 5V tolerant.

---

## Image: Hardware Setup

Add the hardware setup image here:

![Hardware Setup](images/hardware_setup.jpg)

---

## Why GPIO34 Was Used

GPIO34 was selected because it belongs to ADC1.

On the ESP32, ADC2 can conflict with WiFi operation. Since this experiment uses WiFi and ADC at the same time, using an ADC1 pin is the safer and more reliable choice.

GPIO34 is also input-only, which makes it suitable for analog measurement.

---

## Potentiometer as a Voltage Divider

A potentiometer can be used as an adjustable voltage divider.

In this circuit:

One side is connected to 3.3V.

The other side is connected to GND.

The middle pin, also called the wiper, outputs a voltage between 0V and 3.3V depending on the rotation position.

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

## ADC Basics

ADC stands for Analog-to-Digital Converter.

The ADC converts an analog voltage into a digital number.

In this experiment, the ESP32 ADC was configured as a 12-bit ADC.

This means the output range is:

0 to 4095

The voltage calculation used in the code is:

voltage = raw × 3.3 / 4095

The percentage calculation is:

percent = raw × 100 / 4095

---

## Web Dashboard

The ESP32 runs a local web server.

The phone connects to the ESP32 using a browser and opens the dashboard using the ESP32 IP address.

Example:

http://192.168.1.23

The dashboard displays:

- Raw ADC value
- Measured voltage
- Potentiometer position in percent
- A live progress bar
- JSON response from the ESP32

---

## Image: Web Dashboard Low Value

Add a screenshot of the dashboard with the potentiometer near the low position:

![Web Dashboard Low](images/web_dashboard_low.jpg)

---

## Image: Web Dashboard Middle Value

Add a screenshot of the dashboard around the middle position:

![Web Dashboard Middle](images/web_dashboard_middle.jpg)

---

## Image: Web Dashboard High Value

Add a screenshot of the dashboard near the high position:

![Web Dashboard High](images/web_dashboard_high.jpg)

---

## HTTP Request

The browser periodically sends this request to the ESP32:

GET /data

This request asks the ESP32 for the latest measurement.

Unlike the previous experiments, the browser is not sending a command to control an output. Instead, it is asking the ESP32 to return the current measured input value.

---

## JSON Response

The ESP32 responds with JSON data.

Example:

{
  "raw": 2048,
  "voltage": 1.650,
  "percent": 50.0,
  "status": "ok"
}

The browser receives this JSON response, extracts the values, and updates the dashboard without refreshing the full page.

---

## JavaScript Live Update

The web page uses JavaScript to request new data repeatedly.

The dashboard updates every 300 ms.

This creates a live measurement display where turning the potentiometer immediately changes the values shown on the phone.

---

## Image: Serial Monitor

Add the Serial Monitor output here:

![Serial Monitor](images/serial_monitor.jpg)

---

## How The System Works

Potentiometer  
↓  
Analog voltage changes  
↓  
ESP32 ADC reads GPIO34  
↓  
Raw ADC value is converted to voltage and percent  
↓  
Phone browser sends GET /data request  
↓  
ESP32 returns JSON response  
↓  
Dashboard updates live  

---

## What I Learned

In this experiment I learned how to use the ESP32 as a measurement device, not only as a controller.

Main topics:

- Analog voltage measurement
- Potentiometer voltage divider
- ESP32 ADC
- ADC1 usage with WiFi
- Raw ADC values
- Voltage conversion
- Percentage mapping
- HTTP GET requests
- JSON data format
- Live web dashboard
- Embedded input measurement

---

## Result

The experiment worked successfully.

The ESP32 measured the potentiometer position using the ADC and displayed the live values on a phone browser through WiFi. The dashboard updated in real time and showed the raw ADC value, calculated voltage, percentage, progress bar, and JSON response.
