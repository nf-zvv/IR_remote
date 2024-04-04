#ifndef _UART_H_
#define _UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define baudrate 19200UL

#define HI(x) ((x)>>8)
#define LO(x) ((x)&0xFF)
#define bauddivider (F_CPU/(16UL*baudrate)-1)

void UART_Init(void);
void UART_get_str(unsigned char *s);
uint8_t UART_RX_Ready(void);
void send_UART(unsigned char c);
void send_UART_str(unsigned char *s);
void send_UART_str_P(const unsigned char *s);

#endif