# Receiver Source Flashing Guide

## 1) Build receiver firmware
From `receiver/src`:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SENDER=OFF
cmake --build build -j4
```

Output:
- `build/hid_receiver.hex`

## 2) Flash to nRF52820
Using `pyocd`:

```bash
pyocd erase --target nrf52 --chip
pyocd flash --target nrf52 build/hid_receiver.hex -v
```

Using DAPLink.

## 3) Optional sender build target
For nRF52820-to-nRF52820 RF testing:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DBUILD_SENDER=ON
cmake --build build -j4
```

Output:
- `build/hid_sender.hex`

## 4) Debug logs
Open the receiver CDC serial port to monitor:
- boot and radio state
- packet counters/checksums
- HID send status

## 5) SDK submodule reference
`nrf5_sdk` is managed as a git submodule:

```bash
git submodule update
```
