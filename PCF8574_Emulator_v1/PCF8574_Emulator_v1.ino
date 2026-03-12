/*
PCF8574 Emulator for Nano esp32 S3
*/

#include <Wire.h>

#define I2C_ADDRESS 0x20   // Typical PCF8574 address

// ESP32 GPIO pins that emulate P0-P7
uint8_t gpioPins[8] = {13, 12, 14, 27, 26, 25, 33, 32};

volatile uint8_t portState = 0xFF;   // Default all HIGH (inputs)

/* ---------- Setup ---------- */
void setup()
{
    Wire.begin(I2C_ADDRESS);   // Start ESP32 as I2C slave

    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    for(int i = 0; i < 8; i++)
    {
        pinMode(gpioPins[i], INPUT_PULLUP);
    }
}

/* ---------- Loop ---------- */
void loop()
{
    // Continuously update pin states based on portState
    for(int i = 0; i < 8; i++)
    {
        if(portState & (1 << i))
        {
            pinMode(gpioPins[i], INPUT_PULLUP);
        }
        else
        {
            pinMode(gpioPins[i], OUTPUT);
            digitalWrite(gpioPins[i], LOW);
        }
    }
}

/* ---------- I2C Write Handler ---------- */
void receiveEvent(int bytes)
{
    if(bytes > 0)
    {
        portState = Wire.read();
    }
}

/* ---------- I2C Read Handler ---------- */
void requestEvent()
{
    uint8_t readState = 0;

    for(int i = 0; i < 8; i++)
    {
        if(portState & (1 << i))
        {
            // Pin configured as input
            if(digitalRead(gpioPins[i]))
                readState |= (1 << i);
        }
        else
        {
            // Output LOW
            // bit remains 0
        }
    }

    Wire.write(readState);
}