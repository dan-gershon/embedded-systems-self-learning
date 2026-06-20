# PWM Duty Ramp Experiment

## Goal

The goal of this experiment is to generate a PWM signal whose duty cycle changes automatically over time.

The ESP32 gradually increases the duty cycle from 0% to 100%, then decreases it back to 0%.

This experiment demonstrates how firmware can dynamically control a PWM output, similar to how an embedded controller changes a setpoint in a real system.

## Hardware

- ESP32 development board
- USB cable connected to PC
- USB logic analyzer
- Dupont wires

## Connections

Connect:

```text
ESP32 GPIO18 -> Logic Analyzer D0
ESP32 GND    -> Logic Analyzer GND
```

The ESP32 remains connected to the PC through USB for programming and Serial Monitor output.

## PWM Configuration

```text
PWM pin: GPIO18
Frequency: 1000 Hz
Resolution: 8 bit
Duty range: 0% to 100%
Duty step: 5%
Step time: 300 ms
```

## Code Behavior

The firmware starts with:

```text
Duty = 0%
```

Then it increases the duty cycle in steps:

```text
0%, 5%, 10%, 15%, ... , 100%
```

After reaching 100%, the direction is reversed:

```text
100%, 95%, 90%, ... , 0%
```

The current duty cycle is printed to the Serial Monitor.

## Expected Result

The logic analyzer should show a PWM waveform on D0.

As the duty cycle increases:

```text
Low duty  -> short HIGH time and long LOW time
50% duty  -> approximately equal HIGH and LOW time
High duty -> long HIGH time and short LOW time
```

## Measurement Result

The PWM duty ramp was measured using PulseView and a USB logic analyzer.

Three different duty cycle regions were captured:

```text
Low duty  -> narrow HIGH pulses
Mid duty  -> HIGH and LOW times are closer to equal
High duty -> wide HIGH pulses
```

The measured waveform confirmed that the ESP32 changed the PWM duty cycle over time.

## Theory

PWM duty cycle is defined as:

```text
Duty = Ton / T
```

Where:

```text
Ton = time the signal is HIGH
T   = total PWM period
```

For a PWM frequency of 1000 Hz:

```text
T = 1 / 1000 = 1 ms
```

Examples:

```text
Duty = 10% -> Ton ≈ 0.1 ms
Duty = 50% -> Ton ≈ 0.5 ms
Duty = 90% -> Ton ≈ 0.9 ms
```

For a 3.3 V GPIO signal, the average voltage is approximately:

```text
Vavg = Duty * 3.3 V
```

The ESP32 does not output a true analog voltage.  
It outputs a fast digital signal that switches between 0 V and 3.3 V.

## What I Learned

- PWM duty cycle can be changed dynamically in firmware.
- A ramp can be created by updating duty cycle in small steps over time.
- The Serial Monitor can be used to track the current duty cycle.
- A logic analyzer can confirm that the electrical signal changes as expected.
- PWM ramp behavior is useful for control systems, motor control, LED dimming, and power electronics.
