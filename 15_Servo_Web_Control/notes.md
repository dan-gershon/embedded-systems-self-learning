# Experiment 15 – ESP32 Servo Web Control Stable Version

## Overview

In this experiment, I controlled an SG90 servo motor from a phone browser using an ESP32 web server.

The ESP32 connects to a WiFi network and serves a local web page. The web page includes a slider and preset buttons for selecting the servo angle. When the user changes the angle from the phone, the browser sends an HTTP request to the ESP32. The ESP32 receives the requested angle, converts it to a servo pulse width, moves the servo, and returns a JSON response.

This experiment is a continuation of the previous servo sweep test, but instead of moving automatically according to fixed code, the servo is now controlled wirelessly in real time from a web interface.

During the first version of this experiment, the ESP32 had lag and sometimes reconnected to WiFi. This happened because the servo motor can draw high current during movement, causing voltage drops and ESP32 resets. The stable version uses an external 5V supply for the servo, shared ground, a capacitor across the servo supply, WiFi sleep disabled, and throttled web requests.

---

## Project Goals

- Control an SG90 servo motor from a phone browser
- Use the ESP32 as a local WiFi web server
- Send angle commands using HTTP GET requests
- Use query parameters to send values from the browser to the ESP32
- Convert angle values into servo pulse width
- Return JSON responses from the ESP32
- Understand the difference between LED PWM and servo PWM
- Solve real hardware stability problems caused by motor current spikes
- Use an external 5V supply for a motor load
- Understand the importance of common ground
- Improve web control stability using throttled JavaScript requests

---

## Hardware Used

- ESP32 Development Board
- SG90 Servo Motor
- External 5V power supply
- 100uF capacitor
- Jumper wires
- Breadboard
- Smartphone / browser
- USB cable
- Optional: Multimeter

---

## Final Circuit Connections

The SG90 servo has three wires:

Brown / Black → GND

Red → 5V

Orange / Yellow → Signal

Final stable connection:

External 5V + → Servo Red

External 5V GND → Servo Brown / Black

ESP32 GPIO18 → Servo Orange / Yellow

External 5V GND → ESP32 GND

100uF capacitor + → External 5V +

100uF capacitor - → External 5V GND

The ESP32 remains powered from USB.

Important:

The external 5V supply and the ESP32 must share the same GND. Without a common ground, the servo signal has no proper voltage reference.

Do not connect the external 5V supply to the ESP32 3V3 pin.

---

## Why an External 5V Supply Was Needed

In the first version, the servo was powered directly from the ESP32 board 5V/VIN pin through USB.

This caused instability. When a servo starts moving, it can draw a short current spike. This current spike may cause the ESP32 supply voltage to drop for a moment. If the voltage drops too much, the ESP32 resets.

The symptom was:

- The web page had large lag
- The servo updated very late
- The ESP32 printed "Connecting to WiFi" again
- The ESP32 disconnected and reconnected to WiFi repeatedly

This showed that the ESP32 was probably resetting, not just experiencing a software delay.

The fix was to power the servo from a separate 5V supply while keeping the ESP32 powered by USB.

The grounds were connected together so the ESP32 signal pin and the servo control input share the same reference.

---

## Capacitor Usage

A 100uF capacitor was added across the servo supply.

Connection:

Capacitor + → Servo 5V

Capacitor - → Servo GND

The capacitor helps reduce short voltage drops when the servo starts moving. It works like a small local energy buffer near the motor.

If using an electrolytic capacitor, polarity is important. The side marked with a stripe is usually the negative side and must be connected to GND.

A larger capacitor, such as 470uF or 1000uF, could help even more, but 100uF is still useful for reducing small supply dips.

---

## Optional Multimeter Checks

Before powering the servo, the external supply can be checked with a multimeter.

Expected measurement:

External 5V + to External GND ≈ 5V

After connecting the servo:

Servo Red to Servo Brown / Black ≈ 5V

To confirm common ground:

ESP32 GND to External 5V GND ≈ 0V

This confirms that the ESP32 and servo supply share the same reference.

---

## Servo Horn Assembly

