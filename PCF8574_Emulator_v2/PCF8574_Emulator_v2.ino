/*
PCF8574 Emulator for Nano esp32 S3
*/

#include <Wire.h>

#define I2C_ADDRESS 0x20
#define INT_PIN 4

uint8_t gpioPins[8] = {13,12,14,27,26,25,33,32};

volatile uint8_t latchState = 0xFF;
volatile uint8_t lastInputState = 0xFF;

/* ---------- Setup ---------- */

void setup()
{
    Wire.begin(I2C_ADDRESS);

    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(INT_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(INT_PIN, HIGH);

    for(int i=0;i<8;i++)
        pinMode(gpioPins[i], INPUT_PULLUP);
}

/* ---------- Loop ---------- */

void loop()
{
    uint8_t currentState = readPins();

    if(currentState != lastInputState)
    {
        digitalWrite(INT_PIN, LOW);   // trigger interrupt
        lastInputState = currentState;
    }

    applyLatch();
}

/* ---------- Apply Latch ---------- */

void applyLatch()
{
    for(int i=0;i<8;i++)
    {
        if(latchState & (1<<i))
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

/* ---------- Read Pins ---------- */

uint8_t readPins()
{
    uint8_t val = 0;

    for(int i=0;i<8;i++)
    {
        if(digitalRead(gpioPins[i]))
            val |= (1<<i);
    }

    return val;
}

/* ---------- I2C Write ---------- */

void receiveEvent(int len)
{
    if(len)
    {
        latchState = Wire.read();
        applyLatch();
    }
}

/* ---------- I2C Read ---------- */

void requestEvent()
{
    uint8_t state = readPins();

    Wire.write(state);

    digitalWrite(INT_PIN, HIGH); // clear interrupt
}