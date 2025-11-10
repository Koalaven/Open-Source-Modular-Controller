#pragma once
#ifndef GAMEPAD_CONFIG_H
#define GAMEPAD_CONFIG_H

// Button Pins ABXY & LR
#define PIN_L1     9
#define PIN_L2     8

#define PIN_A      39
#define PIN_B      14
#define PIN_X      16
#define PIN_Y      17

#define PIN_R2     3
#define PIN_R1     18

// Joystick Pins (Analog Inputs)
#define PIN_Ly     11  // Left Y-axis
#define PIN_Ry     2   // Right Y-axis  
#define PIN_Lx     12  // Left X-axis
#define PIN_Rx     4   // Right X-axis

// Joystick Click Buttons
#define PIN_L3     10   // Left stick click
#define PIN_R3     40   // Right stick click

// Additional Analog Inputs
#define PIN_Lx1    13  // Secondary Left X
#define PIN_Rx1    1  // Secondary Right X

// D-Pad & Control Buttons
#define PIN_up     6
#define PIN_down   15
#define PIN_left   5
#define PIN_right  7
// control buttons
#define PIN_HOME   21
#define PIN_START  47
#define PIN_SELECT 48

// LED Indicators
#define PIN_LED1   35  // On Carrier board
#define PIN_LED2   36  
#define PIN_LED3   37
#define PIN_LED4   38

// Serial for Debug
#define PIN_TX     42
#define PIN_RX     41

#endif //end of file