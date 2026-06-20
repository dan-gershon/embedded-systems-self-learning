# UART Packet, Checksum, and BER Experiment

## Goal

The goal of this experiment is to build a small UART communication lab using the ESP32.

Instead of only sending plain text, this experiment sends structured packets, receives them through a UART loopback connection, parses the received data, checks packet validity, and measures artificial bit errors.

This experiment demonstrates basic concepts used in real digital communication systems:

- Transmitter
- Receiver
- Communication channel
- Packet framing
- Checksum validation
- Error detection
- Bit Error Rate, BER

## Hardware

- ESP32 development board
- USB cable connected to PC
- One Dupont wire
- Optional: 24 MHz 8-channel USB logic analyzer

## Connections

Connect:

```text
GPIO17 -> GPIO16
```

Where:

```text
GPIO17 = UART2 TX
GPIO16 = UART2 RX
```

This creates a physical UART loopback connection.

The ESP32 transmits data from UART2 TX and receives the same data back through UART2 RX.

The USB connection is used for programming and for printing debug information to the Arduino Serial Monitor.

## UART Configuration

```text
UART port: Serial2
Baud rate: 9600
Data bits: 8
Parity: none
Stop bits: 1
Format: 8N1
TX pin: GPIO17
RX pin: GPIO16
```

## Part 1: UART2 Loopback

In the first part, the ESP32 sends simple text messages through UART2:

```text
MSG 0
MSG 1
MSG 2
...
```

Because GPIO17 is physically connected to GPIO16, the ESP32 receives its own transmitted messages.

The received data is printed to the Serial Monitor.

Expected output:

```text
Sent: MSG 0
Received: MSG 0
Sent: MSG 1
Received: MSG 1
```

## Part 2: Packet Framing

Instead of sending plain text, the firmware sends structured packets.

Packet format:

```text
<counter,value,checksum>
```

Example:

```text
<2,6,8>
```

Where:

```text
counter = 2
value = 6
checksum = 8
```

In this experiment:

```text
value = counter * 3
checksum = counter + value
```

The characters `<` and `>` are used as packet start and end markers.

The commas are used to separate fields.

## Part 3: Packet Parsing

The receiver reads a full packet and extracts the fields:

```text
counter
value
checksum
```

Then it calculates the checksum again:

```text
calculatedChecksum = counter + value
```

If:

```text
calculatedChecksum == receivedChecksum
```

Then the packet is considered valid.

Expected result:

```text
Received: counter=2, value=6, checksum=8 -> Packet OK
```

If the checksum does not match, the receiver reports:

```text
Checksum ERROR
```

## Part 4: Artificial Bit Error Rate Experiment

In a real communication system, noise can corrupt transmitted bits.

With a short wire between TX and RX, the physical UART channel is almost perfect, so this experiment injects artificial bit errors in software.

The firmware randomly flips bits before transmission according to a defined probability.

Example:

```text
BER_PPM = 10000
```

This means approximately:

```text
10000 ppm = 1% bit flip probability
```

The receiver compares the received data to the original data and counts bit errors.

The measured BER is calculated as:

```text
BER = bit_errors / total_bits
```

Expected output example:

```text
SEQ=7 | bit errors in packet=1 | checksum=ERROR | total BER=0.008928 | packets=8 | bad packets=1 | checksum errors=1
```

## What I Learned

- UART can be used not only for text but also for structured communication.
- A loopback connection is useful for testing transmit and receive logic.
- Packets need clear start and end markers.
- A checksum can detect many transmission errors.
- BER measures the ratio between wrong bits and total transmitted bits.
- A clean physical wire may have almost no real errors, so artificial noise can be used to emulate a noisy channel.
- Communication systems are built from layers: raw bits, frames, packets, validation, and error handling.
