# nRF24-Based Remote Control

This project contains source code for both the transmitter and the receiver. The PCBs and the code is successfully tested and working as intended. The first revision of the remote needs soldering external wires for programming but this has been fixed with updated programming header in the current revision.

## Transmitter  

The transmitter is based on the **nRF24LE1**, which uses a proprietary 2.4 GHz technology called **ShockBurst**. This chip operates within a voltage range of **1.9V to 3.6V**, making it ideal for **two AAA batteries**.  

### Features:
- **15 keys** – two of which are positioned on the PCB's side. These side keys are not intended for use but may be useful for experimentation.  
- **Antenna placement** – positioned on the top side of the PCB for improved signal performance.  
- **Programming interface** – uses an **SPI interface** and additional PROG pin to flash it through Arduino. Pinout is printed on the board.  

## Receiver  

The receiver is based on the **nRF52820**, the most cost-effective chip that supports **USB HID** natively. It is an ARM-based microcontroller that uses **SWD and SWDCLK lines** for flashing.  

### Features:
- **USB HID support** – translates the received 2.4 GHz signal into a USB interface.  
- **Programming interface** – uses the **TagConnect 6-pin NL connector**.  
- **No additional I/O usage** – the chip's primary function is signal conversion.  
- **Hardware design** – includes a **PCB antenna** and a **PCB USB 2.0 connector**.  

## Further Work  

As soon as the PCBs arrive, I will begin testing and programming the devices to work together.  

- [x] Program the remote  
- [x] Program the receiver  
- [x] Test stability and antenna performance  
- [x] Design a 3D-printed case  

## Contribution  

Contributions are welcome! If you plan to contribute, please open an issue first to discuss your proposed changes.  
