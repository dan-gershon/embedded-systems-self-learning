# PWM Lab

## Goal

The goal of this experiment is to generate a hardware PWM signal using the ESP32 and measure it with a USB logic analyzer.

PWM stands for Pulse Width Modulation.

PWM is used in many electrical and embedded systems applications, including:

- LED brightness control
- Motor control
- Servo control
- Buck converters
- Boost converters
- Inverters
- Digital power control

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

The ESP32 remains connected to the PC through USB for power and programming.

## PWM Configuration

For the first experiment:

```text
PWM pin: GPIO18
Frequency: 1000 Hz
Resolution: 8 bit
Duty value: 128
```

With 8-bit resolution, the duty value range is:

```text
0 to 255
```

Therefore:

```text
Duty = 128 / 255 ≈ 50%
```

## Expected Signal

The PWM frequency is:

```text
f = 1000 Hz
```

Therefore the period is:

```text
T = 1 / f = 1 / 1000 = 1 ms
```

For approximately 50% duty cycle:

```text
Ton ≈ 0.5 ms
Toff ≈ 0.5 ms
```

The logic analyzer should show a square wave on D0.

## Theory

A PWM signal switches between LOW and HIGH.

For the ESP32 GPIO:

```text
LOW  ≈ 0 V
HIGH ≈ 3.3 V
```

The average voltage depends on duty cycle:

```text
Vavg = Duty * Vhigh
```

For 50% duty:

```text
Vavg = 0.5 * 3.3 = 1.65 V
```

The ESP32 does not actually output a constant 1.65 V.

It outputs a fast digital signal switching between 0 V and 3.3 V.

A filter, motor, LED, or power converter can respond to the average effect of this switching signal.

## What I Learned

- PWM is a digital signal with a fixed frequency and variable duty cycle.
- Duty cycle controls the ratio between ON time and total period.
- ESP32 can generate PWM using hardware peripherals.
- A logic analyzer can measure PWM frequency, period, and duty cycle.
- PWM is a fundamental concept in embedded systems and power electronics.
