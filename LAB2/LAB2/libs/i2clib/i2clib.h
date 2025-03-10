/*
 * adclib.h
 *
 * Created: 3/10/2025 12:13:28
 *  Author: Student
 */ 

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>  // for uint8_t
#include <avr/io.h>
#include <util/twi.h>  // TWI status codes (optional)

// Function Prototypes
void i2cInit(void);           // Initialize I2C with 400kHz SCL
void i2cStart(void);          // Generate I2C Start condition
void i2cStop(void);           // Generate I2C Stop condition
void i2cWrite(uint8_t u8data); // Write a byte to I2C
uint8_t i2cReadACK(void);     // Read a byte from I2C with ACK
uint8_t i2cReadNACK(void);    // Read a byte from I2C with NACK
uint8_t i2cGetStatus(void);   // Get the status of the I2C operation

#endif /* I2C_H */
