# nRF24-Based Remote Control

This project contains source code for both the transceiver and the receiver. Currently, only the hardware design files are available, as the PCBs have just been ordered for manufacturing.  

The software will be released soon after testing and programming are completed.  

## Transceiver  

The transceiver is based on the **nRF24LE1**, which uses a proprietary 2.4 GHz technology called **ShockBurst**. This chip operates within a voltage range of **1.9V to 3.6V**, making it ideal for **two AAA batteries**.  

### Features:
- **15 keys** – two of which are positioned on the PCB's side. These side keys are not intended for use but may be useful for experimentation.  
- **Antenna placement** – positioned on the top side of the PCB for improved signal performance.  
- **Programming interface** – uses an **SPI interface** and a **TagConnect 6-pin NL connector** for flashing.  

## Receiver  

The receiver is based on the **nRF52820**, the most cost-effective chip that supports **USB HID** natively. It is an ARM-based microcontroller that uses **SWD and SWDCLK lines** for flashing.  

### Features:
- **USB HID support** – translates the received 2.4 GHz signal into a USB interface.  
- **Programming interface** – uses the same connector as the transceiver for simplicity.  
- **No additional I/O usage** – the chip's primary function is signal conversion.  
- **Hardware design** – includes a **PCB antenna** and a **PCB USB 2.0 connector**.  

## Further Work  

As soon as the PCBs arrive, I will begin testing and programming the devices to work together.  

- [ ] Program the remote  
- [ ] Program the receiver  
- [ ] Test stability and antenna performance  
- [ ] Design a 3D-printed case  

## Contribution  

Contributions are welcome! If you plan to contribute, please open an issue first to discuss your proposed changes.  
