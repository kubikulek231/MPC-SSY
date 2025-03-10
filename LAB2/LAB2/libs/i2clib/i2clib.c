/*
 * adclib.c
 *
 * Created: 3/10/2025 12:13:12
 *  Author: Student
 */ 

#include "i2clib.h"

// Initialize I2C (TWI)
void i2cInit(void) {
	// Set SCL frequency to 400kHz (assuming 16MHz clock)
	TWSR = 0x00;  // Prescaler set to 1
	TWBR = 0x02;  // Bit rate register
	TWCR = (1 << TWEN);  // Enable TWI
}

// Send START condition
void i2cStart(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));  // Wait for completion
}

// Send STOP condition
void i2cStop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Write one byte to I2C
void i2cWrite(uint8_t data) {
	TWDR = data;  // Load data
	TWCR = (1 << TWINT) | (1 << TWEN);  // Start transmission
	while (!(TWCR & (1 << TWINT)));  // Wait for completion
}

// Read one byte and send ACK
uint8_t i2cReadACK(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));  // Wait for completion
	return TWDR;
}

// Read one byte and send NACK
uint8_t i2cReadNACK(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));  // Wait for completion
	return TWDR;
}

uint8_t i2cGetStatus(void) {
	uint8_t status;
	status = TWSR & 0xF8;  // Mask the prescaler bits
	return status;
}
