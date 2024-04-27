/* SPI Hardware AVR Library
 *
 *
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include "macro.h"

#define SPI_MOSI_PORT PORTB
#define SPI_MOSI_DDR DDRB
#define SPI_MOSI PB3

#define SPI_MISO_PORT PORTB
#define SPI_MISO_DDR DDRB
#define SPI_MISO PB4

#define SPI_SCK_PORT PORTB
#define SPI_SCK_DDR DDRB
#define SPI_SCK PB5

#define SPI_SS_PORT PORTB
#define SPI_SS_DDR DDRB
#define SPI_SS PB2

void spi_init(void);
uint8_t spi_transfer(uint8_t data);

#endif