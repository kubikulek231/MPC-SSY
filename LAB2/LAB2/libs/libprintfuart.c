
/*
 * libprintfuart.c
 *
 * Created: 2/24/2025 12:24:51
 *  Author: Student
 */ 
#include "libprintfuart.h"
#include <avr/io.h>
#include <stdio.h>

void UART_init(uint16_t Baudrate) {
	uint16_t ubrr = (F_CPU / 16 / Baudrate) - 1;  // Calculate UBRR value
	UBRR1H = (uint8_t)(ubrr >> 8);  // Set baud rate high byte
	UBRR1L = (uint8_t)ubrr;         // Set baud rate low byte
	UCSR1B = (1 << RXEN1) | (1 << TXEN1);  // Enable RX and TX
}

void UART_SendChar(uint8_t data) {
	while (!(UCSR1A & (1 << UDRE1)));  // Wait for buffer to be empty
	UDR1 = data;  // Send data
}

uint8_t UART_GetChar(void) {
	while (!(UCSR1A & (1 << RXC1)));  // Wait for data to be received
	return UDR1;  // Return received data
}

void UART_SendString(char *text) {
    while (*text != 0x00) {  // Explicitly check for null terminator
        UART_SendChar(*text);
        text++;
    }
}

void UART_SendStringNewLine(char *text) {
    while (*text != 0x00) {  // Explicitly check for null terminator
        UART_SendChar(*text);
        text++;
    }
    UART_SendChar('\r');  // Carriage Return
    UART_SendChar('\n');  // New Line
    UART_SendChar(0x00);  // Explicitly send NULL character
}

// Function to send a colored string with a newline
void UART_SendStringNewLineColored(char *str, char *color_code) {
    // Send the color code first
    while (*color_code != 0x00) {
        UART_SendChar(*color_code);  // Send color code character by character
        color_code++;
    }
    
    // Now send the string
    UART_SendStringNewLine(str);  // Send the string with a newline
    
    // Reset the color to default
    while (*RESET_COLOR != 0x00) {
        UART_SendChar(*RESET_COLOR);  // Send reset color code character by character
    }
}

int printCHAR(char character, FILE *stream) {
	UART_SendChar(character);  // Send character via UART
	return 0;
}
