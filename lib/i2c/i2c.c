
#include "i2c.h"
#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define I2C_FREQ 100000UL

void i2cInit(void) {
    TWSR = 0;
    TWBR = (F_CPU / I2C_FREQ - 16) / 2;
    TWCR = (1 << TWEN);
}

void i2cStart(void) {
    TWCR = (1 << TWINT) |
           (1 << TWSTA) |
           (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

void i2cStop(void) {
    TWCR = (1 << TWINT) |
           (1 << TWSTO) |
           (1 << TWEN);

    while (TWCR & (1 << TWSTO));
}

void i2cWrite(uint8_t data) {
    TWDR = data;

    TWCR = (1 << TWINT) |
           (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));
}

uint8_t i2cRead(uint8_t ack) {
    TWCR = (1 << TWINT) |
           (1 << TWEN) |
           (ack ? (1 << TWEA) : 0);

    while (!(TWCR & (1 << TWINT)));

    return TWDR;
}
