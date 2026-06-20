# ADC Filtering Experiment

## Goal

The goal of this experiment is to compare raw ADC readings with a filtered ADC signal using a moving average filter.

The ESP32 reads an analog input from GPIO34 and sends both the raw value and the filtered value to the Arduino Serial Plotter.

This experiment demonstrates how digital filtering can reduce measurement noise.

## Hardware

- ESP32 development board
- USB cable connected to PC
- Dupont wire
- Arduino Serial Plotter

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
raw ≈ 0
filtered ≈ 0
```

### Test 2: ADC input floating

```text
GPIO34 -> not connected
```

Expected result:

```text
raw changes strongly and may jump between low and high ADC values
filtered changes more slowly and smooths the raw readings
```

### Test 3: Touching the floating input wire

```text
GPIO34 -> floating wire touched by hand
```

Expected result:

```text
The raw ADC value becomes more unstable because the body acts like an antenna.
```

## Code Behavior

The firmware reads the ADC value from GPIO34:

```text
raw = analogRead(GPIO34)
```

Then it calculates a moving average using the last 20 samples.

The Serial Plotter receives two values:

```text
raw
filtered
```

The raw signal reacts quickly and may contain large jumps.

The filtered signal is smoother because it averages multiple samples.

## Moving Average Filter

The moving average filter stores the most recent samples and calculates their average.

In this experiment:

```text
NUM_SAMPLES = 20
```

The filtered value is:

```text
filtered = sum of last 20 samples / 20
```

This reduces fast noise, but it also makes the signal respond more slowly.

## Noise vs Power

A floating ADC input can show very large readings, sometimes close to the maximum value:

```text
ADC raw ≈ 4095
```

This does not necessarily mean that a strong power source is connected to the pin.

A high ADC reading is a voltage measurement, not a power measurement.

Important distinction:

```text
High voltage reading does not necessarily mean dangerous power.
```

A floating ADC input has very high impedance and draws almost no current.  
Because there is no defined connection to GND or 3.3V, tiny charges and environmental interference can move the measured voltage significantly.

The floating wire behaves like:

```text
small antenna + small capacitor + sensitive ADC input
```

It can pick up:

- 50 Hz mains interference
- USB and computer noise
- capacitive coupling from nearby wires
- static charge
- interference from the user's hand
- internal ADC sampling effects

## Why It Does Not Burn the ESP32

The ESP32 pin is not damaged just because the ADC reading jumps high.

To damage a GPIO pin, the pin usually needs to be connected to an unsafe voltage source that can supply current.

Unsafe examples:

```text
GPIO34 -> 5V
GPIO34 -> 12V
GPIO34 -> external supply without protection
```

A floating wire or a finger touching the wire can cause a large ADC reading, but the available current is extremely small.

That is why the measurement can look dramatic without damaging the ESP32.

## Why Touching the Wire Changes the Reading

The human body can act like an antenna.

When touching a floating wire connected to GPIO34, the body couples environmental electrical noise into the ADC input.

This can make the raw ADC value jump strongly.

The input is still not connected to a strong voltage source.  
It is mainly picking up weak interference through capacitance.

## Why Floating Inputs Are Bad in Real Circuits

In a real circuit, an ADC input should not be left floating.

A floating ADC input does not measure a defined voltage.  
It measures an undefined mixture of noise, charge, and coupling from the environment.

A real circuit should provide a defined source, such as:

- sensor output
- voltage divider
- potentiometer
- pull-down resistor
- pull-up resistor

Example pull-down connection:

```text
GPIO34 -> 10kΩ -> GND
```

This forces the ADC input to a known value when no other signal is connected.

## Expected Plot

The Arduino Serial Plotter should show two curves:

```text
raw
filtered
```

For a noisy or floating input:

```text
raw      -> large fast jumps
filtered -> smoother slower curve
```

This demonstrates the tradeoff:

```text
More filtering = less noise, slower response
Less filtering = more noise, faster response
```

## Measurement Result

The raw ADC signal showed large jumps when GPIO34 was floating.

The filtered signal stayed smoother and moved more slowly around the average value.

This confirmed that the moving average filter reduced fast changes in the ADC measurement.

## What I Learned

- ADC readings can be noisy, especially when the input is floating.
- A floating ADC input can reach high raw values without being connected to a strong power source.
- High ADC reading does not necessarily mean high current or dangerous power.
- Touching a floating input can increase noise because the body acts like an antenna.
- A moving average filter smooths noisy measurements.
- Filtering reduces noise but also slows the response.
- Real ADC inputs should be connected to a defined voltage source and should not be left floating.
