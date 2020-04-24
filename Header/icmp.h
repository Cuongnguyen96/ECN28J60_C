#ifndef ICMP_H_
#define ICMP_H_
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
typedef struct
  {
    uint8_t MAC_dich[6];     //--------------|
    uint8_t MAC_nguon[6];    //              |   => It is Ethernet Frame II
    uint16_t Ethernet_type;  //--------------|

    uint8_t  Header_length;  //--------------|   => IP
    uint8_t  Services;       //              |
    uint16_t TotoLength;     //              |
    uint16_t Identification; //              |
    uint16_t Flag;           //              |
    uint8_t  TimeToLive;     //              |
    uint8_t  Protocol;       //              |
    uint16_t CheckSum;       //              |
    uint8_t  SourceIP[4];    //              |
    uint8_t  DestIP[4];      //--------------|

    uint8_t  ICMP_Type;     //---------------|   //ICMP
    uint8_t  ICMP_Code;     //               |
    uint16_t ICMP_checkSum; //               |
    uint16_t ICMP_Identifier;//              |
    uint16_t ICMP_Squenquer; //              |
    uint8_t *data;          //---------------|
  }ICMP_struct;

  void ICMP_read(uint8_t *ICMP_Frame,uint16_t len);
  void ICMP_ping_reply(ICMP_struct * ping_packet,uint16_t len);
  uint16_t ICMP_checksum(ICMP_struct *icmp_packet,uint16_t icmp_len );
//--------------------------------------------------
#endif /* ICMP_H_ */