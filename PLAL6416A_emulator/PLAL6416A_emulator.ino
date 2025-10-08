/*
   PCAL6416A Emulator for Pi Pico (Arduino version)
   ---------------------------------------------------
   - Emulates PCAL6416A 16-bit I/O expander
   - I2C slave at address 0x20
   - Inputs & registers controlled via Serial commands:
       I0 XX        -> set Input Port 0 (0x00) to hex value XX
       I1 XX        -> set Input Port 1 (0x01) to hex value XX
       REG AA XX    -> set register AA to value XX
       READ AA      -> read register AA
       SHOW         -> print key registers (inputs & outputs)
       DUMP         -> print all registers 0x00–0x4F
       HELP         -> show this command list
*/

#include <Wire.h>
#include <Arduino.h>

#define I2C_ADDR  0x20
#define INT_PIN   10   // INT output pin (active LOW)

static uint8_t regs[0x50];
static uint8_t pointer_reg = 0x00;
static bool pointer_valid = false;

static uint8_t prev_input0 = 0, prev_input1 = 0;
static uint8_t latched_input0 = 0, latched_input1 = 0;

// ===== Reset defaults =====
void init_registers() {
  memset(regs, 0, sizeof(regs));
  regs[0x02] = regs[0x03] = 0xFF;  // outputs default high
  regs[0x06] = regs[0x07] = 0xFF;  // all pins input
  regs[0x40] = regs[0x41] = regs[0x42] = regs[0x43] = 0xFF;
  regs[0x48] = regs[0x49] = 0xFF;
  regs[0x4A] = regs[0x4B] = 0xFF;  // all masked
  regs[0x4F] = 0x00;               // push-pull

  prev_input0 = regs[0x00];
  prev_input1 = regs[0x01];
  latched_input0 = latched_input1 = 0;

  digitalWrite(INT_PIN, HIGH); // release INT
}

static uint8_t next_reg(uint8_t reg) {
  switch (reg) {
    case 0x00: return 0x01; case 0x01: return 0x00;
    case 0x02: return 0x03; case 0x03: return 0x02;
    case 0x04: return 0x05; case 0x05: return 0x04;
    case 0x06: return 0x07; case 0x07: return 0x06;
    case 0x40: return 0x41; case 0x41: return 0x40;
    case 0x42: return 0x43; case 0x43: return 0x42;
    case 0x44: return 0x45; case 0x45: return 0x44;
    case 0x46: return 0x47; case 0x47: return 0x46;
    case 0x48: return 0x49; case 0x49: return 0x48;
    case 0x4A: return 0x4B; case 0x4B: return 0x4A;
    case 0x4C: return 0x4D; case 0x4D: return 0x4C;
    default:   return reg + 1;
  }
}

// ===== Interrupt update =====
static void update_interrupt() {
  uint8_t input0 = regs[0x00], input1 = regs[0x01];
  uint8_t mask0  = regs[0x4A], mask1  = regs[0x4B];
  uint8_t latch0 = regs[0x44], latch1 = regs[0x45];

  uint8_t changed0 = (input0 ^ prev_input0) & ~mask0;
  uint8_t changed1 = (input1 ^ prev_input1) & ~mask1;

  if (latch0) latched_input0 |= changed0; else latched_input0 = changed0;
  if (latch1) latched_input1 |= changed1; else latched_input1 = changed1;

  regs[0x4C] = latched_input0;
  regs[0x4D] = latched_input1;

  digitalWrite(INT_PIN, (latched_input0 || latched_input1) ? LOW : HIGH);
}

// ===== Clear interrupt on read =====
static void clear_interrupt_on_read(uint8_t reg) {
  if (reg == 0x00 || reg == 0x01) {
    prev_input0 = regs[0x00];
    prev_input1 = regs[0x01];
    latched_input0 = latched_input1 = 0;
    regs[0x4C] = regs[0x4D] = 0;
    digitalWrite(INT_PIN, HIGH);
  }
}