The servo motor comes with several plastic servo horns.

For this experiment, the single-arm horn is useful because it makes the angle easy to see in photos.

Before attaching the horn permanently, I first moved the servo to 90 degrees. This places the servo near its center position.

After the servo reached 90 degrees, I attached the horn in a centered orientation and tightened the small center screw.

This prevents the horn from being mounted at a random angle and helps avoid mechanical limits during movement.

---

## What Is a Servo Motor?

A servo motor is a small actuator that moves to a commanded angular position.

The SG90 servo contains:

- DC motor
- Gearbox
- Internal position sensor
- Internal controller

The ESP32 does not directly control the internal DC motor. Instead, it sends a control pulse to the servo. The servo's internal controller reads the pulse width and moves the shaft to the requested angle.

This makes the servo a small closed-loop mechanical system.

---

## Servo PWM Signal

Servo control uses PWM, but it is different from PWM used for LED brightness.

For an LED, changing the duty cycle changes the average power and therefore the brightness.

For a servo, the important value is the pulse width.

Typical servo signal:

Control frequency ≈ 50 Hz

Period ≈ 20 ms

Pulse width ≈ 500 us → about 0 degrees

Pulse width ≈ 1500 us → about 90 degrees

Pulse width ≈ 2500 us → about 180 degrees

In the stable version of this experiment, I used a safer range:

Minimum angle = 10 degrees

Maximum angle = 170 degrees

Minimum pulse width = 600 us

Maximum pulse width = 2400 us

This avoids pushing the servo into the mechanical edges, where it may buzz, draw more current, or get stuck.

---

## Angle to Pulse Width Conversion

The ESP32 receives an angle from the browser.

Example:

angle = 90

The code maps the angle range into a pulse width range.

Example mapping:

10 degrees → 600 us

90 degrees → around 1500 us

170 degrees → 2400 us

The ESP32 then sends the pulse width to the servo using:

myServo.writeMicroseconds(currentPulseUs)

This makes the relationship between command angle and physical control signal clear.

---

## Web Interface

The ESP32 serves a web page directly to the phone browser.

The web interface includes:

- Angle slider
- Preset buttons
- Current angle display
- Pulse width display
- Progress bar
- JSON response display

The slider range is 10 to 170 degrees because this is safer for the SG90 servo than using the full 0 to 180 range.

Preset buttons:

10 degrees

45 degrees

90 degrees

135 degrees

170 degrees

---

## HTTP Request

When the user changes the slider or presses a preset button, the browser sends an HTTP GET request to the ESP32.

Example:

GET /set?angle=90

The part after the question mark is the query parameter.

In this case:

angle = 90

The ESP32 extracts the angle value from the request string, limits it to the safe range, converts it to pulse width, and moves the servo.

---

## Query Parameters

A query parameter allows the browser to send data inside the URL.

Example:

/set?angle=135

This means the browser is asking the ESP32 to set the servo angle to 135 degrees.

This is the same basic idea used in many web APIs and IoT devices.

---

## JSON Response

After applying the angle command, the ESP32 sends a JSON response back to the browser.

Example:

{
  "angle": 90,
  "pulse_us": 1500,
  "min_angle": 10,
  "max_angle": 170,
  "status": "ok"
}

The browser displays this JSON response on the page.

This confirms that the ESP32 received the command and applied it.

---

## Why the First Web Version Had Lag

The first version sent a new HTTP request every time the slider moved.

When dragging a slider, the browser can generate many updates very quickly. This can overload a small embedded web server, especially when the ESP32 is also controlling a motor.

The servo also caused current spikes, and these could reset the ESP32.

So the lag came from two sources:

1. Hardware instability from servo current spikes
2. Too many HTTP requests from the slider

The stable version fixes both issues.

---

## Software Stability Improvements

The stable code includes several improvements:

### 1. WiFi Sleep Disabled

WiFi sleep was disabled using:

WiFi.setSleep(false)

This helps reduce WiFi latency and improves responsiveness.

### 2. Safer Servo Range

Instead of using 0 to 180 degrees, the servo is limited to:

10 degrees to 170 degrees

