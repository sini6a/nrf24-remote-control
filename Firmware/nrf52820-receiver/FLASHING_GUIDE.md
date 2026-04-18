# Receiver Source Flashing Guide

## 1) Initialize SDK submodule
On first clone, fetch the Nordic SDK:

```bash
git submodule update --init --recursive
```

## 2) Build receiver firmware
From `Firmware/usb-remote-nrf52820-receiver/`:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SENDER=OFF
cmake --build build -j4
```

Output:
- `build/hid_receiver.hex`

## 3) Flash to nRF52820

### Using pyocd
```bash
pyocd erase --target nrf52820 --chip
pyocd flash --target nrf52820 build/hid_receiver.hex -v
```

### Using DAPLink
Copy `build/hid_receiver.hex` to the DAPLink mass-storage drive that appears when the programmer is connected.

## 4) Optional sender build target
For nRF52820-to-nRF52820 RF testing:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SENDER=ON
cmake --build build -j4
```

Output:
- `build/hid_sender.hex`

## 5) Debug logs
Open the receiver CDC serial port (virtual COM port) to monitor:
- boot and radio state
- packet counters and checksums
- HID send status
- statistics: RX total, bad checksums, duplicates, HID sends, failures
