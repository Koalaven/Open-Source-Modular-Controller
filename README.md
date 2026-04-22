# Open Source Modular Controller

Using a modular, reconfigurable hardware architecture to create a customizable USB HID game controller.

## Project Details

The Open Source Modular Controller project is a fully modular and customizable gaming input system. Users can freely rearrange joysticks, buttons, and future input modules on a set of interchangeable receiver boards to build a layout that fits their gameplay style, ergonomic needs, or accessibility requirements.

The controller connects over USB and is recognized as a standard Human Interface Device (HID) with no additional drivers required.

<img src="/Media/OSHE_Logo_300PPI.png" width="250" >

## Current State

The current prototype includes:
- A functional ESP32‑S3‑based main controller
- Hot-swappable joystick and button modules
- Receiver boards that can be placed anywhere in the 3D‑printed shell
- USB HID communication via TinyUSB

The system operates as a recognized USB gamepad on Windows. Some hardware issues were identified and corrected in updated revisions.

## Project Structure

The repository is organized as follows:
1. **Firmware** – ESP32‑S3 code handling HID communication and module input processing.  
   *Link: [Firmware](/Controller%20Code/Firmware.md)*
2. **Software** – PC‑side utilities or test scripts (coming soon).  
3. **Hardware** – KiCad project files, schematics, PCBs, and 3D‑printed shell models.  
   *Link: [Hardware](/Hardware/Hardware.md)*
4. **Documentation** – Assembly instructions, diagrams, semester report, and testing notes.  
   *Link: [Documentation](/Documents/Documents.md)*

## Features

- Modular input modules (buttons, joystick, future sensors)
- Repositionable input layout using multiple receiver boards
- ESP32‑S3 native USB HID support
- Open‑source hardware and firmware
- Premade 3D‑printable ergonomic controller shell

## System Architecture

Core system components include:
- **Receiver boards** distributing power, signals, and micro-controller
- **Module boards** for inputs (joystick/button)
- **3D‑printed enclosure** with heat‑set inserts for secure mounting

USB HID enumeration and report handling are implemented using TinyUSB. Modules communicate through standardized connectors to the controller board.

## Bill of Materials
A full BOM including the ESP32‑S3 boards, connector sets, resistors, capacitors, PCBs, wire, and 3D‑printed components is provided in the documentation.  
*[BOM](/Documents/BOM.md)*

## Tools Used
### Software
- Arduino IDE
- KiCad
- Fusion (frame design)
- GitHub Desktop
- Cura slicer

### Hardware
- 3D printer
- Soldering iron
- Hot air station
- Multimeter
- Tweezers

## Assembly Overview
1. Gather materials (refer to BOM).  
2. Order PCBs (Gerber files included).  
3. 3D print shell and module housings.  
4. Clean and prepare boards.  
5. Solder components (headers, ESP32 module, connectors, buttons/joysticks).  
6. Upload firmware via Arduino IDE.  
7. Install modules into the shell.  
8. Test controller.  

[Full assembly guide](/Documents/Assembly%20guide.md)

## What the Project Could Use Next
1. On‑board processing for more advanced input mapping
2. PCB adjustment and testing
3. Wireless mode using Bluetooth HID
4. Additional module types (battery, gyroscope, sliders, touchpads)

## Semester End Report
For the Enterprise Program at Michigan Tech there is a end of semester report for the project to summarize the progress of the project. [Link](/Documents/Open%20Source%20Modular%20Controller%20Fall%202025%20Final%20Report.pdf)

## Acknowledgements
This project is supported by the Open Source Hardware Enterprise at Michigan Technological University.

## Helpful Links
- [TinyUSB](https://docs.tinyusb.org/en/latest/)
- [Gamepad tester](https://hardwaretester.com/gamepad)

## References

[GitHub Doc](https://docs.github.com/en/get-started/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax#links)

[Alpakka conroller](https://inputlabs.io/alpakka)

[Proteus ControllerTM kit](https://byowave.com/products/proteus-controller )

[Thrustmaster](https://eshop.thrustmaster.com/en_us/gamepads/pc.html)
