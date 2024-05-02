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


uint8_t rf_data_array[4];
uint8_t rf_tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};



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

    if (!nrf24_config(70,2))
    {
        send_UART_str_P(PSTR("No nRF24 found!\r\n"));
    }
    else
    {
        send_UART_str_P(PSTR("RF transmitter Online!\r\n"));

        /* Set the device addresses */
        nrf24_tx_address(rf_tx_address);
	}

	// Разрешаем прерывания
	sei();

	uint8_t buffer[16];
	int8_t IR_address_H, IR_address_L, IR_command_H, IR_command_L;

	while (1) {
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
			uint8_t IR_addr = IR_get_address();
			binToHex(IR_addr, buffer);
			send_UART_str(buffer);
			uint8_t IR_cmd = IR_get_command();
			binToHex(IR_cmd, buffer);
			send_UART_str(buffer);
			send_UART_str_P(PSTR("\r\n"));
            /* Fill the data buffer */
            rf_data_array[0] = IR_addr;
            rf_data_array[1] = IR_cmd;
            /* Automatically goes to TX mode */
            nrf24_send(rf_data_array);
            /* Wait for transmission to end */
            while(nrf24_isSending());
            /* Make analysis on last tranmission attempt */
            uint8_t rf_status = nrf24_lastMessageStatus();

            if(rf_status == NRF24_TRANSMISSON_OK)
            {                    
                send_UART_str_P(PSTR("Tranmission went OK\r\n"));
            }
            else if(rf_status == NRF24_MESSAGE_LOST)
            {                    
                send_UART_str_P(PSTR("Message is lost ...\r\n"));
            }
		}
	}
	return 0;
}