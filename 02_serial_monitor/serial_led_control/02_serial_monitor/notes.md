# Serial Monitor LED Control Experiment

## Goal

The goal of this experiment is to control the ESP32 built-in LED using text commands sent from the Arduino Serial Monitor.

This experiment demonstrates basic serial communication between the PC and the ESP32.

## Hardware

- ESP32 development board
- Built-in blue LED on GPIO2
- USB cable connected to PC

## Connections

No external wiring is required.

The ESP32 communicates with the PC through the USB connection.

## Code Behavior

The firmware listens for commands from the Serial Monitor.

Supported commands:

```text
on
off
blink
