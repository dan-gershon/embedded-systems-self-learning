# Experiment 19 – ESP32 Stepper Motor Web Controller

## Overview

In this experiment, I controlled a 28BYJ-48 stepper motor using an ESP32, a ULN2003 stepper motor driver, and a WiFi web dashboard.

The phone or computer browser connects to the ESP32 through WiFi. The web interface allows the user to start and stop the motor, change direction, and adjust the motor speed. The ESP32 responds with JSON data containing the current motor state, direction, speed, step delay, estimated RPM, step index, and total step count.

This experiment is different from the previous servo experiments. A servo motor receives a target angle and contains its own internal control circuit. A stepper motor does not work that way. The ESP32 must generate a correct sequence of signals to energize the motor coils step by step.

A USB logic analyzer was also used to verify the actual control signals sent from the ESP32 to the ULN2003 driver.

---

## Project Goals

- Control a 28BYJ-48 stepper motor using an ESP32
- Use a ULN2003 module as a motor driver
- Understand why a motor driver is needed
- Generate a half-step sequence
- Control motor direction by reversing the step sequence
- Control motor speed by changing the time between steps
- Build a WiFi web dashboard for motor control
- Use HTTP GET requests and query parameters
- Return motor status using JSON
- Use non-blocking timing so the motor and web server can work at the same time
- Verify the electrical step sequence using a USB logic analyzer

---

## Hardware Used

- ESP32 Development Board
- 28BYJ-48 5V Stepper Motor
- ULN2003 Stepper Motor Driver Module
- External 5V power supply
- USB Logic Analyzer
- Jumper wires
- Breadboard
- Smartphone or computer browser

---

## Circuit Connections

The 28BYJ-48 stepper motor connects directly to the white motor connector on the ULN2003 driver module.

The ESP32 connects to the ULN2003 input pins.

Connections:

ULN2003 IN1 → ESP32 GPIO25

ULN2003 IN2 → ESP32 GPIO26

ULN2003 IN3 → ESP32 GPIO27

ULN2003 IN4 → ESP32 GPIO14

ULN2003 VCC → External 5V +

ULN2003 GND → External 5V GND

ESP32 GND → External 5V GND

Important:

The stepper motor must not be powered directly from ESP32 GPIO pins. The ESP32 only sends control signals. The ULN2003 driver supplies the current to the motor coils.

The ESP32 ground and the external 5V supply ground must be connected together.

---

## Image: Hardware Setup

Add a full hardware setup image here:

![Hardware Setup](images/hardware_setup.jpg)

---

## Why a Motor Driver Is Needed

The ESP32 GPIO pins can output digital logic signals, but they cannot supply the current required by a motor coil.

The ULN2003 driver module solves this problem.

The ESP32 sends low-current logic signals to the ULN2003 input pins. The ULN2003 then switches the motor coils using the external 5V supply.

System idea:

ESP32 GPIO signals  
↓  
ULN2003 driver inputs  
↓  
ULN2003 switches motor coil current  
↓  
Stepper motor rotates  

The ESP32 is the controller, while the ULN2003 is the power-switching stage.

---

## Stepper Motor vs DC Motor

A regular DC motor spins continuously when voltage is applied.

A stepper motor works differently. It moves by energizing coils in a specific order.

Instead of simply applying voltage and letting the motor spin, the controller must create a step sequence.

Each change in the coil activation pattern moves the rotor by a small step.

This makes stepper motors useful for applications where position control is important.

---

## Stepper Motor Control Signals

The motor driver has four input signals:

IN1

IN2

IN3

IN4

Each input controls one motor coil path through the ULN2003 driver.

The ESP32 outputs a sequence of HIGH and LOW signals on these four pins.

These signals are not regular PWM signals. They are a sequence of coil activation states.

---

## Half-Step Sequence

This experiment uses a half-step sequence.

The sequence used in the code is:

| Step | IN1 | IN2 | IN3 | IN4 |
|---:|---:|---:|---:|---:|
| 0 | 1 | 0 | 0 | 0 |
| 1 | 1 | 1 | 0 | 0 |
| 2 | 0 | 1 | 0 | 0 |
| 3 | 0 | 1 | 1 | 0 |
| 4 | 0 | 0 | 1 | 0 |
| 5 | 0 | 0 | 1 | 1 |
| 6 | 0 | 0 | 0 | 1 |
| 7 | 1 | 0 | 0 | 1 |

