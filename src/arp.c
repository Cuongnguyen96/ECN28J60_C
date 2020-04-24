/* ARP:: Address Resoulution Protocol */

#include "arp.h"


extern const uint8_t macaddr[6];
extern const uint8_t ip[4];
extern uint8_t debug_string[60];

struct
{
   uint8_t ip[4];
   uint8_t mac[6];
}ARP_table[5];
uint8_t ARP_table_index=0;

/*
*	1. Check ARP is type ARP Requset or ARP Reponse with method check Opcode bit 20, 21
*	2. If ARP request then repair packet response
*	3. If ARP response, check target MAC if true save MAC of response into table
*/
void ARP_read_packet(uint8_t *ARP_Buff,uint16_t len)
{
  struct
  {
    uint8_t MAC_dich[6];                    // MAC dich
    uint8_t MAC_nguon[6];                   //MAC nguon
    uint16_t Ethernet_type;                 //ethernet type
    uint16_t Hardwave_type;                 //hardwave type
    uint16_t Protocol_type;                 //protocol type (ARP)
    uint16_t Size;                          //size
    uint16_t Opcode;                        //opcode
    uint8_t MAC_sender[6];                  //sender MAC
    uint8_t IP_sender[4];                   //sender IP
    uint8_t MAC_target[6];                  // Target MAC
    uint8_t IP_target[4];                   // Target IP
  }ARP_reponse;

  //tao 2 array de luu IP và MAC cua nguon
  uint8_t MAC_source[6];
  uint8_t IP_source[4];
  uint8_t IP_target[4];

  memcpy(MAC_source,(uint8_t *)&ARP_Buff[6],6);  //lay MAC cua nguon gui packet
  memcpy(IP_source ,(uint8_t *)&ARP_Buff[28],4); //lay IP cua nguon gui packet
  memcpy(IP_target ,(uint8_t *)&ARP_Buff[38],4); //lay IP cua nguon nhan packet

  if(ARP_Buff[20] == 0x00 && ARP_Buff[21] == 0x01) //neu Opcode = 0x0001 thi day la ban tin ARP request
  {
     //kiem tra xem co trung voi IP cua minh khong
     if(IP_target[0] != ip[0] ||  IP_target[1] != ip[1] || IP_target[2] != ip[2] || IP_target[3] != ip[3] ) return;
     UART_putString("Da nhan 1 arp request\r\n");
     //tao goi tin ARP reponse
     memcpy(ARP_reponse.MAC_dich,MAC_source,6);   //dia chi MAC cua thang nhan goi tin reponse
     memcpy(ARP_reponse.MAC_target,MAC_source,6);
     memcpy(ARP_reponse.IP_target,IP_source,4);   //dia chi IP cua thang nhan goi tin reponse
     memcpy(ARP_reponse.MAC_nguon,macaddr,6);    //dia chi MAC cua thang gui ( chinh la MAC cua ENC28J60)
     memcpy(ARP_reponse.MAC_sender,macaddr,6);
     memcpy(ARP_reponse.IP_sender,ip,4);   //dia chi IP cua thang gui goi tin reponse ( chinh la MAC cua ENC28J60)
     ARP_reponse.Ethernet_type=0x0608;
     ARP_reponse.Hardwave_type=0x0100;
     ARP_reponse.Protocol_type=0x0008;
     ARP_reponse.Size=0x0406;
     ARP_reponse.Opcode=0x0200;
     UART_putString("Gui gon tin reply\r\n");
     NET_SendFrame((uint8_t *)&ARP_reponse,42);   //gui goi ARP reponse di
  }
  else if(ARP_Buff[20] == 0x00 && ARP_Buff[21] == 0x02) //neu Opcode = 0x0001 thi day la ban tin ARP reponse
  {
       UART_putString("Da nhan 1 arp reponse\r\n");
       ARP_table_setIP(IP_source,MAC_source);
  }
}

void ARP_send_request(uint8_t *ip_dest)
{
  struct
  {
    uint8_t MAC_dich[6];                    // MAC dich
    uint8_t MAC_nguon[6];                   //MAC nguon
    uint16_t Ethernet_type;                 //ethernet type
    uint16_t Hardwave_type;                 //hardwave type
    uint16_t Protocol_type;                 //protocol type (ARP)
    uint16_t Size;                          //size
    uint16_t Opcode;                        //opcode
    uint8_t MAC_sender[6];                  //sender MAC
    uint8_t IP_sender[4];                   //sender IP
    uint8_t MAC_target[6];                  // Target MAC
    uint8_t IP_target[4];                   // Target IP
  }ARP_request;
  uint8_t MAC_dest[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uint8_t MAC_target[6]={0x00,0x00,0x00,0x00,0x00,0x00};
  memcpy(ARP_request.MAC_dich,MAC_dest,6);
  memcpy(ARP_request.MAC_target,MAC_target,6);
  memcpy(ARP_request.IP_target,ip_dest,4);
  memcpy(ARP_request.MAC_nguon,macaddr,6);
  memcpy(ARP_request.MAC_sender,macaddr,6);
  memcpy(ARP_request.IP_sender,ip,4);
  ARP_request.Ethernet_type=0x0608;
  ARP_request.Hardwave_type=0x0100;
  ARP_request.Protocol_type=0x0008;
  ARP_request.Size=0x0406;
  ARP_request.Opcode=0x0100;     //request
  UART_putString("Gui gon tin arp request\r\n");
  NET_SendFrame((uint8_t *)&ARP_request,42);   //gui goi ARP reponse di
}


int8_t ARP_table_checkIP(uint8_t *ip_check)  //kiem tra xem co ton tai ip trong bang chua
{
 int i;
 for(i=0;i<5;i++)
 {
   if(ip_check[0] == ARP_table[i].ip[0] && ip_check[1] == ARP_table[i].ip[1] && ip_check[2] == ARP_table[i].ip[2] && ip_check[3] == ARP_table[i].ip[3] )
      return i+1; //tra ve vi tri cua ip trong table
 }
 return -1;
}
void ARP_table_setIP(uint8_t *ip_set, uint8_t *mac_set)
{
  if(ARP_table_checkIP(ip_set) == -1) //neu chua ton tai IP trong table
  {
    sprintf(debug_string,"Da luu ip %i.%i.%i.%i = %02X:%02X:%02X:%02X:%02X:%02X\r\n",ip_set[0],ip_set[1],ip_set[2],ip_set[3],mac_set[0],mac_set[1],mac_set[2],mac_set[3],mac_set[4],mac_set[5]);
    UART_putString(debug_string);
    memcpy(ARP_table[ARP_table_index].ip,ip_set,4);
    memcpy(ARP_table[ARP_table_index].mac,mac_set,6);
    ARP_table_index++;if(ARP_table_index==5)ARP_table_index=0; //neu so luong ip vuot qua muc cho phep thi quay lai
  }
  else
    UART_putString("IP da ton tai trong bang\r\n");
}

uint32_t ARP_clear_time;
void ARP_clear_table(void)
{
  uint8_t *end;
  uint8_t *start;
  if(++ARP_clear_time > COUNT_TICK)
  {
     ARP_clear_time=0;
     //xoa bang
     start = (uint8_t *)&ARP_table;
     end =  start + sizeof(ARP_table);
     for(;start<end;start++)
        *start=0;
  }
}

void ARP_table_get_MAC(uint8_t *ip_check,uint8_t * MAC_dest)
{
  int i=ARP_table_checkIP(ip_check) - 1;
  memcpy(MAC_dest,ARP_table[i].mac,6);
}
//--------------------------------------------------------------------------
//End file