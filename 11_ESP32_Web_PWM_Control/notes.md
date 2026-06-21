# Experiment 11 – ESP32 Web PWM Control

## Overview

This project demonstrates real-time PWM control of an LED using an ESP32 web server.

A smartphone connects to the ESP32 through a standard web browser and adjusts LED brightness using a slider interface. The ESP32 receives HTTP requests, updates the PWM duty cycle, and returns JSON responses containing the current operating parameters.

This experiment expands on the previous WiFi web server project by introducing PWM control, HTTP query parameters, JavaScript requests, and JSON-based communication.

---

## Objectives

* Connect ESP32 to a WiFi network
* Operate ESP32 as a web server
* Generate a dynamic web interface
* Control PWM output remotely
* Process HTTP GET requests
* Return JSON responses
* Introduce basic API concepts

---

## Hardware

* ESP32 Development Board
* LED
* 220Ω resistor
* Breadboard
* Jumper wires
* Smartphone

---

## Circuit

GPIO18 → 220Ω Resistor → LED Anode

LED Cathode → GND

---

## PWM Fundamentals

PWM (Pulse Width Modulation) is a technique used to generate an adjustable average voltage by rapidly switching a digital output between HIGH and LOW states.

Duty Cycle:

* 0% = LED OFF
* 50% = Medium Brightness
* 100% = Maximum Brightness

The ESP32 generates PWM at 5 kHz with 8-bit resolution.

PWM Range:

0 → 255

Examples:

* 0 = 0%
* 128 ≈ 50%
* 255 = 100%

---

## WiFi Communication

The ESP32 connects to the local WiFi network and receives an IP address from the router.

Example:

192.168.1.23

The smartphone accesses the ESP32 through:

http://192.168.1.23

The ESP32 acts as a server while the smartphone browser acts as a client.

---

## HTTP Requests

The browser sends requests such as:

GET /set?pwm=128

where:

pwm = requested PWM value

The ESP32 extracts the value and updates the LED brightness accordingly.

---

## Query Parameters

The experiment introduces URL parameters.

Example:

/set?pwm=200

The server interprets the parameter and changes the PWM duty cycle.

This mechanism is widely used in IoT devices and embedded web interfaces.

---

## JSON Responses

After processing a request, the ESP32 returns structured JSON data.

Example:

{
"pwm": 128,
"duty_percent": 50.2,
"status": "ok"
}

JSON is commonly used for communication between embedded devices, web applications, mobile applications, and cloud services.

---

## JavaScript Communication

The web page uses JavaScript fetch() requests.

Example:

fetch("/set?pwm=128")

This allows communication with the ESP32 without reloading the page.

The result is a responsive user interface with real-time control.

---

## Results

The LED brightness was successfully controlled from a smartphone through a web browser.

The system demonstrated:

* WiFi connectivity
* Embedded web serving
* PWM generation
* HTTP communication
* Query parameter processing
* JSON responses
* Real-time browser interaction

---

## Skills Acquired

* ESP32 Programming
* Embedded Systems
* PWM Generation
* WiFi Networking
* HTTP Protocol
* Client-Server Architecture
* JavaScript Fetch API
* JSON Data Exchange
* IoT Fundamentals

---

## Project Images

### Hardware Setup

Insert hardware_setup.jpg here.

### Web Interface

Insert pwm_webpage.jpg here.
