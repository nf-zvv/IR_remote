#include <avr/io.h>
#include "macro.h"

// Modify these variables to customize the ports/pins the RF module will use
#define NRF24_CE_PORT PORTC
#define NRF24_CE_DDR DDRC
#define NRF24_CE PC0

#define NRF24_CSN_PORT PORTC
#define NRF24_CSN_DDR DDRC
#define NRF24_CSN PC1

//#define NRF24_IRQ_PORT PORTD
//#define NRF24_IRQ_DDR DDRD
//#define NRF24_IRQ PD0

//#define LOW 0
//#define HIGH 1

/** nrf24 CE pin control function
 *
 *  \param[in] state      State of pin: HIGH or LOW
 */
#define nrf24_ce(x) (((x) == 0) ? (cbi(NRF24_CE_PORT,NRF24_CE)) : (sbi(NRF24_CE_PORT,NRF24_CE)))

/** nrf24 CSN pin control function
 *
 *  \param[in] state      State of pin: HIGH or LOW
 */
#define nrf24_csn(x) (((x) == 0) ? (cbi(NRF24_CSN_PORT,NRF24_CSN)) : (sbi(NRF24_CSN_PORT,NRF24_CSN)))
