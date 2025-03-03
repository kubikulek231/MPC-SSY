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
#include <avr/interrupt.h>
#include "libs/libprintfuart.h"
#include <stdio.h>
#include <stdbool.h>

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
bool btn_pressed = false;

//musime vytvorit soubor pro STDOUT
FILE uart_str = FDEV_SETUP_STREAM(printCHAR, NULL, _FDEV_SETUP_RW);

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void board_init();

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

#define PRESCALE_VALUE 1024
#define PRESCALE 5
#define FREQ 2

void Timer1_cmp_start(uint16_t porovnani) {
	cli();
	TCCR1A = 0 ;
	TCCR1B = 0 ;
	TIMSK1 = 0 ;
	
	OCR1A = porovnani ;
	// CTC mod :
	TCCR1B |= ( 1 << WGM12) ;
	// 1024 pred delicka:
	//TCCR1B |= ( 1 << CS10 );
	//TCCR1B |= ( 1 << CS12 );
	TCCR1B |= 5;
	// p o v o l i t p r e r u s e ni , pokud budete POTREBOVAT:
	TIMSK1 |= ( 1 << OCIE1A);
	// vystup na pin OC1A, t o g g l e
	TCCR1A |= ( 1 << COM1A0);
	sei();
}

void Timer2_fastpwm_start (uint8_t strida) {
	cli () ;
	TCCR2A = 0 ; // v y c i s t i t k o n t r o l n i r e g i s t r y
	TCCR2B = 0 ;
	TIMSK2 = 0 ; //
	// n a s t a v i t hodnotu pro PWM
	OCR2A = (255 * strida) / 100;
	// fastpwm mod:
	TCCR2A |= ( 1 << WGM21) ;
	TCCR2A |= ( 1 << WGM20) ;
	// 1024 p r e d d eli c k a :
	TCCR2B |= 5 ;
	TIMSK2 |= ( 1 << TOIE2 ) ;
	TCCR2A |= ( 1 << COM2A1) ;
	sei(); // p o v o l i t g l o b a l n i p r e r u s e n i
}

// NEZAPOMENTE PAK V PROGRAMU OSETRIT PRERUSENI
ISR (TIMER1_COMPA_vect)
{
	LED1CHANGE;
}void Timer0_ovf_start ( ) {
	cli();
	TCCR0A = 0;
	TCCR0B = 0;
	TIMSK0 = 0; 
	// Nastav_timer ( mil s ) ;
	// nastavit pocatecni hodnotu
	// 1024 pred delicka:
	TCCR0B |= PRESCALE;
	// vystup na pin OC0A, t o g g l e
	TCCR0A |= ( 1 << COM0A0) ;
	TIMSK0 |= ( 1 << TOIE0 ) ;
	sei();
}
void Timer1Stop ( ) {
	TCCR1B=0;
}void Timer2Stop ( ) {
	TCCR2B=0;
}
ISR(INT5_vect)
{
	btn_pressed = true;  // Set flag when button is pressed (interrupt occurs)
}

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
	UART_SendStringNewLine("7 ...... enter button input mode");
	UART_SendStringNewLine("8 ...... turn on led blinking");
	UART_SendStringNewLine("9 ...... turn off led blinking");
	UART_SendStringNewLine("a ...... turn on PWM blinking");
	UART_SendStringNewLine("b ...... turn OFF PWM blinking");
	UART_SendStringNewLine("0 ...... clear");
}

void cleanConsole() {
	for (int i = 0; i < 30; i++) {
        UART_SendStringNewLine("");  // Send an empty string which is just a newline
    }
}

void send_counter(int counter)
{
	char buf[10]; // Buffer to hold the counter value as a string
	itoa(counter, buf, 10);  // Convert counter to string in base 10
	UART_SendString("Button pressed count: ");
	UART_SendString(buf);  // Send the string (counter value) over UART
	UART_SendString("\r\n");
}

int main(void) {
    UART_init(38400);  // Initialize UART with 9600 baud
	
	// Set PB6 as output for LED control
	sbi(DDRB, PORTB6);
		
	cbi(DDRE, PORTE5); // Set PE5 as input
	
	sbi(EICRB, ISC51); // Falling edge
	cbi(EICRB, ISC50);
	sbi(EIMSK, INT5); // Enable INT5
	
	
	sei(); // Enable global interrupts
		
	uint8_t test_sequence[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'U', 'A', 'R', 'T', '\r', '\n', 0 };

	for (uint8_t i = 0; test_sequence[i] != 0; i++) {
		UART_SendChar(test_sequence[i]);  // Send each character
	}
	
	DDRB |= (1 << DDB5) | (1 << DDB6);  // Set PORTB pins 5 and 6 as output
    DDRE |= (1 << DDE3);  // Set PORTE pin 3 as output
	
	printMenu();
	while (1) {
		char received = UART_GetChar();  // Wait for input
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
			case '7':
				UART_SendStringNewLine("Entered button input mode:");
				uint8_t counter = 0;
				while(1) {
					if (btn_pressed == true)
					{
						send_counter(counter);
						counter++;
						btn_pressed = false;
					}
				}
				break;
			case '8':
				UART_SendStringNewLine("Turned on led blinking!");
				uint16_t porovnani = (F_CPU / (2 * PRESCALE_VALUE * FREQ)) - 1;
				Timer1_cmp_start(porovnani);
				break;
			case '9':
				UART_SendStringNewLine("Turned off led blinking!");
				Timer1Stop();
				break;
			case 'a':
				UART_SendStringNewLine("Turned on PWM led blinking!");
				//uint16_t porovnani = (F_CPU / (2 * PRESCALE_VALUE * FREQ)) - 1;
				//Timer1_cmp_start(porovnani);
				break;
			case 'b':
				UART_SendStringNewLine("Turned off PWM led blinking!");
				Timer2_fastpwm_start(50);
				break;
            default:
                UART_SendStringNewLine("Invalid input, please choose again.");
				break;
		}
	}
}