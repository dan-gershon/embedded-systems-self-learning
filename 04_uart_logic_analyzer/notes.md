# UART Logic Analyzer Experiment

## Goal

The goal of this experiment is to observe UART communication as real digital electrical signals and decode it using a USB logic analyzer.

This experiment connects the theoretical UART frame structure to the actual waveform measured on the ESP32 TX pin.

## Hardware

- ESP32 development board
- 24 MHz 8-channel USB logic analyzer
- USB cable connected to PC
- Dupont wires

## Connections

Connect:

```text
ESP32 TX0 / GPIO1 -> Logic Analyzer D0
ESP32 GND         -> Logic Analyzer GND
