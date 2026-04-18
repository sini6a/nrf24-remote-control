# Receiver Source Project (nRF52820)

## Purpose
This firmware runs on the nRF52820 USB receiver dongle.
It receives 4-byte packets from the nRF24LE1 remote and exposes them as USB HID input.

## Source layout
- `CMakeLists.txt`: build targets, include paths, linker/post-build rules.
- `src/main.c`: app flow, RF receive path, HID report sending, debug logs.
- `src/hid_descriptor.h`: keyboard and consumer-control HID report descriptors.
- `src/sdk_config.h`: nRF5 SDK feature configuration.
- `nrf5_sdk/`: Nordic SDK source tree used by this project.
- `build/`: generated build output (not source-controlled).

## Packet format
- Byte 0: button group 0
- Byte 1: button group 1
- Byte 2: sequence number
- Byte 3: checksum (`b0 ^ b1 ^ b2`)

## USB interfaces
- HID keyboard interface
- HID consumer-control interface (volume/power)
- CDC ACM debug serial interface

## SDK references
- Nordic nRF5 SDK repository:
  - https://github.com/NordicSemiconductor/nRF5-SDK
- Submodule entry is tracked in `.gitmodules`.
