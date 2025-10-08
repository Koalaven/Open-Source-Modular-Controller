/**
 * Simple HID gamepad, 32 buttons + 2x 3 axis + 8 position hat
 * author: chegewara
 * library used ESP32TinyUSB
 */

unsigned long previousMillis = 0;
int state = 0;
bool testing = false;
const int deadzone = 9;
const int polldelay = 10;

#include "hidgamepad.h"
#if CFG_TUD_HID
HIDgamepad gamepad;

void setup() {
  Serial.begin(115200);
  gamepad.begin();
  Serial.println("Controller ready. Send 'test' to begin auto-cycling.");
}

void loop() {

  // -------------------
  // Loop through inputs
  // -------------------
  uint32_t buttons = 0;
  uint8_t hat = 0;  // default center
  String Hats = "";
  int ryVal = 0;
  int lyVal = 0;
  int rxVal = 0;
  int lxVal = 0;
  int l2Val = 0;
  int r2Val = 0;
  int currentMillis = millis();
  switch (state) {
    case 0:
      if (currentMillis - previousMillis < 1000) {
        gamepad.buttons(0);
      } else {
        nextState();
      }
      break;
    //-------------------
    //-----Buttons-------
    //-------------------
    case 1:
      // button A
      buttons |= (1 << 0);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 2:
      // button B
      buttons |= (1 << 1);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 3:
      // button X
      buttons |= (1 << 2);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 4:
      // button Y
      buttons |= (1 << 3);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 5:
      // button R1
      buttons |= (1 << 4);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 6:
      // button L1
      buttons |= (1 << 5);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 7:
      // button R3
      buttons |= (1 << 6);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 8:
      // button L3
      buttons |= (1 << 7);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 9:
      // start
      buttons |= (1 << 8);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 10:
      // select
      buttons |= (1 << 9);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 11:
      // home
      buttons |= (1 << 10);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 12:
      // button R2
      buttons |= (1 << 11);
      gamepad.buttons(buttons);
      nextState();
      break;
    case 13:
      // button L2
      buttons |= (1 << 12);
      gamepad.buttons(buttons);
      nextState();
      break;
    //------------------------
    //----Hat Switch Cycle----
    //------------------------
    case 14:
      // hat up
      hat = 1;
      Hats = "Up";
      nextState();
      break;
    case 15:
      // hat up - right
      hat = 2;
      Hats = "Up Right"
      nextState();
      break;
    case 16:
      // hat right
      hat = 3;
      Hats = "Right";
      nextState();
      break;
    case 17:
      // hat down- right
      hat = 4;
      Hats = "Down Right";
      nextState();
      break;
    case 18:
      // hat down
      hat = 5;
      Hats = "Down";
      nextState();
      break;
    case 19:
      // hat down left
      hat = 6;
      Hats = "Down Left";
      nextState();
      break;
    case 20:
      // hat left
      hat = 7;
      Hats = "Left";
      nextState();
      break;
    case 21:
      // hat up left
      hat = 8;
      Hats ="Up Left";
      nextState();
      break;
    case 22:
      joystickSweep(1);
      break;
    case 23:
      joystickSweep(2);
      break;
  }

  Serial.println("joystick position");
  Serial.printf("Left y: %d x: %d \nRight y: %d x: %d \n", lyVal, lxVal, ryVal, rxVal);
  Serial.printf("Button State A B X Y R1 L1 R2 L2 R3 L3 \n%d \n", buttons);
  Serial.printf("Hat position: %f \n", Hats);

  delay(polldelay);  // Small poll delay
}

void nextState() {
  if (state < 25 && previousMillis-millis() < 1000) {
    state++;
    previousMillis = millis();
  } else if(state < 25 && previousMillis-millis() >= 1000) {
    state = 0;
  } else{

  }
}

void sendgamepad(uint32_t button, int rxVa, int lxVa, int lyVa, int ryVa, int r2Va, int l2Va, uint8_t ha) {
  gamepad.sendAll(
    button,  // 32 buttons bitmask
    lxVa,    // LX
    lyVa,    // LY
    l2Va,    // L2 (treated as analog axis)
    rxVa,    // RX
    ryVa,    // RY
    r2Va,    // R2 (treated as analog axis)
    ha       // D-pad hat
  );
}

void joystickSweep(int stick) {
  unsigned long elapsed = millis() - previousMillis;
  while (elapsed < 6000) {
    elapsed = millis() - previousMillis;
    if (elapsed < 1000) {
      if (stick == 1) gamepad.joystick1(0, 0, 0);
      else gamepad.joystick2(0, 0, 0);
    } else if (elapsed < 2000) {
      if (stick == 1) gamepad.joystick1(127, 0, 0);
      else gamepad.joystick2(127, 0, 0);
    } else if (elapsed < 3000) {
      if (stick == 1) gamepad.joystick1(-127, 0, 0);
      else gamepad.joystick2(-127, 0, 0);
    } else if (elapsed < 4000) {
      if (stick == 1) gamepad.joystick1(0, 127, 0);
      else gamepad.joystick2(0, 127, 0);
    } else if (elapsed < 5000){
      if (stick == 1) gamepad.joystick1(0, -127, 0);
      else gamepad.joystick2(0, -127, 0);
    } else {
      if (stick == 1) gamepad.joystick1(0, 0, 0);
      else gamepad.joystick2(0, 0, 0);
      state++;
      previousMillis = millis();
    }
  }
}

#endif
