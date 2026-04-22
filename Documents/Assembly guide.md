# Assembly Instructions

## 1. Gather Materials

* Refer to the **Bill of Materials (BOM)**.
* Download the main GitHub repository branch: [link](https://github.com/OSHE-Github/Open-Source-Modular-Controller/tree/main).

---

## 2. Order PCBs

1. Use a PCB manufacturer of your choice.
2. Gerber files are provided in the GitHub repository: [link](/Hardware/PCBs/gerber%20files/).
3. Boards can be ordered fully assembled or as bare PCBs.
4. Two versions exist:

   * **Custom ESP32 carrier board**
   * **ESP32-S3 DevKitC-based interposer board** (used in the shell design)

---

## 3. 3D Print the Shell

For Our Shells:
1. Body shells: [link](/Hardware/Shell/OSMC)
    - The full casing can be printed from the .step file
    - Individual case sections can be printed from the .stl files
2. Module shells: [link](/Hardware/Module%STLs)
    - Standard configurations require **7** modules: 1 Master, 2 Bumpers, 2 Joysticks, 2 Buttons
    - Each **Master** modules requires: 1 Master top casing, 1 Master bottom casing, 3 Master buttons
    - Each **Joystick** module requires: 1 Joystick top casing, 1 Joystick bottom casing
    - Each **Bumper** module requires 1 Bumper body casing and 2 Bumper caps
    - Each **Button** module requires: 1 Button top casing, 1 Button Bottom casing, 4 non-Master buttons
        - For push buttons, options include the normal button caps ([link](/Hardware/Module%STLs/OSMC_Module_Buttons_Button)) and the taller button caps ([link](/Hardware/Module%STLs/Button%Cap%Taller.stl))
        - For membrane buttons, print the SMD caps ([link](/Hardware/Module%STLs/Button%Cap$SMD.stl))
    
For **Custom** shell designs we recommend following this design process: [link](/Documents/Custom%Shell%Recommendations.md)


---

## 4. Prep the PCBs

1. **Sand** the edges to clean rough PCB cuts.
2. **Clean** with isopropyl alcohol to remove oils and residue.

---

## 5. Solder Components

Most boards in this build were hand-soldered.

### 5a. Custom ESP32-S3 Board

1. Solder **headers** on the bottom side.
2. Add remaining bottom-side components from lowest profile upward.
3. Solder the **USB port** using flux and wick.
4. Attach the **ESP32-S3 module**:

   * Pre-tin both board pads and module pads.
   * Heat with hot air until alignment occurs.
   * Hold lightly with tweezers if needed.

---

### 5b. Receiver Modules

1. Solder **female headers** at J1–J4.
2. Solder **26 AWG stranded wires** at J5 in this order (bottom → top):
   **Black – Red – Blue – Green – White – Yellow**.
3. Repeat wiring for all receiver boards.

---

### 5c. Dev Board Carrier

1. Insert headers into the ESP32 DevKit board and solder the carrier board on top.
2. Add push **buttons**.
3. Solder module wires in this order (top → bottom):
   **Red – Blue – Yellow – White – Green – Black**.

---

### 5d. Custom ESP Interposer

1. Solder header sockets.
2. Add remaining components.
   *(LEDs and their resistors are optional in this version.)*

---

### 5e. Joystick Modules

1. Solder joystick into labeled pads.
2. Insert and solder **male pin headers** on the opposite side.
3. Verify solder joints and joystick button clearance.
4. Place PCB into joystick shell and mark **up direction**.
5. Glue the top cover to the shell.

---

### 5f. Button Modules

1. Solder four push-buttons into labeled positions.
2. Insert and solder **male pin headers** on the opposite side.
3. Confirm button movement is unobstructed.
4. Place module in shell and mark **up direction**.
5. Glue the top plate onto the shell.

---

## 6. Upload Firmware

1. Install **Arduino IDE**.
2. Add ESP32 board manager URL in Preferences:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Install the **ESP32 board package**.
4. Install **ESP32TinyUSB** library.
5. Import project files.
6. Connect via USB (UART port).
7. Select:
   * **ESP32-S3 Dev Module**
   * Proper **COM port**
8. Adjust other board settings as documented.
9. Compile and upload firmware.
10. Use **UART** for programming and **USB** for HID operation.

---

## 7. Assemble the Controller Shell

1. Place Receiver boards in the main shell.
2. Mark screw hole locations.
4. Install **M3 heat-set inserts** at marked points.
5. Repeat steps **1-3** instead with the Bumper modules
6. Connect the Bumper modules to the back of the Receiver board using the **JST** connectors
7. Screw Receiver board and Bumper modules into place.
8. Insert the **ESP-32** into the pin connectors on the bottom of the Receiver board
9. Set Button, Joystick, and Master modules on their respective pogo pins following the screenprint's outline
10. Place the shell's top plate over the modules, while verifying their alignment
11. Slide the shell's bottom plate into place

Note: To rearrange the top modules, carefully push up on the tab of the top plate, and repeat steps 8-9 with the new configuration

---

## 8. Testing

1. Plug the controller with the USB port
2. Use an online tester such as [Gamepad tester](https://hardwaretester.com/gamepad).
3. Verify:

   * All buttons register
   * Joystick axes update smoothly
   * No shorts or missed inputs

If issues occur, check soldering, wiring order, and continuity.

---

**Your controller is now fully assembled and ready for use!**
