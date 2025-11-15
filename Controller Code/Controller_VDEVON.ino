const int deadzone = 9;  // Set deadzone
const int polldelay = 10;  // Set 100Hz polling rate
bool devMode = false;  // devMode is off
bool PSMode = true;  // Playstation configuration by default
bool XBOXMode = false;  // Xbox configuration swap

#include "USB.h"
#include "USBHIDGamepad.h"

#if CFG_TUD_HID
USBHIDGamepad gamepad;

// Button Pins ABXY & LR
int PIN_L1 = 9;
int PIN_L2 = 8;

int PIN_A = 39;
int PIN_B = 14;
int PIN_X = 16;
int PIN_Y = 17;

int PIN_R2 = 3;
int PIN_R1 = 18;

// Joystick Pins (Analog Inputs)
int PIN_Ly = 6;  // Left Y-axis XBOX: 11 PS5: 6
int PIN_Ry = 2;  // Right Y-axis BOTH: 2 
int PIN_Lx = 7;  // Left X-axis  XBOX: 12 PS5: 7
int PIN_Rx = 4;  // Right X-axis BOTH: 4 

// Joystick Click Buttons
int PIN_L3 = 5;   // Left stick click XBOX: 10 PS5: 5
int PIN_R3 = 40;  // Right stick click BOTH: 40

// Additional Analog Inputs
int PIN_Lx1 = 13;  // Secondary Left X
int PIN_Rx1 = 1;   // Secondary Right X

// D-Pad Buttons
int PIN_up    = 11;  // XBOX: 6 PS5: 11
int PIN_down  = 13;  // XBOX: 15 PS5: 13
int PIN_left  = 10;  // XBOX: 5 PS5: 10
int PIN_right = 12;  // XBOX: 7 PS5: 12

// Center Buttons
int PIN_HOME   = 21;
int PIN_START  = 47;
int PIN_SELECT = 48;

void setup() {
  pinMode(PIN_Ry, INPUT);
  pinMode(PIN_Rx, INPUT);
  pinMode(PIN_Ly, INPUT);
  pinMode(PIN_Lx, INPUT);

  pinMode(PIN_R1, INPUT_PULLUP);
  pinMode(PIN_L1, INPUT_PULLUP);
  pinMode(PIN_R2, INPUT_PULLUP);
  pinMode(PIN_L2, INPUT_PULLUP);
  pinMode(PIN_R3, INPUT_PULLUP);
  pinMode(PIN_L3, INPUT_PULLUP);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_X, INPUT_PULLUP);
  pinMode(PIN_Y, INPUT_PULLUP);

  pinMode(PIN_up, INPUT_PULLUP);
  pinMode(PIN_down, INPUT_PULLUP);
  pinMode(PIN_left, INPUT_PULLUP);
  pinMode(PIN_right, INPUT_PULLUP);

  pinMode(PIN_START, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_HOME, INPUT_PULLUP);

  Serial.begin(115200);
  delay(10);
  gamepad.begin();
}