Then the sequence repeats.

This pattern alternates between energizing one coil and two neighboring coils.

This gives smoother motion and better angular resolution than a simple full-step sequence.

---

## Why the Motor Rotates

Each coil activation creates a magnetic field inside the motor.

When IN1 is active, the rotor aligns with the magnetic field produced by that coil.

Then the code changes the active coils to IN1 + IN2.

This shifts the magnetic field slightly.

The rotor follows the moving magnetic field.

Then the pattern continues:

IN1  
↓  
IN1 + IN2  
↓  
IN2  
↓  
IN2 + IN3  
↓  
IN3  
↓  
IN3 + IN4  
↓  
IN4  
↓  
IN4 + IN1  
↓  
repeat  

The rotor follows this rotating magnetic field step by step.

---

## Direction Control

Direction is controlled by the order of the step sequence.

Forward sequence:

0 → 1 → 2 → 3 → 4 → 5 → 6 → 7

Reverse sequence:

7 → 6 → 5 → 4 → 3 → 2 → 1 → 0

The same electrical states are used, but the order is reversed.

This makes the motor rotate in the opposite direction.

In the web dashboard, the user can select:

CW

CCW

The code changes the direction variable accordingly.

---

## Speed Control

The motor speed is controlled by changing the delay between steps.

In the code:

stepDelayUs = 1000000 / stepsPerSecond

Example:

If speed = 300 steps/sec:

stepDelayUs = 3333 us

If speed = 700 steps/sec:

stepDelayUs = 1428 us

A smaller delay means the sequence advances faster, so the motor rotates faster.

A larger delay means the sequence advances more slowly, so the motor rotates more slowly.

---

## Estimated RPM

The code estimates motor speed in RPM using:

rpm = steps_per_second × 60 / steps_per_revolution

In this experiment, the 28BYJ-48 motor is treated as approximately:

4096 half-steps per output shaft revolution

This value includes the internal gearbox.

The RPM value shown on the dashboard is an estimate.

---

## Non-Blocking Motor Control

A simple way to drive a stepper motor would be to use delay between steps.

However, using delay would block the ESP32 and make the web server less responsive.

Instead, this experiment uses non-blocking timing with micros().

The main loop keeps running continuously.

Every loop cycle, the code checks whether enough time has passed since the last step.

If enough time passed, the code advances the motor by one step.

If not, the ESP32 continues handling WiFi and browser requests.

This allows the motor to rotate while the web dashboard remains responsive.

---

## Web Dashboard

The ESP32 runs a local web server.

The browser opens the ESP32 IP address and displays a control dashboard.

The dashboard includes:

- Motor state: Running / Stopped
- Start button
- Stop button
- Direction buttons: CW / CCW
- Speed slider
- Direction display
- RPM estimate
- Step delay
- Total step counter
- JSON response display

---

## Image: Web Dashboard Running

Add screenshot of the dashboard while the motor is running:

![Web Dashboard Running](images/web_dashboard_running.jpg)

---

## Image: Web Dashboard Stopped

Add screenshot of the dashboard while the motor is stopped:

![Web Dashboard Stopped](images/web_dashboard_stopped.jpg)

---

## HTTP Commands

The dashboard sends HTTP GET requests to control the motor.

Start motor:

GET /set?cmd=start

Stop motor:

GET /set?cmd=stop

Set clockwise direction:

GET /set?dir=cw

Set counterclockwise direction:

GET /set?dir=ccw

Set speed:

GET /set?speed=300

The ESP32 reads these query parameters and updates the motor state.

---

## JSON Response

The ESP32 returns JSON status data.

Example:

{
  "running": true,
  "direction": "CW",
  "steps_per_second": 300,
  "step_delay_us": 3333,
  "rpm_estimate": 4.39,
  "step_index": 2,
  "total_steps": 1520,
  "status": "ok"
}

The JSON response confirms the current motor state.

---

## Logic Analyzer Verification

A USB logic analyzer was used to verify the actual stepper control signals.

The logic analyzer was connected to the ULN2003 input pins.

Connections:

