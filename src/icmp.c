/*	ICMP:: INTERNET CONTROL MESSAGE PROTOCOL

	ETHERNET BUFFER
	Ethernet Frame Type II		IPV4		ICMP
	14byte						20byte		unknow

	https://webee.technion.ac.il/labs/comnet/netcourse/CIE/Course/Section3/7.htm
	IP PACKET
	0			4				8				16		19					31
	Version		Header length 	Type of Service |		Total Length 
	-------------------------------------------	|------------------------------
				Identification					|IP Flags 		Fragment Offset
	--------------------------------------------|------------------------------
	Time to Live 				Protocol 		|		Header Checksum
	---------------------------------------------------------------------------
				Source Address
	---------------------------------------------------------------------------
				Destination Address
	---------------------------------------------------------------------------			
				IP Option
	---------------------------------------------------------------------------
				Data

	Header Length: this 4 bit field tells us the length of the IP header in 32 bit increments. T

	Type of Service: this is used for QoS (Quality of Service). 8bit
		111 - 	Network Control 		011 - 	Flash
		110 - 	Internetwork Control 	010 - 	Immediate
		101 - 	CRITIC/ECP 				001 - 	Priority
		100 - 	Flash Override 			000 - 	Routine

	Total Length: this 16-bit field indicates the entire size of the IP packet (header and data) in bytes. 
		The minimum size is 20 bytes (if you have no data) and the maximum size is 65.535 bytes, 
		that’s the highest value you can create with 16 bits.

	Identification: If the IP packet is fragmented then each fragmented packet will use the same 16 bit 
		identification number to identify to which IP packet they belong to.

	IP Flags: These 3 bits are used for fragmentation:
	    The first bit is always set to 0.
	    The second bit is called the DF (Don’t Fragment) bit and indicates that this packet should not be fragmented.
	    The third bit is called the MF (More Fragments) bit and is set on all fragmented packets except the last one.
	
	Time to live: (TTL) time life exits of the packet in networking before delete by router
		Everytime an IP packet passes through a router, the time to live field is decremented by 1. 
		Once it hits 0 the router will drop the packet and sends an ICMP time exceeded message to the sender. 
		The time to live field has 8 bits and is used to prevent packets from looping around forever (if you have a routing loop).
	
	Protocol: this 8 bit field tells us which protocol is enapsulated in the IP packet, 
		for example ICMP = 0x01 TCP has value 6 and UDP has value 17. positon in packet 23

	Header Checksum: this 16 bit field is used to store a checksum of the header. 
		The receiver can use the checksum to check if there are any errors in the header.	

	Source Address: at the positon in the packet 26
	Destination Address: at the positon in the packet 30

	IP Protocol have Ether Type  0x0800 	
*/

/*
	ICMP STRUCT
	0		8			16			  32
	Type	Code	|	ICMP Checksum
	----------------|-------------------
	  Idetifier		|   Sequence Number
	  			   ...
	------------------------------------
				Macgic Number
				IP
				Port
				State
				Length
				Data
	Type: at the position in the packet 34
		Type = 0x08 is ping quesion
		Type = 0x08 is ping answer

*/

/*	Checksum
	Checksum is a vaule calculated of packet data. at the recevie Checksum have add header of the packet.
	inwihle destination calculated checksum and exam with total current value in the header of the packet
	1. add all byte 16 bit, if vaule is old then add 0x00 for prupose even
	2. If return value more than 16 bit then continew add 16 bit -> 1 byte 16 bit completed
	3. Inver

	ex: 
		0x00,0x00,0x49,0x66,0x00,0x01,0x0B,0xF5,0x61,0x62,0x63,0x64,0x65,0x66,
		0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
		0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69

		0x0000 + 0x0001 + 0x0BF5 + 0x6162 + 0x6364 + 0x6566 + 0x6768 + 0x696A + 0x6B6C + 0x6D6E + 
		0x6F70 + 0x7172 + 0x7374 + 0x7576 + 0x7761 + 0x6263 + 0x6465 + 0x6667 + 0x6869 = 0x6B693

		0x0006 + 0xB693 = 0xB699

		~checksum = checksum
*/

#include "icmp.h"
extern const uint8_t macaddr[6];
extern const uint8_t ip[4];
extern uint8_t debug_string[60];

void ICMP_read(uint8_t *ICMP_Frame,uint16_t len)
{
   ICMP_struct *ICMP_Ping_Packet = (ICMP_struct *)ICMP_Frame;

  //kiem tra dia chi ip xem co phai no gui cho minh khong
  if( memcmp(ICMP_Ping_Packet->DestIP,ip,4) )return; // dung memcmp de so sanh, neu khac thi thoat

  //kiem tra xem co phai ping reuqest khong ?
  if(ICMP_Ping_Packet->ICMP_Type == 0x08)
  {
      UART_putString("Da nhan 1 goi ping request\r\n");
     //phan hoi lai goi ping
  }
}

void ICMP_ping_reply(ICMP_struct * ping_packet,uint16_t len)
{
  uint8_t buffer_temp[6];  //buff trung gian phuc vu hoan doi

  //hoan vi MAC
  memcpy(buffer_temp,ping_packet->MAC_nguon,6);           //copy mac nguon vao bo nho tam
  memcpy(ping_packet->MAC_nguon,ping_packet->MAC_dich,6); //copy mac dich vai mac nguon
  memcpy(ping_packet->MAC_dich,buffer_temp,6);

  //hoan vi IP
  memcpy(buffer_temp,ping_packet->SourceIP,4);           //copy ip nguon vao bo nho tam
  memcpy(ping_packet->SourceIP,ping_packet->DestIP,4);   //copy ip dich vai mac nguon
  memcpy(ping_packet->DestIP,buffer_temp,4);

  //make packet reply
  ping_packet->ICMP_Type = 0x00; //type = reply
  ping_packet->ICMP_checkSum = ICMP_checksum(ping_packet,len);

  NET_SendFrame((uint8_t *)ping_packet,len); //gui phan hoi
}

uint16_t ICMP_checksum(ICMP_struct *icmp_packet,uint16_t icmp_len )
{
    uint32_t checksum=0;
    uint8_t *ptr;
    icmp_packet->ICMP_checkSum=0; //reset check sum
    icmp_len-=34;    //bo qua truong ethernet va truong ip
    ptr = &icmp_packet->ICMP_Type;
    while(icmp_len>1) //cong het cac byte16 lai
    {
       checksum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
       ptr+=2;
	   icmp_len-=2;
    }
    if(icmp_len) checksum+=((uint32_t)*ptr)<<8; //neu con le 1 byte
    while (checksum>>16) checksum=(uint16_t)checksum+(checksum>>16);

    //nghich dao bit
    checksum=~checksum;

    //hoan vi byte thap byte cao
    return (  ((checksum>>8)&0xFF) | ((checksum<<8)&0xFF00)  );
}
//--------------------------------------------------
//End file

