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
uint32_t buttons = 0;
uint8_t hat = 0;  // default center
String Hats = "";
String Button = "";
int ryVal = 0;
int lyVal = 0;
int rxVal = 0;
int lxVal = 0;
int l2Val = 0;
int r2Val = 0;

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
  buttons = 0;
  hat = 0;  // default center
  String Hats = "";
  String Button = "";
  ryVal = 0;
  lyVal = 0;
  rxVal = 0;
  lxVal = 0;
  l2Val = 0;
  r2Val = 0;
  int currentMillis = millis();
  switch (state) {
    // initial delay 1 sec
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
      // button B
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 0);
        gamepad.buttons(buttons);
        Button = "B";
      } else {
        nextState();
      }
      break;
    case 2:
      // button A
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 1);
        gamepad.buttons(buttons);
        Button = "A";
      } else {
        nextState();
      }
      break;
    case 3:
      // button Y
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 2);
        gamepad.buttons(buttons);
        Button = "Y";
      } else {
        nextState();
      }
      break;
    case 4:
      // button X
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 3);
        gamepad.buttons(buttons);
        Button = "X";
      } else {
        nextState();
      }
      break;
    case 5:
      // button L1
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 4);
        gamepad.buttons(buttons);
        Button = "L1";
      } else {
        nextState();
      }
      break;
    case 6:
      // button R1
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 5);
        gamepad.buttons(buttons);
        Button = "R1";
      } else {
        nextState();
      }
      break;
    case 7:
      // button L2
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 6);
        gamepad.buttons(buttons);
        Button = "L2";
      } else {
        nextState();
      }
      break;
    case 8:
      // button R2
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 7);
        gamepad.buttons(buttons);
        Button = "R2";
      } else {
        nextState();
      }
      break;
    case 9:
      // start
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 8);
        gamepad.buttons(buttons);
        Button = "Start";
      } else {
        nextState();
      }
      break;
    case 10:
      // select
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 9);
        gamepad.buttons(buttons);
        Button = "Select";
      } else {
        nextState();
      }
      break;
    case 11:
      // button L3
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 10);
        gamepad.buttons(buttons);
        Button = "L3";
      } else {
        nextState();
      }
      break;
    case 12:
      // button R3
      if (currentMillis - previousMillis < 1000) {
        buttons |= (1 << 11);
        gamepad.buttons(buttons);
        Button = "R3";
      } else {
        nextState();
      }
      break;
    //------------------------
    //----Hat Switch Cycle----
    //------------------------
    case 13:
      //Hat up
      hat = 1;
      buttons |= (1 << 12);
      Hats = "Up";
      gamepad.buttons(buttons);
      hatCycle(1, 14);
      break;
    case 14:
      // hat Down
      hat = 3;
      buttons |= (1 << 13);
      Hats = "Down";
      gamepad.buttons(buttons);
      hatCycle(3, 15);
      break;
    case 15:
      // hat left
      hat = 7;
      buttons |= (1 << 14);
      Hats = "Left";
      gamepad.buttons(buttons);
      hatCycle(7, 16);
      break;
    case 16:
      // hat right
      hat = 3;
      buttons |= (1 << 15);
      Hats = "Right";
      gamepad.buttons(buttons);
      hatCycle(3, 17);
      break;
    //-------------------
    //--Special buttons--
    //-------------------
    case 17:
      if (currentMillis - previousMillis < 1000) {
        // home
        buttons |= (1 << 16);
        Button = "Home";
        gamepad.buttons(buttons);
      } else {
        nextState();
      }
      break;
    case 18:
      if (currentMillis - previousMillis < 1000) {
        // Screenshot
        buttons |= (1 << 17);
        Button = "Screenshot";
        gamepad.buttons(buttons);
      } else {
        gamepad.buttons(0);
        nextState();
      }
      break;
    //-------------------
    //-----Joysticks-----
    //-------------------
    case 19:
      joystickSweep(1);
      break;
    case 20:
      joystickSweep(2);
      break;
    case 21:
      nextState();
      break;
  }

  Serial.println("joystick position");
  Serial.printf("Left y: %d x: %d \nRight y: %d x: %d \n", lyVal, lxVal, ryVal, rxVal);
  Serial.printf("Button State A B X Y R1 L1 R2 L2 R3 L3 \n%d \n", buttons);
  Serial.printf("Button Pressed: %f \n", Button);
  Serial.printf("Hat position: %d \n", hat);
  Serial.printf("Hat position: %f \n", Hats);

  delay(polldelay);  // Small poll delay
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

void nextState() {
  if (state < 20) {
    state++;
    previousMillis = millis();
  } else {
    previousMillis = millis();
    state = 0;
  }
}

void hatCycle(int direction, int next) {
  if (millis() - previousMillis < 1000) {
    gamepad.hat(direction);
  } else {
    gamepad.hat(0);
    buttons = 0;
    gamepad.buttons(0);
    state = next;
    previousMillis = millis();
  }
}


void joystickSweep(int stick) {
  unsigned long elapsed = millis() - previousMillis;
  if (elapsed < 5010) {
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
    } else if (elapsed < 5000) {
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
