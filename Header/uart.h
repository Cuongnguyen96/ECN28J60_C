#ifndef UART_H_
#define UART_H_
//--------------------------------------------------
#define atmega328p
//#define atmega8
//--------------------------------------------------
#ifdef atmega328p
#include <mega328p.h>
#else
  #ifdef atmega8
   #include <mega8.h>
  #endif
#endif

#ifndef UDRE
#define UDRE 5
#endif

void UART_init(void);
void UART_putChar(unsigned char _data);
void UART_putString(unsigned char *u);

#endif /* UART_H_ */

