/*	UDP::	User Datagram Data
	use 16 bit for address. Not use gate 0
	1 - 1023:	"well-know"	permission root
	2014  - 49151: gate is use register
	49152 - 65535: gate temperory use client connect to server

	ETHERNET FRAME
	ETHERNET type II		IP			UPD
	(14byte)				(20byte)	length

	UPD is protocol (IP) 0x11

	STRUCT UPD
	+	  |bit 0 - 15	    |bit 16 - 31
	----|---------------|-----------------
	0	  |Source Port 	  |Destination port
	32	|Length 		    |Checksum
	----|---------------------------------
	64	|			  Data
*/


#include "udp.h"

extern const uint8_t macaddr[6];
extern const uint8_t ip[4];
extern uint8_t debug_string[60];



void UDP_read(uint8_t *UDP_Frame,uint16_t len)
{
  UDP_struct *UDP_Struct_Frame = (UDP_struct *)UDP_Frame;

  //kiem tra dia chi ip xem co phai no gui cho minh khong
  if( memcmp(UDP_Struct_Frame->DestIP,ip,4) )return; // dung memcmp de so sanh, neu khac thi thoat

  UART_putString("Da nhan 1 goi UDP\r\n");
}


uint8_t UDP_send(uint8_t *IP_dest,uint16_t port_dest,uint8_t *data,uint32_t timeout)
{
   uint32_t count=0;
   uint16_t length_mess,frame_length;
   UDP_struct UDP_Struct_Frame;
   length_mess = strlen(data);
   while(1)
   {
     if(ARP_table_checkIP(IP_dest) != -1)break; // da nhan dc MAC
     ARP_send_request(IP_dest);
     delay_ms(50);
     count+=50;
     if(count >= timeout)
     {
       UART_putString("Chua gui goi tin udp\r\n");
       return 0; //gui that bai
     }
   }

   //make Ethernet II
   ARP_table_get_MAC(IP_dest,UDP_Struct_Frame.MAC_dich);   //mac cua thang nhan
   memcpy(UDP_Struct_Frame.MAC_nguon,macaddr,6);  //mac cua enc28j60
   UDP_Struct_Frame.Ethernet_type = 0x0008; // Type = 0x800 = IP

   //make IP packet
   UDP_Struct_Frame.Header_length = 0x45;
   UDP_Struct_Frame.Services=0x00;
   UDP_Struct_Frame.TotoLength=swap16(length_mess+8+20);	//
   UDP_Struct_Frame.Identification=0x2111;	//
   UDP_Struct_Frame.Flag=0x0000;
   UDP_Struct_Frame.TimeToLive=0x80;
   UDP_Struct_Frame.Protocol=0x11; //UDP
   UDP_Struct_Frame.CheckSum=0x0000;	//
   memcpy(UDP_Struct_Frame.SourceIP,ip,4); //ip cua enc28j60
   memcpy(UDP_Struct_Frame.DestIP,IP_dest,4); //ip cua thang nhan
   //tinh checksum goi ip
   UDP_Struct_Frame.CheckSum=NET_ipchecksum((uint8_t *)&UDP_Struct_Frame.Header_length);


   //make UDP packet
   UDP_Struct_Frame.UDP_Source_Port = swap16(ENC28J60_UDP_PORT);
   UDP_Struct_Frame.UDP_Dest_Port =  swap16(port_dest);
   UDP_Struct_Frame.UDP_Length = swap16(length_mess + 8);
   UDP_Struct_Frame.UDP_Checksum=0x0000;
   strcpy((char*)UDP_Struct_Frame.data,data); //copy data to struct
   //tinh checksum cho udp
   UDP_Struct_Frame.UDP_Checksum = UDP_checksum(&UDP_Struct_Frame);

   frame_length = swap16(UDP_Struct_Frame.UDP_Length) + 34;
   NET_SendFrame((uint8_t *)&UDP_Struct_Frame,frame_length); //gui goi tin udp
   return 1; //da gui
}

/*
*	Checksum UDP: IP Protocol (0x11) + IP source + IP dest +  UDP packet + UDP packet length
*/
uint16_t UDP_checksum(UDP_struct *UDP_Struct_Frame)
{
   uint32_t checksum;
   uint8_t *ptr;
   uint16_t length;
   UDP_Struct_Frame->UDP_Checksum=0; //reset check sum
   length = swap16(UDP_Struct_Frame->UDP_Length) + 8;
   ptr = (uint8_t *)&UDP_Struct_Frame->SourceIP;

   checksum=0x11 + length-8;
   while(length>1) //cong het cac byte16 lai
    {
       checksum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
       ptr+=2;
	   length-=2;
    }
    if(length) checksum+=((uint32_t)*ptr)<<8; //neu con le 1 byte
    while (checksum>>16) checksum=(uint16_t)checksum+(checksum>>16);
    //nghich dao bit
    checksum=~checksum;
    //hoan vi byte thap byte cao
    return swap16(checksum);
}
//--------------------------------------------------
//end file