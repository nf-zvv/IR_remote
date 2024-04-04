#include <avr/io.h>
#include <util/delay.h>


#define IR_LED_PORT PORTB
#define IR_LED_DDR DDRB
#define IR_LED PB3


/* Prototypes */
//void Freq_ON(void);
//void Freq_OFF(void);
void IR_transmit(uint8_t IR_address, uint8_t IR_command);
