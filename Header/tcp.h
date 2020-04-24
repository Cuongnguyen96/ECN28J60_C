#ifndef TCP_H_
#define TCP_H_
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
//--------------------------------------------------
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

    uint16_t Source_Port;//---------------|   //TCP
    uint16_t Dest_Port;  //               |
    uint32_t Sequence_Number;//           |
    uint32_t Acknowledgement;//           |
    uint8_t  data_offset;//               |
    uint8_t  TCP_Flags;//                 |
    uint16_t Window;//                    |
    uint16_t TCP_Checksums;//             |
    uint16_t Urgent_Pointer;//            |
    uint8_t  data[];//--------------------|
  }TCP_struct;
  //dinh nghia cac macro cho truong Flags
  #define TCP_CWR 0x80
  #define TCP_ECE 0x40
  #define TCP_URG 0x20
  #define TCP_ACK 0x10
  #define TCP_PSH 0x08
  #define TCP_RST 0x04
  #define TCP_SYN 0x02
  #define TCP_FIN 0x01
//--------------------------------------------------

void TCP_read(uint8_t *TCP_Frame,uint16_t len);
void TCP_send(TCP_struct *TCP_Struct_Frame,uint16_t len,uint8_t *data);
uint16_t TCP_checksum(TCP_struct *TCP_Struct_Frame);
//--------------------------------------------------
#endif /* TCP_H_ */