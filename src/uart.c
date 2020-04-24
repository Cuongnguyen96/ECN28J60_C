#include "uart.h"

void UART_init(void)
{
  PORTB.0=PORTB.1=1; //pull up
  DDRD.0=0;          //RX in
  DDRD.1=1;          //TX out

  //9600 cry 16
  UCSR0A=0x00;
  UCSR0B=0x98;
  UCSR0C=0x06;
  UBRR0H=0x00;
  UBRR0L=0x67;
}
#ifdef atmega328p
void UART_putChar(unsigned char _data)
{
  while(!(UCSR0A & (1<<UDRE))); //kiem tra UDRE
  UDR0=_data;
}
void UART_putString(unsigned char *u)
{
  while(*u)
  {
    UART_putChar(*u);
    u++;
  }
}
#else
  #ifdef atmega8
       void UART_putChar(unsigned char _data)
        {
          while(!(UCSRA & (1<<UDRE))); //kiem tra UDRE
          UDR=_data;
        }
        void UART_putString(unsigned char *u)
        {
          while(*u)
          {
            UART_putChar(*u);
            u++;
          }
        }
  #endif
#endif

//--------------------------------------------END------------------------------------//


