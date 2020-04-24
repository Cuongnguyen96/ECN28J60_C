#ifndef ARP_H_
#define ARP_H_
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
#include "net.h"
//-------------------------------------------------
//dinh nghia thoi gian xoa bang arp
#define MAX_TIME_SAVE 10 //(phut)
//-------------------------------------------------
#define COUNT_TICK (MAX_TIME_SAVE*60000)
//-------------------------------------------------


void ARP_read_packet(uint8_t *ARP_Buff,uint16_t len);
void ARP_send_request(uint8_t *ip_dest);
int8_t ARP_table_checkIP(uint8_t *ip_check);  //kiem tra xem co ton tai ip trong bang chua
void ARP_clear_table(void);
void ARP_table_get_MAC(uint8_t *ip_check,uint8_t * MAC_dest);

//--------------------------------------------------
#endif /* ARP_H_ */