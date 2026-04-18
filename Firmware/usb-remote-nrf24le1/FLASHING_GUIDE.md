# Transmitter Flashing Guide

## Prerequisite: SDCC compiler
This transmitter firmware is compiled with **SDCC** (Small Device C Compiler).

Download SDCC:
- Official site: https://sdcc.sourceforge.net/
- Downloads page: https://sourceforge.net/projects/sdcc/files/

## 1) Build transmitter firmware
From `Desktop/src`:

```bat
run.bat
```

Output file:
- `output.ihx`

## 2) Flash to nRF24LE1
Programmer tool location in this project:
- `nRF24LE1_Programmer/Desktop/Programmer.exe`

Example command:

```bat
.\nRF24LE1_Programmer\Desktop\Programmer.exe \\.\COM3 w .\output.ihx
```

Use your actual COM port.

## 3) Build and flash RX test firmware (optional)
For nRF24LE1-to-nRF24LE1 radio testing:

```bat
run_rx.bat
Programmer.exe \\.\COM3 w .\src\output_rx.ihx
```

## 4) Debug serial notes
- Default debug baud in this project is `9600`.
- Debug pin depends on package type (set in firmware logic).
- On 32-pin devices, UART TX is typically `P0.3`.
