#include <avr/io.h>
#include "spi.h"

void spi_init(void)
{
    sbi(SPI_MOSI_DDR,SPI_MOSI); // Set MOSI pin output
    cbi(SPI_MISO_DDR,SPI_MISO); // Set MISO pin input
    sbi(SPI_SCK_DDR,SPI_SCK); // Set SCK pin output
    // SS нужно настраивать на выход, иначе SPI модуль не будет считать себя мастером
    sbi(SPI_SS_DDR,SPI_SS); // Set SS pin output

    // MSTR = 1 - set as master
    // SPE = 1 - enable SPI
    // SPI mode 0: Clock Polarity CPOL = 0, Clock Phase CPHA = 0
    // Clock 2X speed: SPR0 = 0, SPR1 = 0
    // DORD = 0 - most significant first (MSB)
    SPCR = (1 << MSTR)|(1 << SPE)|(0 << CPOL)|(0 << CPHA)|(0 << SPR0)|(0 << SPR1)|(0 << DORD);

   // Clock 2X speed: SPI2X = 1
    SPSR = (1 << SPI2X);
}

uint8_t spi_transfer(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}