void loop() {
  // -------------------
  // Read analog joysticks
  // -------------------
  // Map ADC values (0–4095 on ESP32) to HID range (-127..127)
  int lxVal = map(analogRead(PIN_Lx), 0, 4095, -127, 127);
  if (abs(lxVal) < deadzone) {
    lxVal = 0;
  }
  int lyVal = map(analogRead(PIN_Ly), 0, 4095, -127, 127);
  if (abs(lyVal) < deadzone) {
    lyVal = 0;
  }
  int rxVal = map(analogRead(PIN_Rx), 0, 4095, -127, 127);
  if (abs(rxVal) < deadzone) {
    rxVal = 0;
  }
  int ryVal = map(analogRead(PIN_Ry), 0, 4095, -127, 127);
  if (abs(ryVal) < deadzone) {
    ryVal = 0;
  }

  // Triggers (if you want them analog, otherwise treat as buttons)
  int l2Val = !digitalRead(PIN_L2) ? 127 : 0;
  int r2Val = !digitalRead(PIN_R2) ? 127 : 0;

  // -------------------
  // Read buttons
  // -------------------
  uint32_t buttons = 0;
  if (!digitalRead(PIN_A)) buttons |= (1 << 0);
  if (!digitalRead(PIN_B)) buttons |= (1 << 1);
  if (!digitalRead(PIN_X)) buttons |= (1 << 2);
  if (!digitalRead(PIN_Y)) buttons |= (1 << 3);

  if (!digitalRead(PIN_R1)) buttons |= (1 << 4);
  if (!digitalRead(PIN_L1)) buttons |= (1 << 5);
  if (!digitalRead(PIN_R3)) buttons |= (1 << 6);
  if (!digitalRead(PIN_L3)) buttons |= (1 << 7);

  if (!digitalRead(PIN_START)) buttons |= (1 << 8);
  if (!digitalRead(PIN_SELECT)) buttons |= (1 << 9);
  if (!digitalRead(PIN_HOME)) buttons |= (1 << 10);

  // -------------------
  // Read D-pad → hat
  // -------------------
  uint8_t hat = 0;                                              // default center
  if (!digitalRead(PIN_up) && !digitalRead(PIN_right)) hat = 2;         // up-right
  else if (!digitalRead(PIN_up) && !digitalRead(PIN_left)) hat = 8;     // up-left
  else if (!digitalRead(PIN_down) && !digitalRead(PIN_right)) hat = 4;  // down-right
  else if (!digitalRead(PIN_down) && !digitalRead(PIN_left)) hat = 6;   // down-left
  else if (!digitalRead(PIN_up)) hat = 1;
  else if (!digitalRead(PIN_right)) hat = 3;
  else if (!digitalRead(PIN_down)) hat = 5;
  else if (!digitalRead(PIN_left)) hat = 7;

  // Disable all inputs except for center inputs while devMode active
  if (devMode) {
    lxVal = lyVal = rxVal = ryVal = 0;
    l2Val = r2Val = 0;
    hat = 0;
    buttons &= ( (1<<8) | (1<<9) | (1<<10));
  }

  static int onHoldTime = 0;
  static bool devModeTriggered = 0;
  static int devModeDelay = 0;
  // Toggle a developer mode to configure layout on controller
  if (!digitalRead(PIN_START) && !digitalRead(PIN_SELECT)) {
    // Check if start + select held for 3 seconds
    if (onHoldTime == 0) onHoldTime = millis();
    if (millis() - onHoldTime > 3000 && !devMode) {
      // Activate devMode
      devMode = true;
      devModeTriggered = true;
      onHoldTime = 0;
    }
  } else {
    // Reset holdtime if not held long enough
    onHoldTime = 0;
  }
 
  // Provide indicator that devMode activated (we need LEDs)
  if (devModeTriggered) {
    devModeDelay = millis();
    devModeTriggered = false;
  }
  // B3 press indicates activation (gamepadtester)
  if (millis() - devModeDelay < 200) {
    buttons |= (1 << 3);
  }

  static int offHoldTime = 0;
  static bool devModeoffTriggered = 0;
  static int devModeoffDelay = 0;
  // Turn off devMode
  if (!digitalRead(PIN_START) && !digitalRead(PIN_SELECT) && devMode) {
    // Check if start + select held for 1 seconds
    if (offHoldTime == 0) offHoldTime = millis();
    if (millis() - offHoldTime > 1000) {
      devMode = false;
      devModeoffTriggered = true;
      offHoldTime = 0;
    }
  } else {
    offHoldTime = 0;    
  }
  
  // Provide indicator that devMode deactivated (we need LEDs)
  if (devModeoffTriggered) {
    devModeoffDelay = millis();
    devModeoffTriggered = false;
  }
  // B2 press indicates activation (gamepadtester)
  if (millis() - devModeoffDelay < 200) {
    buttons |= (1 << 2);
  }
  
  // Prevent multiple swaps due to polling rate
  static bool configSwap = false;
  // Change the controller configuration on home press if devMode is enabled
  if (!digitalRead(PIN_HOME) && devMode) {
    // Swaps configuration to xbox
    if (PSMode && !configSwap) {
      // Joystick Pins (Analog Inputs)
      PIN_Ly = 11;  // Left Y-axis XBOX: 11 PS5: 6
      PIN_Lx = 12;  // Left X-axis  XBOX: 12 PS5: 7
       
      pinMode(PIN_Ly, INPUT);
      pinMode(PIN_Lx, INPUT);

      // Joystick Click Buttons
      PIN_L3 = 10;  // Left stick click XBOX: 10 PS5: 5

      // D-Pad & Control Buttons
      PIN_up    = 6;  // XBOX: 6 PS5: 11
      PIN_down  = 15;  // XBOX: 15 PS5: 13
      PIN_left  = 5;  // XBOX: 5 PS5: 10
      PIN_right = 7;  // XBOX: 7 PS5: 12

      pinMode(PIN_up, INPUT_PULLUP);
      pinMode(PIN_down, INPUT_PULLUP);
      pinMode(PIN_left, INPUT_PULLUP);
      pinMode(PIN_right, INPUT_PULLUP);

      // Update current configuration
      PSMode = false;
      XBOXMode = true;
      configSwap = true;
    } else if (XBOXMode && !configSwap) {
      // Swaps configuration to playstation
      PIN_Ly = 6;  // Left Y-axis XBOX: 11 PS5: 6
      PIN_Lx = 7;  // Left X-axis  XBOX: 12 PS5: 7

      pinMode(PIN_Ly, INPUT);
      pinMode(PIN_Lx, INPUT);
      
      // Joystick Click Buttons
      PIN_L3 = 5;  // Left stick click XBOX: 10 PS5: 5

      // D-Pad & Control Buttons
      PIN_up    = 11;  // XBOX: 6 PS5: 11
      PIN_down  = 13;  // XBOX: 15 PS5: 13
      PIN_left  = 10;  // XBOX: 5 PS5: 10
      PIN_right = 12;  // XBOX: 7 PS5: 12

      pinMode(PIN_up, INPUT_PULLUP);
      pinMode(PIN_down, INPUT_PULLUP);
      pinMode(PIN_left, INPUT_PULLUP);
      pinMode(PIN_right, INPUT_PULLUP);

      // Update current configuration
      PSMode = true;
      XBOXMode = false;
      configSwap = true;
    }
  } else {
    // Reset configSwap upon release
    configSwap = false;
  }

  // -------------------
  // Send state to HID
  // -------------------
  gamepad.send(
    lxVal,    // LX
    lyVal,    // LY
    l2Val,    // L2 (treated as analog axis)
    rxVal,    // RX
    ryVal,    // RY
    r2Val,    // R2 (treated as analog axis)
    hat,      // D-pad hat
    buttons   // 32 buttons bitmask
  );
  Serial.println("Joystick position");
  Serial.printf("Left y: %d x: %d \nRight y: %d x: %d \n", lyVal, lxVal, ryVal, rxVal);
  Serial.printf("Button State A B X Y R1 L1 R2 L2 R3 L3 \n%d \n", buttons);
  Serial.printf("Hat position: %d \n", hat);

  delay(polldelay);  // Small poll delay
}

#endif
