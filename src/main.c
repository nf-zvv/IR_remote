#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include "uart.h"
#include "IR_NEC_transmitter.h"
#include "IR_NEC_receiver.h"
#include "spi.h"
#include "nrf24.h"


#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED PB5


#define num_channels 126

uint8_t values[num_channels];
const int num_reps = 100;

// Convert one digit to ascii hex
unsigned char toHex(unsigned char digit)
{
    if (digit > 9) digit += 0x27; // adjust 0x0a-0x0f to come out 'a' - 'f'
    digit += 0x30;                // to ascii
    return digit;
}

uint8_t min(uint8_t a, uint8_t b)
{
    if (a < b) 
        return a;
    else 
        return b;
}

void scan(void)
{
    nrf24_ce(LOW);
    // Clear measurement values
    for (uint8_t i = 0; i < num_channels; i++)
        values[i] = 0;

    // Scan all channels num_reps times
    int rep_counter = num_reps;
    while (rep_counter--)
    {
        int i = num_channels;
        while (i--)
        {
            // Set RF channel i
            nrf24_configRegister(RF_CH,i);

            // Listen for a little
            nrf24_powerUpRx();
            _delay_us(150);
            nrf24_powerUpTx();

            // Did we get a carrier?
            if ( nrf24_isCarrierDetected() ) {
                ++values[i];
            }
        }
    }

    // Print out channel measurements, clamped to a single hex digit
    int i = 0;
    while ( i < num_channels )
    {
      if (values[i])
        send_UART(toHex(min(0x0f,values[i])));
      else
        send_UART(45); // dash
      ++i;
    }
    send_UART_str_P(PSTR("\r\n"));
}

int8_t hexCharToBin(char c)
{
	if (isdigit(c))  // '0'...'9'
		return c - '0';
	else if (isxdigit(c))  // A-F, a-f
		return (c & 0x0F) + 9;
	return -1;
}

unsigned char binToHexChar(uint8_t bin)
{
	if (bin < 0x0a)
		return bin + 0x30;
	else
		return bin + 0x37;
}

void binToHex(uint8_t bin, unsigned char *s)
{
	uint8_t hi_nibble = (bin & 0xF0) >> 4;
	*s++ = binToHexChar(hi_nibble);
	uint8_t lo_nibble = bin & 0x0F;
	*s++ = binToHexChar(lo_nibble);
	*s = '\0';
}

int main(void)
{
	IR_LED_DDR |= 1 << IR_LED;
	LED_DDR |= 1 << LED;

	DDRD &= ~(1 << PD3); // input for IR receiver
	PORTD |= (1 << PD3); // turn on pull-up resistor

	DDRD |= (1 << PD1); // TXD output
	DDRD &= ~(1 << PD0); // RXD input
	PORTD |= (1 << PD0); // TXD pull up

	// Инициализация UART
	UART_Init();

	// Запускаем ИК прослушивание
	IR_listen_start();

	/* init hardware pins */
    spi_init();
    nrf24_init();

    if (!nrf24_config(70,1))
    {
        send_UART_str_P(PSTR("No nRF24 found!\r\n"));
    }
    else
    {
        send_UART_str_P(PSTR("RF scanner Online!\r\n"));

        // Print header
        int i = 0;
        // First line - high digit
        while ( i < num_channels )
        {
            send_UART(toHex(i >> 4));
            ++i;
        }
        send_UART_str_P(PSTR("\r\n"));
        i = 0;
        // Second line - low digit
        while ( i < num_channels )
        {
            send_UART(toHex(i & 0x0f));
            ++i;
        }
        send_UART_str_P(PSTR("\r\n"));
	}

	// Разрешаем прерывания
	sei();

	uint8_t buffer[16];
	int8_t IR_address_H, IR_address_L, IR_command_H, IR_command_L;

	while (1) {
		scan();
		if (UART_RX_Ready())
		{
			// Выключить ожидание приема данных по IR
			IR_listen_stop();

			// Взять из буфера адрес и команду для отправки по IR
			UART_get_str(buffer);

			// Подготовить данные
			IR_address_H = hexCharToBin(buffer[0]);
			IR_address_L = hexCharToBin(buffer[1]);
			IR_command_H = hexCharToBin(buffer[2]);
			IR_command_L = hexCharToBin(buffer[3]);

			// Проверить данные на пригодность
			if ((IR_address_H < 0) || (IR_address_L < 0) || (IR_command_H < 0) || (IR_command_L < 0))
				continue;

			// Послать данные
			IR_transmit(IR_address_H << 4 | IR_address_L, IR_command_H << 4 | IR_command_L);

			// Включить ожидание приема данных по IR
			IR_listen_start();
		}
		if (IR_available())
		{
			uint8_t addr = IR_get_address();
			binToHex(addr, buffer);
			send_UART_str(buffer);
			uint8_t cmd = IR_get_command();
			binToHex(cmd, buffer);
			send_UART_str(buffer);
			send_UART_str_P(PSTR("\r\n"));
		}
	}
	return 0;
}