# nRF24LE1 Transmitter Firmware

Firmware for a battery-powered remote built on `nRF24LE1` (32-pin target in current scripts).

It scans button inputs, builds a compact 4-byte payload, and transmits it over 2.4 GHz to the receiver.

## Quick Start

1. Build:
```bat
run.bat
```
2. Flash:
```bat
.\nRF24LE1_Programmer\Desktop\Programmer.exe \\.\COM3 w .\output.ihx
```
3. For full flashing notes, see `FLASHING_GUIDE.md`.

## Folder Overview

- `main.c`: main loop, button handling, packet creation, TX flow, power behavior.
- `radio.c`, `radio.h`: radio register setup and transmit helpers.
- `buttons.c`, `buttons.h`: button sampling and packed state output.
- `debug.c`, `debug.h`: UART debug support (compile-time controlled).
- `config.h`: RF settings, timing, button masks, packet layout, pin mapping.
- `main_rx.c`: nRF24LE1 receive-test firmware for bring-up/debug.
- `run.bat`: transmitter build script (`output.ihx`).
- `run_rx.bat`: RX test build script (`output_rx.ihx`).
- `nRF24LE1_SDK/`: local SDK used by SDCC build.
- `nRF24LE1_Programmer/`: programmer utility source/binary used for flashing.

## RF Packet Format

Each packet is 4 bytes:
- `byte0`: button bits (group 0 / P0.x)
- `byte1`: button bits (group 1 / P1.x)
- `byte2`: sequence counter
- `byte3`: checksum (`byte0 ^ byte1 ^ byte2`)

## Build Outputs

- `output.ihx`: main transmitter firmware.
- `output_rx.ihx`: local RX test firmware.

## Debug Notes

- Debug UART is enabled by uncommenting `#define DEBUG` in `config.h`.
- TX debug pin depends on package type:
  - 24-pin: `P0.5`
  - 32-pin: `P0.3`
  - 48-pin: `P1.0`
- In debug mode, the UART TX pin is excluded from button scanning.

## Toolchain

- Compiler: SDCC (`sdcc`)
- Target flags are configured in `run.bat` / `run_rx.bat` for:
  - `--model-small`
  - `--std-sdcc99`
  - `-DNRF24LE1_PACKAGE_32_PIN`

## Related Project

Receiver firmware (nRF52820) is located in the sibling folder:
- `../nrf52820-receiver/`

## References

- Nordic nRF24LE1 product page: https://www.nordicsemi.com/Products/nRF24LE1
- nRF24LE1 SDK source (Dean Cording): https://github.com/DeanCording/nRF24LE1_SDK