Logic Analyzer GND → Common GND

CH0 → ULN2003 IN1

CH1 → ULN2003 IN2

CH2 → ULN2003 IN3

CH3 → ULN2003 IN4

The logic analyzer was connected in parallel with the existing signals. It only observed the logic signals and did not replace any connection.

The logic analyzer ground had to be connected to the common system ground. Without a shared ground, the analyzer would not have a valid voltage reference for reading HIGH and LOW levels.

---

## Important Logic Analyzer Note

The logic analyzer must be connected to the input side of the ULN2003 driver, not to the motor wires.

Correct measurement points:

ULN2003 IN1-IN4

Not recommended:

Motor coil wires

ULN2003 output pins

The IN1-IN4 signals are safe 3.3V logic signals generated by the ESP32.

---

## Image: Logic Analyzer Connection

Add image of the USB logic analyzer connection here:

![Logic Analyzer Connection](images/logic_analyzer_connection.jpg)

---

## Image: Logic Analyzer Half-Step Sequence

Add screenshot of the captured half-step sequence here:

![Logic Analyzer Half-Step Sequence](images/logic_analyzer_half_step_sequence.png)

---

## What the Logic Analyzer Shows

The logic analyzer shows the four control signals over time.

The captured signals show the half-step sequence:

IN1  
IN1 + IN2  
IN2  
IN2 + IN3  
IN3  
IN3 + IN4  
IN4  
IN4 + IN1  

Then the sequence repeats.

This confirms that the ESP32 is generating the correct coil activation pattern.

The capture also shows that changing speed in the web dashboard changes the timing between transitions.

Lower speed results in wider spacing between signal changes.

Higher speed results in narrower spacing between signal changes.

Changing direction reverses the order of the sequence.

---

## Why D4-D7 Were Flat

Only four logic analyzer channels were used for this experiment:

D0 to D3

These correspond to:

D0 = IN1

D1 = IN2

D2 = IN3

D3 = IN4

Channels D4 to D7 were not connected to active signals, so they remained flat.

This is normal.

---

## Torque Discussion

Torque was not directly measured in this experiment.

The ESP32 controls the step sequence and speed, but without a current sensor or torque sensor it cannot calculate real mechanical torque.

A torque estimate based only on software commands would not be reliable, because actual torque depends on:

- Motor current
- Supply voltage
- Coil resistance
- Driver voltage drop
- Motor speed
- Load
- Missed steps
- Internal gearbox behavior

A future improvement could use a current sensor such as INA219 or ACS712 to measure motor current and estimate torque more meaningfully.

A shunt resistor method was considered, but only 10Ω resistors were available. Using many 10Ω resistors in parallel would be physically messy and not ideal for this setup, so torque measurement was left as a future improvement.

---

## System Flow

Phone browser  
↓  
User presses Start / Stop or changes speed / direction  
↓  
Browser sends HTTP GET request  
↓  
ESP32 updates motor state  
↓  
ESP32 advances half-step sequence using non-blocking timing  
↓  
ULN2003 driver switches current through motor coils  
↓  
Stepper motor rotates  
↓  
Browser requests status data  
↓  
ESP32 returns JSON response  
↓  
Dashboard updates live  
↓  
Logic analyzer verifies IN1-IN4 signals  

---

## What I Learned

In this experiment I learned how to control a stepper motor from an ESP32 and how to verify the control signals electrically.

Main topics:

- 28BYJ-48 stepper motor
- ULN2003 motor driver
- Why motor drivers are needed
- Half-step sequence
- Coil activation order
- Direction control by reversing the sequence
- Speed control by changing step timing
- Non-blocking timing with micros()
- ESP32 web server
- HTTP GET commands
- Query parameters
- JSON responses
- Web-based motor control
- Logic analyzer measurement
- Signal verification on IN1-IN4
- Difference between observing software behavior and measuring real electrical signals

---

## Result

The experiment worked successfully.

The ESP32 controlled the 28BYJ-48 stepper motor through the ULN2003 driver. The web dashboard allowed the motor to start, stop, change direction, and change speed.

The USB logic analyzer confirmed that the ESP32 generated the expected half-step sequence on the ULN2003 input pins.

This experiment demonstrated not only motor control, but also electrical verification of the digital control signals.
