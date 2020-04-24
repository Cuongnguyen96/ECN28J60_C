#ifndef NET_H_
#define NET_H_
//--------------------------------------------------
//Include cac thu vien can thiet
#include <mega328p.h>
#include <delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <spi.h>
#include "uart.h"
#include "enc28j60.h"
#include "apr.h"
//-------------------------------------------------
//Because type 16 bit and 32 bit in struct is type litte endian
#define swap16(a) ((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define swap32(a) ((((a)>>24)&0xff)|(((a)>>8)&0xff00)|(((a)<<8)&0xff0000)|(((a)<<24)&0xff000000))
//-------------------------------------------------
#define BUFFER_LENGTH 512

void NET_SendFrame(uint8_t *EthFrame,uint16_t len);
void ARP_read_packet(uint8_t *ARP_Buff,uint16_t len)
void NET_read(uint8_t *net_buffer,uint16_t len);
uint16_t NET_ipchecksum(uint8_t *IP_packet_start);
void NET_loop(void);

//--------------------------------------------------
#endif /* ENC28J60_H_ */
