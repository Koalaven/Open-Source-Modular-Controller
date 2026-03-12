/*
ADS1115 Emulator for Nano esp32 S3
*/

#include <Wire.h>

#define ADS1115_ADDR 0x48

#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG     0x01

uint16_t configRegister = 0x8583;
/*
Config breakdown:
OS      = 1 (start single conversion)
MUX     = 100 (AIN0 vs GND)
PGA     = 001 (±4.096V range)
MODE    = 1 (single-shot mode)
DR      = 100 (128 SPS)
COMP    = disabled
*/

void writeRegister(uint8_t reg, uint16_t value)
{
  Wire.beginTransmission(ADS1115_ADDR);
  Wire.write(reg);
  Wire.write((value >> 8) & 0xFF);   // MSB
  Wire.write(value & 0xFF);          // LSB
  Wire.endTransmission();
}

uint16_t readRegister(uint8_t reg)
{
  Wire.beginTransmission(ADS1115_ADDR);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(ADS1115_ADDR, 2);

  uint16_t value = (Wire.read() << 8) | Wire.read();
  return value;
}

int16_t readADS1115()
{
  writeRegister(ADS1115_REG_CONFIG, configRegister);

  delay(10); // wait for conversion

  return (int16_t)readRegister(ADS1115_REG_CONVERSION);
}

void setup()
{
  Serial.begin(115200);

  Wire.begin(21, 22); // SDA, SCL

  Serial.println("ADS1115 ADC Example");
}

void loop()
{
  int16_t raw = readADS1115();

  float voltage = raw * 0.000125; 
  // LSB size = 125uV for ±4.096V range

  Serial.print("Raw ADC: ");
  Serial.print(raw);

  Serial.print("  Voltage: ");
  Serial.print(voltage, 6);
  Serial.println(" V");

  delay(500);
}