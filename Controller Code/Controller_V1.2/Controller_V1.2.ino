const int deadzone = 9;
const int polldelay = 10;

#include "USB.h"
#include "USBHIDGamepad.h"
#include "Gamepad_config.h"

#if CFG_TUD_HID
USBHIDGamepad gamepad;

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
