# nRF52820 Receiver Firmware

Firmware for the USB receiver dongle. It receives 4-byte packets from the nRF24LE1 remote and exposes them as USB HID input (keyboard + consumer control).

## Quick Start

```bash
# First clone only — fetch the Nordic SDK submodule
git submodule update --init --recursive

# Build
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SENDER=OFF
cmake --build build -j4

# Flash (pyocd)
pyocd erase --target nrf52820 --chip
pyocd flash --target nrf52820 build/hid_receiver.hex -v
```

For full flashing details see `FLASHING_GUIDE.md`.

## Source Layout

- `CMakeLists.txt` — build targets, include paths, linker/post-build rules
- `src/main.c` — app flow, RF receive path, HID report sending, debug logs
- `src/hid_descriptor.h` — keyboard and consumer-control HID report descriptors
- `src/sdk_config.h` — nRF5 SDK feature configuration
- `nrf5_sdk/` — Nordic SDK source tree (git submodule)
- `build/` — generated build output (not source-controlled)

## RF Packet Format

Each packet received from the remote is 4 bytes:

| Byte | Content |
|------|---------|
| 0 | Button group 0 (buttons 1–8, P0.x bits) |
| 1 | Button group 1 (buttons 9–15, P1.x bits) |
| 2 | Sequence number (0–255, wraps) |
| 3 | Checksum (`b0 ^ b1 ^ b2`) |

## Button to Key Mapping

| Remote pin | HID action |
|------------|-----------|
| P0.1 | Left arrow |
| P0.2 | Down arrow |
| P0.3 | Page Down |
| P0.4 | Escape |
| P0.5 | Volume Up (consumer) |
| P0.6 | Volume Down (consumer) |
| P0.7 | Page Up |
| P1.0 | Menu |
| P1.1 | Enter |
| P1.2 | Right arrow |
| P1.4 | F13 |
| P1.5 | Power (consumer) |
| P1.6 | Up arrow |

## USB Interfaces

- HID keyboard interface
- HID consumer-control interface (volume, power)
- CDC ACM debug serial interface (virtual COM port)

## Related Project

Transmitter firmware (nRF24LE1) is located in the sibling folder:
- `../nrf24le1-transmitter/`

## SDK References

- Nordic nRF5 SDK: https://github.com/NordicSemiconductor/nRF5-SDK
- Submodule entry tracked in `.gitmodules`