// ===== I2C callbacks =====
void onReceive(int howMany) {
  if (howMany <= 0) return;
  uint8_t first = Wire.read();
  howMany--;
  if (howMany == 0) { pointer_reg = first; pointer_valid = true; return; }
  pointer_reg = first; pointer_valid = true;
  while (howMany-- > 0) {
    uint8_t val = Wire.read();
    if (!(pointer_reg == 0x00 || pointer_reg == 0x01 ||
          pointer_reg == 0x4C || pointer_reg == 0x4D)) {
      regs[pointer_reg] = val;
    }
    pointer_reg = next_reg(pointer_reg);
  }
  update_interrupt();
}

void onRequest() {
  if (!pointer_valid) pointer_reg = 0;
  uint8_t val = regs[pointer_reg];
  Wire.write(val);
  //clear_interrupt_on_read(pointer_reg);
  pointer_reg = next_reg(pointer_reg);
}

// ===== Command list printer =====
void printHelp() {
  Serial.println("Available commands:");
  Serial.println("  I0 XX       - Set Input Port 0 (0x00) to hex value XX");
  Serial.println("  I1 XX       - Set Input Port 1 (0x01) to hex value XX");
  Serial.println("  REG AA XX   - Set register AA to hex value XX");
  Serial.println("  READ AA     - Read register AA");
  Serial.println("  SHOW        - Show inputs and outputs");
  Serial.println("  DUMP        - Dump all registers 0x00–0x4F");
  Serial.println("  HELP        - Show this list");
}

// ===== Serial command parser =====
void handleSerial() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.startsWith("I0")) {
      int val = strtol(line.substring(3).c_str(), NULL, 16);
      regs[0x00] = val & 0xFF;
      Serial.printf("Input Port 0 (0x00) set to 0x%02X\n", regs[0x00]);
      update_interrupt();
    } else if (line.startsWith("I1")) {
      int val = strtol(line.substring(3).c_str(), NULL, 16);
      regs[0x01] = val & 0xFF;
      Serial.printf("Input Port 1 (0x01) set to 0x%02X\n", regs[0x01]);
      update_interrupt();
    } else if (line.startsWith("REG")) {
      int addr, val;
      if (sscanf(line.c_str(), "REG %x %x", &addr, &val) == 2) {
        if (addr >= 0 && addr < sizeof(regs)) {
          regs[addr] = val & 0xFF;
          Serial.printf("Register 0x%02X set to 0x%02X\n", addr, regs[addr]);
          update_interrupt();
        } else {
          Serial.println("Error: Invalid register address");
        }
      } else {
        Serial.println("Usage: REG <addr> <val> (hex)");
      }
    } else if (line.startsWith("READ")) {
      int addr;
      if (sscanf(line.c_str(), "READ %x", &addr) == 1) {
        if (addr >= 0 && addr < sizeof(regs)) {
          Serial.printf("Register 0x%02X = 0x%02X\n", addr, regs[addr]);
        } else {
          Serial.println("Error: Invalid register address");
        }
      } else {
        Serial.println("Usage: READ <addr> (hex)");
      }
    } else if (line.startsWith("SHOW")) {
      Serial.printf("Inputs: P0=0x%02X P1=0x%02X | Outputs: P0=0x%02X P1=0x%02X\n",
                    regs[0x00], regs[0x01], regs[0x02], regs[0x03]);
    } else if (line.startsWith("DUMP")) {
      Serial.println("Register dump:");
      for (int i = 0; i < 0x50; i++) {
        Serial.printf("0x%02X: 0x%02X\t", i, regs[i]);
        if ((i % 8) == 7) Serial.println();
      }
    } else if (line.startsWith("HELP")) {
      printHelp();
    } else {
      Serial.println("Unknown command. Type HELP for list of commands.");
    }
  }
}

// ===== Setup / Loop =====
void setup() {
  pinMode(INT_PIN, OUTPUT);
  digitalWrite(INT_PIN, HIGH);
  init_registers();

  Wire.begin(I2C_ADDR);   // join I2C bus as slave
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);

  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Pico PCAL6416A Emulator started (Serial inputs)");
  printHelp();
}

void loop() {
  handleSerial();
}
