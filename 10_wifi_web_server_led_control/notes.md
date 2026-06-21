# WiFi Web Server LED Control Experiment

## Goal

The goal of this experiment is to control the ESP32 built-in LED from a phone using WiFi and a web browser.

The ESP32 creates its own WiFi network and hosts a small web page with control buttons.

This experiment introduces basic embedded networking concepts:

- WiFi Access Point
- Web server
- HTTP requests
- HTML response
- Remote GPIO control

## Hardware

- ESP32 development board
- USB cable connected to PC
- Phone with WiFi and web browser

## WiFi Mode

The ESP32 was configured as a WiFi Access Point.

This means the ESP32 creates its own WiFi network instead of connecting to an existing home router.

Network name:

```text
ESP32_LAB
```

Password:

```text
12345678
```

Default ESP32 Access Point IP address:

```text
192.168.4.1
```

## How It Works

The phone connects directly to the WiFi network created by the ESP32.

The communication path is:

```text
Phone browser -> ESP32 WiFi Access Point -> ESP32 Web Server
```

The ESP32 serves an HTML page to the phone.

The page contains buttons:

```text
LED ON
LED OFF
BLINK
```

When a button is pressed, the browser sends an HTTP request to the ESP32.

## HTTP Explanation

HTTP stands for Hypertext Transfer Protocol.

It is the protocol used by web browsers to request pages and send commands to web servers.

In this experiment, the phone browser sends requests such as:

```text
GET /on
GET /off
GET /blink
```

The ESP32 receives the request and runs the matching handler function.

Example:

```text
GET /on
```

causes the ESP32 to:

```text
blinkMode = false
ledState = true
digitalWrite(LED_PIN, HIGH)
```

Then the ESP32 sends back an HTML page to the browser.

## Web Server Routes

The ESP32 web server defines several routes:

```text
/       -> main web page
/on     -> turn LED on
/off    -> turn LED off
/blink  -> blink LED
```

Each route is connected to a function in the firmware.

For example:

```text
server.on("/on", handleOn)
```

means that when the browser requests `/on`, the function `handleOn()` is executed.

## HTML Page

The ESP32 generates a simple HTML page as a string.

The page includes:

- Title
- Explanation text
- Buttons
- Current LED status

The phone browser receives this HTML and displays it like a normal website.

## Code Behavior

The firmware does the following:

1. Configures GPIO2 as an output.
2. Starts Serial communication for debugging.
3. Creates a WiFi Access Point.
4. Starts a web server on port 80.
5. Defines HTTP routes for LED control.
6. Handles browser requests inside the main loop.
7. Updates the LED according to button presses.

## Main Loop

The main loop includes:

```text
server.handleClient()
```

This checks if a browser sent a request.

If a request was received, the ESP32 handles it.

The loop also handles blink mode using `millis()` so that the ESP32 can keep responding to web requests while blinking the LED.

## Expected Result

After uploading the firmware, the Serial Monitor should show:

```text
Starting ESP32 Access Point...
WiFi network name: ESP32_LAB
WiFi password: 12345678
Open this address in phone browser: 192.168.4.1
Web server started
```

The phone should connect to the `ESP32_LAB` WiFi network.

Opening this address in the phone browser:

```text
http://192.168.4.1
```

should display a web page with LED control buttons.

Pressing the buttons should control the ESP32 built-in LED.

## Screenshot

A screenshot from the phone browser can be added here:

```markdown
![ESP32 Web LED Control](../images/10_wifi_web_led_control_phone.png)
```

## What I Learned

- The ESP32 can create its own WiFi network.
- A phone can connect directly to the ESP32 without a router.
- The ESP32 can host a small web page.
- A browser button press becomes an HTTP request.
- HTTP routes can be used as control commands.
- Embedded firmware can combine networking, GPIO control, and HTML generation.
- WiFi control does not require a custom phone app when a browser-based interface is enough.
