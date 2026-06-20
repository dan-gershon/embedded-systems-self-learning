# GPIO Blink Experiment

## Goal

The goal of this experiment is to verify that the ESP32 development environment is working and that a GPIO pin can be controlled from firmware.

## Hardware

- ESP32 development board
- Built-in blue LED on GPIO2
- USB cable connected to PC

## Connections

No external wiring is required.

The built-in LED is controlled through GPIO2.

## Code Behavior

The firmware configures GPIO2 as an output pin.

The LED is turned on for 1 second and then turned off for 1 second repeatedly.

## Expected Result

The blue LED on the ESP32 board should blink once per second.

## What I Learned

- How to compile and upload code to the ESP32.
- How to configure a GPIO pin as an output.
- How to use digitalWrite() to set a pin HIGH or LOW.
- How delay() affects timing in a simple firmware loop.
