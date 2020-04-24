/*	ETHERNET TYPE II FRAME
	MAC Receive		MAC Transmit	Ethernet type 	Payload 			CRC Checksum
	6 byte)			(6 byte)		(2 byte)		(46 - 1500 byte)	(4 byte)
	---------------------------------------------	----------------	------------
					MAC Header (14 byte)			Data				

	ARP Ether type = 0x0806
	
	ETHERNET BUFFER
	Ether net Frame type II 	ARP
	(14 byte)					(28 byte)

	example: 
	IP device transmit: 192.168.197
	IP device receive:  192.168.1.4
	The first 192.168.1.4 send request one packet ARP request (boardcast) to All device into internet with conten.
		Hey I'm 192.168.197, This MAC is xxx, who is 192.168.1.4 
	All device receive this packet, but one device IP 192.168.1.4 will send packet ENC28J60 have ip 192.168.197
		Hey T'm 192.168.1.4, This MAC is yyy, 
	ENC28J60 received packet from 192.168.1.4 and save address MAC. NOW you can send to device with MAC.
	
	ARP Resquest
		(boardcast)				(PC)					(ARP)		(Ethernet)

		ff	ff	ff	ff	ff	ff	d4	be 	d9	55	xx 	xx	08	06		00	01
		----------------------	----------------------	------		------
		Destination MAC			Source MAC				Ethertype	HType

		(IPV4)	(4)		(req)	(Router)				(PC)
		08	00	06	04	00	0x 	d4	be 	d9	55	xx 	xx 	xx 	xx 		xx xx
		------	------	------	----------------------	-----------------
		Ptype	Length	OPER	Sender MAC				Sender IP 				

		(default)
		00	00	00	00	00	00	xx 	xx xx 	xx
		----------------------	--------------
		Target MAC 				Target IP

	ARP Response
	as the same ARP requset but any different at 
		OPER = 00 02
		Destination MAC, Target MAC is MAC of decive request

	time ARP
		regula table ARP is upated to 30 - 60s 	
*/

/*	DIAGRAM  SYSTEM
					TCP		   UDP 		   ARP
					^			^			^
					|___________|___________|
								|
							   NET
							   	^
							 ENC28J60
*/
/*	OSI MODEL
	7. Application layer	HTTP, SMTP, telent
	6. Presentaion layer	
	5. Session layer		ASAP, TLS, SSH
	4. Transport layer		UDP, TCP, SCTP
	3. Netwok layer			IP, ICMP, ARP
	2. Data link layer		Ethernet, Wifi
	1. Physical layer
*/

#include "net.h"
uint8_t eth_buffer[BUFFER_LENGTH];
const uint8_t ip [4] = {192,168,1,197};
extern const uint8_t macaddr[6];
extern const uint8_t ip[4];
extern uint8_t debug_string[60];


// Handler, analist packet
void NET_SendFrame(uint8_t *EthFrame,uint16_t len)
{
  sprintf(debug_string,"Gui goi tin ethernet co do dai: %i\r\n",len);
  UART_putString(debug_string);
  ENC28J60_send_packet(EthFrame,len);
}

/*
*	1. ARP sizeof 42 byte, if not enough 42 byte is deny
*	2. Check Ethernet Type bit 12 and 13 is 0x0806 
*	3. exctualy is packet ARP 
*/

void NET_read(uint8_t *net_buffer,uint16_t len)
{
  //in ra do dai cua goi tin
  sprintf(debug_string,"Da doc goi tin ra, do dai goi tin=%i\r\n",len);
  UART_putString(debug_string);

  //kiem tra xem co phai goi tin ARP khong
  if(len > 41)
  {
     if(net_buffer[12] == 0x08 && net_buffer[13] == 0x06)
     {
       //day dung la goi tin ARP
       UART_putString("Day la goi tin ARP\r\n");
       //gui goi tin cho thu vien ARP.h xu li
       ARP_read_packet(net_buffer, len);
     }
       //kiem tra xem co phai goi tin IP khong
	 else if(net_buffer[12] == 0x08 && net_buffer[13] == 0x00)
	 {
	     //check ip
	     //UART_putString("Day la goi tin IP\r\n");
	     NET_ipRead(net_buffer,len);
	  }
  }
}


void NET_ipRead(uint8_t *IP_Frame,uint16_t len)
{
  //chung ta se kiem tra xem goi tin do thuoc loai nao tcp hay udp hay icmp
  if(IP_Frame[23] == 0x01) //it is ICMP packet
  {
     ICMP_read(IP_Frame,len);
  }
  else if(IP_Frame[23] == 0x11) //it is UDP packet
  {
     UDP_read(IP_Frame,len);
  }
  else if(IP_Frame[23] == 0x06) //it is TCP packet
  {
     TCP_read(IP_Frame,len);
  }
}


uint16_t NET_ipchecksum(uint8_t *IP_packet_start)
{
  uint32_t checksum=0;
  uint16_t length=20;
    while(length) //cong het cac byte16 lai
    {
       checksum += (uint16_t) (((uint32_t)*IP_packet_start<<8)|*(IP_packet_start+1));
       IP_packet_start+=2;
	   length-=2;
    }
    while (checksum>>16) checksum=(uint16_t)checksum+(checksum>>16);
    //nghich dao bit
    checksum=~checksum;
    //hoan vi byte thap byte cao
    return  swap16(checksum);
}

// check is packet 
void NET_loop(void)
{
 uint16_t len;
 while(1)
 {
    len=ENC28J60_read_packet(eth_buffer,BUFFER_LENGTH);
    if(len==0)return;
    else NET_read(eth_buffer,len);
 }
}




//--------------------------------------------------------------------------
//End file
