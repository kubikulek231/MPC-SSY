/*
 * LAB1.c
 *
 * Created: 02.02.2020 9:01:38
 * Author : Ondra
 */ 

/************************************************************************/
/* INCLUDE                                                              */
/************************************************************************/
#include "libs/macros.h"
#include <avr/io.h>
#include <util/delay.h>
#include "libs/libprintfuart.h"
#include <stdio.h>

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

#define CONST 2  
#define ODECET  
#define UPPER_CASE 1
#define NORMAL_CASE 2
#define DIRECTION_UP 1
#define DIRECTION_DOWN 2


// F_CPU definovano primo v projektu!!! Debug->Properties->Toolchain->Symbols

/************************************************************************/
/* VARIABLES                                                            */
/************************************************************************/

int vysledek = 10;
unsigned char uch1 = 255;
unsigned char uch2 = 255;

//musime vytvorit soubor pro STDOUT
FILE uart_str = FDEV_SETUP_STREAM(printCHAR, NULL, _FDEV_SETUP_RW);

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void board_init();

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

// Globalni pole
char abeceda[26];

// funkce pro generaci abecedy
void generateField(int case_type) {
	int i;

	if (case_type == UPPER_CASE) {
		for (i = 0; i < 26; i++) {
			abeceda[i] = 'A' + i;
		}
		} else if (case_type == NORMAL_CASE) {
		for (i = 0; i < 26; i++) {
			abeceda[i] = 'a' + i;
		}
		} else {
		// spatna vstupni promenna	
		printf("PROGRAM ERROR\n");
		return;
	}
}

// funkce pro zmenu velikosti pismen
void capsLetters(int case_type) {
	int i;

	if (case_type == UPPER_CASE) {
		for (i = 0; i < 26; i++) {
			abeceda[i] = toupper(abeceda[i]); // zmen na velka
		}
		} else if (case_type == NORMAL_CASE) {
		for (i = 0; i < 26; i++) {
			abeceda[i] = tolower(abeceda[i]); // zmen na mala
		}
		} else {
		printf("PROGRAM ERROR\n");
		return;
	}
}

#include <stdio.h>

// Custom toupper function
char my_toupper(char c) {
	if (c >= 'a' && c <= 'z') {
		return c - 'a' + 'A';
	}
	return c;
}

// Custom tolower function
char my_tolower(char c) {
	if (c >= 'A' && c <= 'Z') {
		return c - 'A' + 'a';
	}
	return c;
}


// Funkce pro v?pis abecedy v po?adovan?m sm?ru
void printField(int direction) {
	int i;
	
	if (direction == DIRECTION_UP) {
		for (i = 0; i < 26; i++) {
			printf("%c ", abeceda[i]);
		}
		} else if (direction == DIRECTION_DOWN) {
		for (i = 25; i >= 0; i--) {
			printf("%c ", abeceda[i]);
		}
		} else {
		// spatny smer
		printf("PROGRAM ERROR\n");
	}
}


void board_init(){
	UART_init(38400); //nastaveni rychlosti UARTu, 38400b/s
	stdout = &uart_str; //presmerovani STDOUT
}

void printMenu() {
	UART_SendStringNewLine("Welcome to interactive terminal!");
	UART_SendStringNewLine("1 ...... turn on led 1");
	UART_SendStringNewLine("2 ...... turn off led 1");
	UART_SendStringNewLine("3 ...... turn on led 2");
	UART_SendStringNewLine("4 ...... turn off led 2");
	UART_SendStringNewLine("5 ...... turn on led 3");
	UART_SendStringNewLine("6 ...... turn off led 3");
	UART_SendStringNewLine("0 ...... clear");
}

void cleanConsole() {
	for (int i = 0; i < 30; i++) {
        UART_SendStringNewLine("");  // Send an empty string which is just a newline
    }
}

int main(void) {
    UART_init(38400);  // Initialize UART with 9600 baud
	uint8_t test_sequence[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'U', 'A', 'R', 'T', '\r', '\n', 0 };

	for (uint8_t i = 0; test_sequence[i] != 0; i++) {
		UART_SendChar(test_sequence[i]);  // Send each character
	}
	
	DDRB |= (1 << DDB5) | (1 << DDB6);  // Set PORTB pins 5 and 6 as output
    DDRE |= (1 << DDE3);  // Set PORTE pin 3 as output
	
	printMenu();
	while (1) {
		uint8_t received = UART_GetChar();  // Wait for input
		UART_SendStringNewLine("Your input is:");
		UART_SendChar(received);
		UART_SendChar('\r');
		UART_SendChar('\n');

		switch (received) {
            case '0':
				cleanConsole();
				printMenu();
                break;  // Exit the program or break the outer loop
            case '1':
                UART_SendStringNewLineColored("Turning LED 1 on!", GREEN_TEXT);
				LED1ON;
				break;
            case '2':
                UART_SendStringNewLine("Turning LED 1 off!");
				LED1OFF;
				break;
	        case '3':
                UART_SendStringNewLine("Turning LED 2 on!");
				LED2ON;
				break;
            case '4':
                UART_SendStringNewLine("Turning LED 2 off!");
				LED2OFF;
				break;
			case '5':
                UART_SendStringNewLine("Turning LED 3 on!");
				LED3ON;
				break;
            case '6':
                UART_SendStringNewLine("Turning LED 3 off!");
				LED3OFF;
				break;
            default:
                UART_SendStringNewLine("Invalid input, please choose again.");
				break;
		}
	}
}