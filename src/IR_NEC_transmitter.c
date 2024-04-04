#include "IR_NEC_transmitter.h"


void Freq_ON(void)
{
	TCCR2A = (1 << COM2A0)|(1 << WGM21);
	TCCR2B = (1 << FOC2A)|(1 << CS20);
	TCNT2 = 0;
	OCR2A = 104;
}

void Freq_OFF(void)
{
	TCCR2B = 0;
	TCCR2A = 0;
}

void IR_transmit_byte(uint8_t payload)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		if (payload & (1 << i))
		{
			// передаем "единицу"
			IR_LED_PORT |= (1 << IR_LED);
			Freq_ON();
			_delay_us(562);
			IR_LED_PORT &= ~(1 << IR_LED);
			Freq_OFF();
			_delay_us(3*562);
		} else {
			// передаем "ноль"
			IR_LED_PORT |= (1 << IR_LED);
			Freq_ON();
			_delay_us(562);
			IR_LED_PORT &= ~(1 << IR_LED);
			Freq_OFF();
			_delay_us(562);
		}
	}
}

void IR_transmit(uint8_t IR_address, uint8_t IR_command) {
	IR_LED_PORT |= (1 << IR_LED);
	Freq_ON();
	_delay_us(9000);
	IR_LED_PORT &= ~(1 << IR_LED);
	Freq_OFF();
	_delay_us(4500);
	IR_transmit_byte(IR_address);
	IR_transmit_byte(~IR_address);
	IR_transmit_byte(IR_command);
	IR_transmit_byte(~IR_command);
}

