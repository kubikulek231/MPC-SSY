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
#include "libs/i2clib/i2clib.h"
#include "libs/adclib/adclib.h"
#include "libs/tempsensor/tempsensor.h"

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
bool btn1_pressed = false;
bool btn2_pressed = false;
bool temperature_ready = false;

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

/************************************************************************/
/* WEIGHT SENSOR DEFINES								                */
/************************************************************************/

#define ADSK_PIN    PINF   // PF3 as clock (ADSK)
#define ADSK_DDR    DDRF
#define ADSK_PORT   PORTF
#define ADSK_BIT    PF3    // Clock signal on PF3

#define ADDO_PIN    PING   // PG5 as data (ADDO)
#define ADDO_DDR    DDRG
#define ADDO_PORT   PORTG
#define ADDO_BIT    PG5    // Data signal on PG5


void Hx711_init(void) {
	ADDO_DDR &= ~(1 << ADDO_BIT);  // Set ADDO as input
	ADDO_PORT |= (1 << ADDO_BIT);  // Enable pull-up (if needed)
	
	ADSK_DDR |= (1 << ADSK_BIT);   // Set ADSK as output
	ADSK_PORT &= ~(1 << ADSK_BIT); // Start ADSK low
}

unsigned long Hx711_read(void) {
	unsigned long Count = 0;
	uint8_t i;

	// Wait until ADDO goes low (indicates data ready)
	while (ADDO_PIN & (1 << ADDO_BIT));

	// Read 24-bit data
	for (i = 0; i < 24; i++) {
		ADSK_PORT |= (1 << ADSK_BIT);  // Set ADSK high
		Count = Count << 1;
		ADSK_PORT &= ~(1 << ADSK_BIT); // Set ADSK low

		if (ADDO_PIN & (1 << ADDO_BIT)) {
			Count++;
		}
	}

	// Send extra clock pulse to set device mode
	ADSK_PORT |= (1 << ADSK_BIT);
	Count ^= 0x800000; // Convert signed 24-bit value
	ADSK_PORT &= ~(1 << ADSK_BIT);

	return Count;
}

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

void Timer3_Temp_cmp_start(uint16_t porovnani) {
    cli();  // Disable global interrupts
    
    // Reset Timer3 configuration
    TCCR3A = 0;  // Set Timer3 to normal mode
    TCCR3B = 0;  // Reset Timer3 control register B
    TIMSK3 = 0;  // Disable interrupts for Timer3
    
    OCR3A = porovnani;  // Set the comparison value for Timer3
    TCCR3B |= (1 << WGM32);  // Set Timer3 to CTC mode (Clear Timer on Compare Match)
    TCCR3B |= 5;  // Set prescaler to 1024
    
    TIMSK3 |= (1 << OCIE3A);  // Enable interrupt for Timer3 compare match A
    
    sei();  // Enable global interrupts
}


ISR(TIMER3_COMPA_vect) {
    // Set a flag to indicate the timer has triggered
    temperature_ready = true;
}


void Timer2_fastpwm_start (uint8_t strida) {
	cli () ;
	TCCR2A = 0 ; // v y c i s t i t k o n t r o l n i r e g i s t r y
	TCCR2B = 0 ;
	TIMSK2 = 0 ; //
	// n a s t a v i t hodnotu pro PWM
	OCR2A = (255 * 1) / 100;
	// fastpwm mod:
	TCCR2A |= ( 1 << WGM21) ;
	TCCR2A |= ( 1 << WGM20) ;
	// 1024 p r e d d eli c k a :
	TCCR2B |= 5;
	TIMSK2 |= ( 1 << TOIE2 ) ;
	TCCR2A |= ( 1 << COM2A1) ;
	sei(); // p o v o l i t g l o b a l n i p r e r u s e n i
}

// NEZAPOMENTE PAK V PROGRAMU OSETRIT PRERUSENI
ISR (TIMER1_COMPA_vect)
{
	LED2CHANGE;
}
void Timer1Stop ( ) {
	TCCR1B=0;
}void Timer2Stop ( ) {
	TCCR2B=0;
}void TempStop ( ) {
	TCCR3B=0;
}
ISR(INT5_vect)
{
	btn1_pressed = true;  // Set flag when button is pressed (interrupt occurs)
}

