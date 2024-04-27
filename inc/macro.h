#ifndef __MACRO_H__
#define __MACRO_H__

/** \brief Set Bit in I/O Register
 *  
 *  \param[in] port     I/O Port
 *  \param[in] bit      Bit position in port
 */
#define sbi(port,bit) ({ __asm__ __volatile__ ("sbi %0,%1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit));})

/** \brief Clear Bit in I/O Register
 *  
 *  \param[in] port     I/O Port
 *  \param[in] bit      Bit position in port
 */
#define cbi(port,bit) ({ __asm__ __volatile__ ("cbi %0,%1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit));})

#endif