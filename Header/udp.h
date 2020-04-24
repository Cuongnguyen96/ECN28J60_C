#ifndef UDP_H_
#define UDP_H_
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

    uint16_t UDP_Source_Port;//---------------|   //UDP
    uint16_t UDP_Dest_Port;  //               |
    uint16_t UDP_Length;     //               |
    uint16_t UDP_Checksum;   //               |
    uint8_t data[];          //---------------|
  }UDP_struct;

#define ENC28J60_UDP_PORT 20798
  
uint8_t UDP_send(uint8_t *IP_dest,uint16_t port_dest,uint8_t *data,uint32_t timeout);
uint16_t NET_ipchecksum(uint8_t *IP_packet_start);

//--------------------------------------------------
#endif /* UDP_H_ */