ISR(INT6_vect)
{
	btn2_pressed = true;  // Set flag when button is pressed (interrupt occurs)
}

// Globalni pole
char abeceda[26];

// funkce pro generaci abecedym
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
	UART_SendStringNewLine("t ...... read temperature");
	UART_SendStringNewLine("d ...... read ADC values");
	UART_SendStringNewLine("e ...... read weight ADC values");
	UART_SendStringNewLine("f ...... calibrate weight reading");
	UART_SendStringNewLine("g ...... read calibrated values");
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

float get_weight_in_grams(unsigned long raw_reading, unsigned long tare_weight, float scale_factor) {
    return (raw_reading - tare_weight) / scale_factor;
}

int main(void) {
	uint16_t adc_value;

    // Initialize ADC with prescaler value and reference voltage (e.g., 64 and AVCC)
    ADC_Init(2, 1);  // prescaler 64, reference voltage AVCC

    // Get ADC value from channel 0 (e.g., analog pin A0)
    adc_value = ADC_get(3);

    // Print the result (you can use UART for output)
    printf("ADC Value: %u\n", adc_value);

    // Stop the ADC
    ADC_stop();
	
    UART_init(38400);  // Initialize UART with 9600 baud
	i2cInit();

    // Configure PE5 & PE6 as INPUT
    cbi(DDRE, PORTE5); // Button 1 (PE5)
    cbi(DDRE, PORTE6); // Button 2 (PE6)

    // Enable internal pull-ups (if no external resistors)
    sbi(PORTE, PORTE5);
    sbi(PORTE, PORTE6);

    // Configure external interrupts
    sbi(EICRB, ISC51); cbi(EICRB, ISC50); // INT5 -> Falling edge
    sbi(EICRB, ISC61); cbi(EICRB, ISC60); // INT6 -> Falling edge

    // Enable external interrupts
    sbi(EIMSK, INT5);  
    sbi(EIMSK, INT6);  

    sei(); // Enable global interrupts
	
	Hx711_init();
	UART_SendStringNewLine("Init done.");
	
	uint8_t test_sequence[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'U', 'A', 'R', 'T', '\r', '\n', 0 };

	for (uint8_t i = 0; test_sequence[i] != 0; i++) {
		UART_SendChar(test_sequence[i]);  // Send each character
	}
	
    // Set PB6 as output for LED control
	//sbi(DDRB, PORTB6);
	DDRB |= (1 << DDB6) | (1 << DDB5) | (1 << DDB6);  // Set PORTB pins 5 and 6 as output
    DDRE |= (1 << DDE3);  // Set PORTE pin 3 as output
	
	printMenu();
	
	char temp_str[20];
	unsigned long tmp_value = 0;
	unsigned long tare_weight = 1000;
	unsigned long standard_weight = 1000;
	unsigned long standard_weight_reading = 1000;
	float scale_factor = 1.0;
	
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
                UART_SendStringNewLineColored("Turning LED 1 off!", RED_TEXT);
				LED1OFF;
				break;
	        case '3':
                UART_SendStringNewLineColored("Turning LED 2 on!", GREEN_TEXT);
				LED2ON;
				break;
            case '4':
                UART_SendStringNewLineColored("Turning LED 2 off!", RED_TEXT);
				LED2OFF;
				break;
			case '5':
                UART_SendStringNewLineColored("Turning LED 3 on!", GREEN_TEXT);
				LED3ON;
				break;
            case '6':
                UART_SendStringNewLineColored("Turning LED 3 off!", RED_TEXT);
				LED3OFF;
				break;
			case '7':
				UART_SendStringNewLine("Entered button input mode:");
				uint8_t counter = 0;
				while(1) {
					if (btn1_pressed == true)
					{
						send_counter(counter);
						counter++;
						btn1_pressed = false;
					}
					if (btn2_pressed == true)
					{
						btn2_pressed = false;
						break;
					}
					if (counter >= 20) {
						UART_SendStringNewLineColored("Reached max counter num!", GREEN_TEXT);
						break;
					}
				}
				UART_SendStringNewLineColored("Exiting button input mode!", RED_TEXT);
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
				//Timer2_fastpwm_start(50);
				//break;
			case 't':
				UART_SendStringNewLine("Enabled temp mode:");
				uint16_t porovnani1 = (F_CPU / (1 * PRESCALE_VALUE * FREQ)) - 1;
				Timer3_Temp_cmp_start(porovnani1);
				while (1) {
					char received_temp = UART_GetCharNoWait();
						if (received_temp == 'c') {
							UART_SendStringNewLine("Exiting temperature reading mode...");
							break;  // Exit the temperature reading mode
						}
					if (temperature_ready) {
						temperature_ready = false;  // Reset the flag
            
						float temperature = readTemp();
            
						if (temperature != -1) {
							// If the temperature was read successfully, convert to an integer (multiply by 100)
							int temperatureInt = (int)(temperature * 100);
							char temp_str[20];
							sprintf(temp_str, "Temp: %d.%02d C", temperatureInt / 100, temperatureInt % 100);
							UART_SendStringNewLine(temp_str);
						} else {
							// Handle error if temperature reading failed
							UART_SendStringNewLine("Error reading temperature.");
						}
					}
				}
			case 'd':
				UART_SendStringNewLine("Reading ADC value:");
                ADC_Init(4, 2);
				uint16_t adc_value = ADC_get(3);
				char temp_str[20];
				sprintf(temp_str, "Value: %d", adc_value);
				UART_SendStringNewLine(temp_str);
				ADC_stop();
				break;
			case 'e':
				UART_SendStringNewLine("Reading weight ADC value:");
				while (1) {
					tmp_value = Hx711_read();
					_delay_ms(500);
					sprintf(temp_str, "Value: %lu", tmp_value);  // Use %lu for unsigned long
					UART_SendStringNewLine(temp_str);
				}
				break;
			case 'f':
				UART_SendStringNewLine("Calibrate tare weight");
				UART_SendStringNewLine("ESC ....... cancel");
				UART_SendStringNewLine("ENTER ..... confirm");
				while (1) {
					char received = UART_GetChar();
					if (received == '\x1B') {
						UART_SendStringNewLine("Not implemented LMAO");
					} else if (received == '\r') {
						tare_weight = Hx711_read();
						sprintf(temp_str, "Current tare value: %lu", tare_weight);
						UART_SendStringNewLine(temp_str);
						break;
					} else {
						UART_SendStringNewLine("Incorrect choice");
					}
				}
				UART_SendStringNewLine("Calibrate with standard weight");
				UART_SendStringNewLine("Enter standard weight weight in grams:");
				UART_SendStringNewLine("ESC ....... cancel");
				UART_SendStringNewLine("ENTER ..... confirm");
				UART_SendStringNewLine("+ ......... add weight");
				UART_SendStringNewLine("- ......... subtract weight");
				while (1) {
					sprintf(temp_str, "Current standard weight: %lu", standard_weight);
					UART_SendStringNewLine(temp_str);
					char received = UART_GetChar();
					if (received == '+') {
						standard_weight += 10;
					} else if (received == '-') {
						standard_weight -= 10;
					} else if (received == '\x1B') {
						UART_SendStringNewLine("Not implemented LMAO");
						break;
					}	else if (received == '\r') {
						sprintf(temp_str, "Final standard weight: %lu", standard_weight);
						UART_SendStringNewLine(temp_str);
						standard_weight_reading = Hx711_read();
						sprintf(temp_str, "Standard weight reading: %lu", standard_weight_reading);
						UART_SendStringNewLine(temp_str);
						break;
					}
				}
				UART_SendStringNewLine("CALIBRATION PROCESS FINISHED!!!");
				break;
			case 'g':
				UART_SendStringNewLine("Reading weight in g:");
				while (1) {
					tmp_value = Hx711_read();
					_delay_ms(500);
					float weight_grams = get_weight_in_grams(tmp_value, tare_weight, scale_factor);
					sprintf(temp_str, "Value: %d g", (unsigned int)weight_grams);
					UART_SendStringNewLine(temp_str);
				}
				break;
            default:
                UART_SendStringNewLine("Invalid input, please choose again.");
				break;
		}
	}
}