This avoids mechanical edge stress.

### 3. Safer Pulse Range

Instead of using 500 to 2500 us, the pulse range is:

600 us to 2400 us

This reduces buzzing and current spikes at the edges.

### 4. Throttled JavaScript Requests

The browser does not send a request for every tiny slider movement.

Instead, it waits a short time and sends the latest requested angle. This prevents the ESP32 from receiving too many requests too quickly.

### 5. Ignore Repeated Commands

If the requested angle is the same as the current angle, the ESP32 does not send another command to the servo.

### 6. Short Client Timeout

The ESP32 uses a short timeout for reading the HTTP client. This prevents the code from waiting too long on incomplete requests.

### 7. Close HTTP Connection

Each HTTP connection is closed after a response. This keeps the simple server stable.

---

## JavaScript Throttling

The web page uses JavaScript to control how often requests are sent.

Instead of sending a request immediately for every slider movement, the latest angle is stored and sent after a short delay.

This makes the system more stable because the ESP32 only receives the most recent useful command instead of a long queue of old commands.

This is important in embedded web control, where the device has limited processing power compared to a full computer.

---

## System Flow

Phone browser  
↓  
User moves angle slider  
↓  
JavaScript stores latest requested angle  
↓  
Browser sends throttled HTTP GET request  
↓  
ESP32 receives angle parameter  
↓  
ESP32 converts angle to pulse width  
↓  
Servo receives 50 Hz control signal  
↓  
Servo moves to requested angle  
↓  
ESP32 returns JSON response  
↓  
Browser updates angle, pulse width, and JSON display  

---

## Image Documentation

I documented the experiment using five image pairs.

Each pair includes:

1. A screenshot of the web interface showing the selected angle
2. A photo of the real servo position at that angle

This makes it easier to compare the software command with the physical result.

---

## Pair 1 – 10 Degrees

Web interface at 10 degrees:

![Web 10 Degrees](images/pair1_web_10_deg.jpg)

Servo physical position at 10 degrees:

![Servo 10 Degrees](images/pair1_servo_10_deg.jpg)

---

## Pair 2 – 45 Degrees

Web interface at 45 degrees:

![Web 45 Degrees](images/pair2_web_45_deg.jpg)

Servo physical position at 45 degrees:

![Servo 45 Degrees](images/pair2_servo_45_deg.jpg)

---

## Pair 3 – 90 Degrees

Web interface at 90 degrees:

![Web 90 Degrees](images/pair3_web_90_deg.jpg)

Servo physical position at 90 degrees:

![Servo 90 Degrees](images/pair3_servo_90_deg.jpg)

---

## Pair 4 – 135 Degrees

Web interface at 135 degrees:

![Web 135 Degrees](images/pair4_web_135_deg.jpg)

Servo physical position at 135 degrees:

![Servo 135 Degrees](images/pair4_servo_135_deg.jpg)

---

## Pair 5 – 170 Degrees

Web interface at 170 degrees:

![Web 170 Degrees](images/pair5_web_170_deg.jpg)

Servo physical position at 170 degrees:

![Servo 170 Degrees](images/pair5_servo_170_deg.jpg)

---

## What I Learned

In this experiment I learned how to control a mechanical actuator wirelessly using an ESP32.

Main topics:

- SG90 servo wiring
- Servo horn alignment
- External 5V supply for motor loads
- Common ground between different power supplies
- Capacitor usage for reducing voltage dips
- Servo PWM signal
- 50 Hz control signal
- Pulse width in microseconds
- Angle to pulse-width mapping
- ESP32 web server
- HTTP GET requests
- Query parameters
- JSON responses
- JavaScript fetch
- JavaScript request throttling
- WiFi stability considerations
- Difference between software lag and hardware resets

---

## Result

The experiment worked successfully.

The SG90 servo motor was controlled from a smartphone browser over WiFi. The web interface allowed angle control using a slider and preset buttons. The ESP32 converted the requested angle into a servo pulse width and returned a JSON response with the current angle and pulse width.

After adding an external 5V supply, shared ground, a 100uF capacitor, and software throttling, the system became more stable and responsive.
