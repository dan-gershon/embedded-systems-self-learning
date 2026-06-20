# ADC Lab

## Goal

The goal of this experiment is to measure an analog voltage using the ESP32 ADC and display the measured value over time using the Arduino Serial Plotter.

This experiment moves from purely digital signals to analog measurement.

## Hardware

- ESP32 development board
- USB cable connected to PC
- Dupont wires

## ADC Pin

The ADC input used in this experiment is:

```text
GPIO34
```

GPIO34 is input-only and is suitable for analog measurements on the ESP32.

## Connections Tested

### Test 1: ADC connected to GND

```text
GPIO34 -> GND
```

Expected result:

```text
ADC raw ≈ 0
```

### Test 2: ADC input floating

```text
GPIO34 -> not connected
```

Expected result:

```text
ADC raw changes due to floating input noise
```

A floating analog input is not connected to a defined voltage, so it can pick up noise from the environment, nearby wires, the computer, and the user touching the circuit.

### Test 3: ADC connected to 3.3V

```text
GPIO34 -> 3V3
```

Expected result:

```text
ADC raw ≈ 4095
```

The ESP32 ADC is 12-bit by default, so the raw value range is approximately:

```text
0 to 4095
```

## Code Behavior

The firmware repeatedly reads the ADC value from GPIO34:

```text
adcRaw = analogRead(GPIO34)
```

Then it prints the raw value to the Serial output.

The Arduino Serial Plotter can display this value as a live graph.

## Why Serial Plotter Was Used

A USB logic analyzer measures digital logic levels such as LOW and HIGH.

It is useful for:

- UART
- SPI
- I2C
- PWM timing
- Digital signal timing

However, it does not measure true analog voltage levels like an oscilloscope.

For this ADC experiment, the analog value is measured by the ESP32 and sent to the computer as serial data.

The Arduino Serial Plotter then displays the ADC value as a graph.

## Expected Result

When GPIO34 is connected to GND, the graph should stay close to zero.

When GPIO34 is floating, the graph may show noise and unstable values.

When GPIO34 is connected to 3.3V, the graph should move close to the maximum ADC value.

## What I Learned

- The ESP32 can measure analog voltage using its ADC.
- ADC converts voltage into a digital number.
- A 12-bit ADC produces values from approximately 0 to 4095.
- A floating analog input can pick up noise.
- A logic analyzer is not an analog oscilloscope.
- Arduino Serial Plotter can be used to visualize ADC readings over time.
