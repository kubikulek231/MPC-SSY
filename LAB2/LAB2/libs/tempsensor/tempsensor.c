/*
 * tempsensor.c
 *
 * Created: 3/10/2025 12:28:40
 *  Author: Student
 */ 

#include "tempsensor.h"

uint8_t setPrecision(uint8_t prec) {
	uint16_t config_register = 0;

	// Set precision in the config register
	config_register |= (uint16_t)(prec << R0);

	// Start I2C communication
	i2cStart();

	// Write the sensor address
	i2cWrite(TempSensorAddrW);
	if (i2cGetStatus() != 0x18) {
		UART_SendString("Error 18\n\r");
		return 0;
	}

	// Write the config register address
	i2cWrite(Temp_configRegister);
	if (i2cGetStatus() != 0x28) {
		UART_SendString("Error 28\n\r");
		return 0;
	}

	// Send the high byte of the config register
	i2cWrite((uint8_t)(config_register >> 8));
	if (i2cGetStatus() != 0x28) {
		UART_SendString("Error 28\n\r");
		return 0;
	}

	// Send the low byte of the config register
	i2cWrite((uint8_t)(config_register));
	if (i2cGetStatus() != 0x28) {
		UART_SendString("Error 28\n\r");
		return 0;
	}

	// Stop I2C communication
	i2cStop();

	return 1;  // Success
}

float readTemp(void) {
	volatile uint8_t buffer[2];     // Buffer to store the temperature bytes
	volatile int16_t temperatureTMP; // To store the combined temperature value

	// Start I2C communication
	i2cStart();
	i2cWrite(TempSensorAddrW); // Write the sensor address (write mode)
	if (i2cGetStatus() != 0x18) {
		UART_SendString("Error 18\n\r");
		return -1;  // Return error code
	}

	// Write the temperature register address
	i2cWrite(Temp_tempRegister);
	if (i2cGetStatus() != 0x28) {
		UART_SendString("Error 28\n\r");
		return -1;  // Return error code
	}
	i2cStop();  // Stop after sending the register address

	// Start I2C communication again for reading
	i2cStart();
	if (i2cGetStatus() != 0x08) {
		UART_SendString("Error 08\n\r");
		return -1;  // Return error code
	}

	// Read data from the temperature sensor
	i2cWrite(TempSensorAddrR); // Write the sensor address (read mode)
	if (i2cGetStatus() != 0x40) {
		UART_SendString("Error 40\n\r");
		return -1;  // Return error code
	}

	buffer[0] = i2cReadACK(); // Read first byte with ACK
	if (i2cGetStatus() != 0x50) {
		UART_SendString("Error 50\n\r");
		return -1;  // Return error code
	}

	buffer[1] = i2cReadNACK(); // Read second byte with NACK
	if (i2cGetStatus() != 0x58) {
		UART_SendString("Error 58\n\r");
		return -1;  // Return error code
	}

	i2cStop();  // Stop I2C communication

	// Combine the two bytes into a 16-bit value (temperature value)
	temperatureTMP = (buffer[0] << 8) | buffer[1];

	// Return the temperature in degrees Celsius by dividing by 256 (resolution of sensor)
	float floatTMP = (float)temperatureTMP / 256.0f; 
	return floatTMP;
}
