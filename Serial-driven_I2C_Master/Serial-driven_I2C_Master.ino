/*
   Raspberry Pi Pico: Serial-driven I2C Master
   ===========================================

   Communicates with ESP32 running PCAL6416A emulator (I2C slave).
   - Accepts commands from Serial (USB)
   - Executes I2C transactions
   - Prints results back over Serial

   Commands:
     R <reg>        Read 1 byte from register
     W <reg> <val>  Write 1 byte to register
     E <reg> <n>    Read n bytes starting from register (dump)
*/

#include <Wire.h>

#define PCAL6416A_ADDR 0x20
#define SDA_PIN 4
#define SCL_PIN 5

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Pico I2C Master Terminal (PCAL6416A emulator)");
  Serial.println("Commands:");
  Serial.println("  R <reg>");
  Serial.println("  W <reg> <val>");
  Serial.println("  E <reg> <n>");
  Serial.println("--------------------------");

  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin(); // Master mode
}

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(PCAL6416A_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(PCAL6416A_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false); // repeated start

  Wire.requestFrom(PCAL6416A_ADDR, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void readBlock(uint8_t reg, uint8_t count) {
  Wire.beginTransmission(PCAL6416A_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);

  Wire.requestFrom(PCAL6416A_ADDR, count);
  for (uint8_t i = 0; i < count; i++) {
    if (Wire.available()) {
      uint8_t val = Wire.read();
      Serial.print("Reg 0x");
      Serial.print(reg + i, HEX);
      Serial.print(" = 0x");
      Serial.println(val, HEX);
    }
    if (Wire.available()) {
      uint8_t val = Wire.read();
      Serial.print("Reg 0x");
      Serial.print(reg + i, HEX);
      Serial.print(" = 0x");
      Serial.println(val, HEX);
    }
  }
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() == 0) return;

    char type;
    int reg, val, n;
    if (sscanf(cmd.c_str(), "R %x", &reg) == 1) {
      // Single register read
      uint8_t data = readRegister((uint8_t)reg);
      Serial.print("Read 0x");
      Serial.print(reg, HEX);
      Serial.print(" = 0x");
      Serial.println(data, HEX);

    } else if (sscanf(cmd.c_str(), "W %x %x", &reg, &val) == 2) {
      // Single register write
      writeRegister((uint8_t)reg, (uint8_t)val);
      Serial.print("Write 0x");
      Serial.print(val, HEX);
      Serial.print(" -> Reg 0x");
      Serial.println(reg, HEX);

    } else if (sscanf(cmd.c_str(), "E %x %d", &reg, &n) == 2) {
      // Block read
      Serial.print("Dump ");
      Serial.print(n);
      Serial.print(" bytes from 0x");
      Serial.println(reg, HEX);
      readBlock((uint8_t)reg, (uint8_t)n);

    } else {
      Serial.println("Invalid command. Use R <reg>, W <reg> <val>, E <reg> <n>");
    }
  }
}
