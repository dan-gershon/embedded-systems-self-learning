# Experiment 12 – ESP32 RGB Web Color Wheel

## Overview

In this experiment, I built a WiFi-based RGB LED controller using an ESP32.

The ESP32 runs a local web server, and the phone connects to it through a regular web browser. Instead of using three separate sliders for red, green, and blue, the web page includes a color wheel. Touching a point on the color wheel sends the selected RGB values to the ESP32.

The ESP32 then updates three PWM outputs, one for each LED color channel, and returns a JSON response with the current values.

This experiment continues the WiFi and PWM work from the previous experiments, but adds multi-channel PWM, a more advanced web interface, and HTTP requests with multiple parameters.

---

## Project Goals

- Control an RGB LED from a phone using WiFi
- Use ESP32 as a local web server
- Build a browser-based color wheel interface
- Convert touch position into RGB values
- Send multiple values using HTTP query parameters
- Control three PWM channels at the same time
- Return JSON data from the ESP32 to the browser
- Understand the basic idea behind smart lighting controllers

---

## Hardware Used

- ESP32 Development Board
- RGB LED
- 3 resistors, 220Ω or 330Ω
- Breadboard
- Jumper wires
- Smartphone / browser

---

## Circuit Connections

The RGB LED has four pins:

- Red
- Green
- Blue
- Common

In this experiment I used the RGB LED as a common cathode LED.

Connections:

GPIO25 → resistor → Red pin

GPIO26 → resistor → Green pin

GPIO27 → resistor → Blue pin

Common pin → GND

Each color channel must have its own resistor. Using only one resistor on the common pin is not recommended, because each color inside the RGB LED has a different voltage drop and current behavior.

---

## Image: Hardware Setup

Add the hardware setup image here:

![Hardware Setup](images/hardware_setup.jpg)

---

## RGB LED Notes

An RGB LED is basically three LEDs inside one package:

- Red LED
- Green LED
- Blue LED

By changing the intensity of each internal LED, different colors can be created.

Examples:

Red:

R = 255, G = 0, B = 0

Green:

R = 0, G = 255, B = 0

Blue:

R = 0, G = 0, B = 255

White:

R = 255, G = 255, B = 255

Yellow:

R = 255, G = 255, B = 0

Cyan:

R = 0, G = 255, B = 255

Purple:

R = 255, G = 0, B = 255

---

## PWM Control

The ESP32 controls the brightness of each RGB channel using PWM.

PWM stands for Pulse Width Modulation.

Instead of outputting a real analog voltage, the ESP32 switches each GPIO pin very quickly between HIGH and LOW. The average power delivered to the LED depends on the duty cycle.

In this experiment:

PWM frequency = 5 kHz

PWM resolution = 8 bit

PWM range = 0 to 255

0 means fully off.

255 means maximum brightness.

Because there are three PWM channels, the ESP32 can control red, green, and blue separately.

---

## Web Interface

The web page includes:

- A color wheel
- A color preview box
- RGB value display
- Brightness slider
- Color preset buttons
- JSON response display

The color wheel is drawn using HTML Canvas and JavaScript.

When I touch the color wheel, the browser calculates the selected RGB values and sends them to the ESP32.

---

## Image: Web Color Wheel

Add the web interface screenshot here:

![Web Color Wheel](images/web_color_wheel.jpg)

---

## HTTP Request

The browser sends an HTTP GET request to the ESP32.

Example:

GET /set?r=255&g=80&b=20&br=100

The values mean:

r = red value

g = green value

b = blue value

br = brightness percentage

This experiment is different from the previous PWM experiment because now the request contains multiple parameters instead of only one PWM value.

---

## Query Parameters

The part after the question mark in the URL contains query parameters.

Example:

/set?r=255&g=80&b=20&br=100

The parameters are separated using the & symbol.

This allows the browser to send several values to the ESP32 in one request.

The ESP32 reads these values from the request string, limits them to valid ranges, and updates the PWM outputs.

---

## JSON Response

After updating the LED color, the ESP32 sends a JSON response back to the browser.

Example:

{
  "r": 255,
  "g": 80,
  "b": 20,
  "brightness": 100,
  "status": "ok"
}

JSON is a structured data format commonly used in web applications, APIs, IoT devices, and embedded systems.

In this project, the JSON response is displayed directly on the web page so I can see what the ESP32 received and applied.

---

## Image: RGB LED Results

Red LED result:

![RGB LED Red](images/rgb_led_red.jpg)

Green LED result:

![RGB LED Green](images/rgb_led_green.jpg)

Blue LED result:

![RGB LED Blue](images/rgb_led_blue.jpg)

Custom selected color:

![RGB LED Custom Color](images/rgb_led_custom_color.jpg)

---

## How The System Works

Phone browser  
↓  
User touches the color wheel  
↓  
JavaScript calculates RGB values  
↓  
Browser sends HTTP request  
↓  
ESP32 receives r, g, b, and brightness values  
↓  
ESP32 updates 3 PWM channels  
↓  
RGB LED changes color  
↓  
ESP32 returns JSON response  
↓  
Browser displays the current values  

---

## What I Learned

In this experiment I learned how to combine several embedded and web concepts into one working system.

Main topics:

- ESP32 WiFi connection
- Embedded web server
- Client-server communication
- HTTP GET requests
- Query parameters
- JSON responses
- JavaScript fetch
- HTML Canvas
- RGB color model
- 3-channel PWM control
- Basic IoT device architecture

---

## Result

The experiment worked successfully.

The RGB LED color can be controlled wirelessly from a smartphone through a web browser. The color changes in real time according to the selected point on the color wheel, and the ESP32 returns a JSON response with the current RGB and brightness values.
