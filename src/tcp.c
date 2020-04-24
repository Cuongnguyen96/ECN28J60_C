/*	TCP	:: Transmission Control Protocol
	I. Set configuration connect
		3-way handshake: befor client connect to server, server must be register gate for this connections.
		this is open passive, client is open avtive.
		
		1. Client request open service with method send packet SYN (packet TCP) to server, in the packet 
		   sequence number have value ramdom X
		2. Server response with method send for client SYN-ACK, in the packet acknowledgement number have
		   value  X + 1, argument sequence number have value ramdom Y
		3. After that handshake, client send packet ACK, in the packet argument sequence number have value
		   X + 1, argument acknowledgement number have value Y + 1

	II. Transmit data
		step 1. and step 2. is call Initial Sequence Number (ISN), the number have ramdom and this is a remark
		for the packet. after transmit the vaule have increase, so that you can arrange

		in reality, byte at first have been assigned number in the packet data and salve send infor received 
		with method send the next number of the byte watting...

		EX: Computer A send 4 byte 100 is the first of the packet data (100	101 102 103), the salve send 
		message for master is content 104 because this is byte is watting the next number. The purpose 
		you can understand salve have 4 byte is successful. in case 2 byte at tail is fault, then salve
		send message with content 102. But fault at the fist -> SCTP (Stream Control Transmitssion Protocol)

	III. Disconnect
		How to disconnect, master and slave action 4-way handshake and direction have disconnet is dependece
		When one of want to disconnet, this is send packet FIN and anthor feedback infor receive ACK.
*/

/*	TCP STRUCT


	+	|Bit 0- 3		4 - 9		10 - 15		|	16 - 31			 |
	----|---------------------------------------|------------------- |
	0	|			Source port 				|	Destination port |
	----|----------------------------------------------------------- |
	32	|			Sequence Number 								 |
	----|----------------------------------------------------------- |
	64	|			Acknowledgement Number 							 |	Header
	----|----------------------------------------------------------- |	(20 bytes)
	96	|Data Offet		Reserved	Flags		|	Window 			 |
	----|---------------------------------------|------------------- |
	128	|			Checksum					|	Urgent Pointer   |
	----|----------------------------------------------------------- 
	160	|			Options											 
	----|-----------------------------------------------------------
	160/|			Data
	192+|

	Source port: port is master
	Destination port: port is slave
	Sequence Number: 
		If flag SYN is set, this is number the first packet and head byte send have value number + 1.
		If flag SYN not set, this is number of the head byte
	Acknowledgement Number: 
		if flag ACK is set then value of the field is the next number of the next packet for need slave
	Data Offset: regula length of header. length max 5 character (160bit) and length max 15 character (480 bit)
	Reserved: for the feature is set 0
	Flags: URG ACK PSH RST SYN FIN
		ACK: Acknowledgement
		SYN: Synchronizes sequence number to initial a TCP connetion 
		FIN: Finish Flag Indicates the end of data transmission to finish a TCP connection
		PSH: Push – Transport layer by default waits for some time for application layer to send enough data 
			 equal to maximum segment size so that the number of packets transmitted on network minimizes w
			 hich is not desirable by some application like interactive applications(chatting)
		URG: Urgent – Data inside a segment with URG = 1 flag is forwarded to application layer immediately 
			 even if there are more data to be given to application layer.
			 It is used to notify the receiver to process the urgent packets before processing all other packets
	Window: a lot of bytes can receive from the vaule of the field ACK
	Checksum: exam all the packet TCP and IP
	Urgent Pointer: URG is set  

	Reference: https://www.geeksforgeeks.org/tcp-flags/
*/

#include "tcp.h"
extern const uint8_t macaddr[6];
extern const uint8_t ip[4];
extern uint8_t debug_string[60];

void TCP_read(uint8_t *TCP_Frame,uint16_t len)
{
  uint16_t port;
  uint32_t dat_ack;
  uint16_t port,datalength,i;

  TCP_struct *TCP_Struct_Frame = (TCP_struct *)TCP_Frame;
  //kiem tra dia chi ip xem co phai no gui cho minh khong
  if( memcmp(TCP_Struct_Frame->DestIP,ip,4) )return; // dung memcmp de so sanh, neu khac thi thoat
  if(TCP_Struct_Frame->TCP_Flags == TCP_SYN)
  {
     //reply voi SYN|ACK
     //make reply
     memcpy(TCP_Struct_Frame->MAC_dich,TCP_Struct_Frame->MAC_nguon,6);
     memcpy(TCP_Struct_Frame->MAC_nguon,macaddr,6);
     TCP_Struct_Frame->CheckSum=0;
     memcpy(TCP_Struct_Frame->DestIP,TCP_Struct_Frame->SourceIP,4); //hoan vi source, dest
     memcpy(TCP_Struct_Frame->SourceIP,ip,4);                       //ip cua minh
     port = TCP_Struct_Frame->Source_Port;
     TCP_Struct_Frame->Source_Port = TCP_Struct_Frame->Dest_Port;
     TCP_Struct_Frame->Dest_Port = port;
     TCP_Struct_Frame->Acknowledgement = swap32(swap32(TCP_Struct_Frame->Sequence_Number) + 1);
     TCP_Struct_Frame->Sequence_Number = swap32(2071998);
     TCP_Struct_Frame->TCP_Flags = TCP_SYN | TCP_ACK;
     TCP_Struct_Frame->TCP_Checksums=0;
     TCP_Struct_Frame->Urgent_Pointer=0;
     TCP_Struct_Frame->CheckSum = NET_ipchecksum((uint8_t *)&TCP_Struct_Frame->Header_length);  //tinh checksum cho goi IO
     TCP_Struct_Frame->TCP_Checksums = TCP_checksum(TCP_Struct_Frame);

     NET_SendFrame((uint8_t *)TCP_Struct_Frame,len); //gui goi tin tcp reply
  }
  if(TCP_Struct_Frame->TCP_Flags == TCP_ACK)
  {
     UART_putString("Da ket noi 1 client\r\n");
  }
  else if(TCP_Struct_Frame->TCP_Flags == (TCP_FIN|TCP_ACK))
  {
     //reply voi ACK
     //make reply
     memcpy(TCP_Struct_Frame->MAC_dich,TCP_Struct_Frame->MAC_nguon,6);
     memcpy(TCP_Struct_Frame->MAC_nguon,macaddr,6);
     TCP_Struct_Frame->CheckSum=0;
     memcpy(TCP_Struct_Frame->DestIP,TCP_Struct_Frame->SourceIP,4); //hoan vi source, dest
     memcpy(TCP_Struct_Frame->SourceIP,ip,4);                       //ip cua minh
     port = TCP_Struct_Frame->Source_Port;
     TCP_Struct_Frame->Source_Port = TCP_Struct_Frame->Dest_Port;
     TCP_Struct_Frame->Dest_Port = port;
     dat_ack =  TCP_Struct_Frame->Acknowledgement;
     TCP_Struct_Frame->Acknowledgement = swap32(swap32(TCP_Struct_Frame->Sequence_Number) + 1);
     TCP_Struct_Frame->Sequence_Number =   dat_ack;
     TCP_Struct_Frame->TCP_Flags = TCP_ACK;
     TCP_Struct_Frame->TCP_Checksums=0;
     TCP_Struct_Frame->Urgent_Pointer=0;
     TCP_Struct_Frame->CheckSum = NET_ipchecksum((uint8_t *)&TCP_Struct_Frame->Header_length);  //tinh checksum cho goi IO
     TCP_Struct_Frame->TCP_Checksums = TCP_checksum(TCP_Struct_Frame);

     NET_SendFrame((uint8_t *)TCP_Struct_Frame,len); //gui goi tin tcp reply
  }
  else if(TCP_Struct_Frame->TCP_Flags == (TCP_PSH|TCP_ACK))
  {
     //tinh do dai cua goi data va in ra man hinh
     datalength= swap16(TCP_Struct_Frame->TotoLength) -20 - (TCP_Struct_Frame->data_offset >> 2);  // ( >> 4)*4 = >> 2
     sprintf(debug_string,"%u:",datalength);
     UART_putString(debug_string);
     for(i=0;i<datalength;i++)
        UART_putChar(TCP_Struct_Frame->data[i]);

     //make reply
     len-=datalength; //resize len

     memcpy(TCP_Struct_Frame->MAC_dich,TCP_Struct_Frame->MAC_nguon,6);
     memcpy(TCP_Struct_Frame->MAC_nguon,macaddr,6);
     TCP_Struct_Frame->CheckSum=0;
     TCP_Struct_Frame->TotoLength = swap16(swap16(TCP_Struct_Frame->TotoLength) - datalength); //make totolength
     memcpy(TCP_Struct_Frame->DestIP,TCP_Struct_Frame->SourceIP,4); //hoan vi source, dest
     memcpy(TCP_Struct_Frame->SourceIP,ip,4);                       //ip cua minh
     port = TCP_Struct_Frame->Source_Port;
     TCP_Struct_Frame->Source_Port = TCP_Struct_Frame->Dest_Port;
     TCP_Struct_Frame->Dest_Port = port;
     dat_ack =  TCP_Struct_Frame->Acknowledgement;
     TCP_Struct_Frame->Acknowledgement = swap32(swap32(TCP_Struct_Frame->Sequence_Number) + datalength);
     TCP_Struct_Frame->Sequence_Number =   dat_ack;
     TCP_Struct_Frame->TCP_Flags = TCP_ACK;
     TCP_Struct_Frame->TCP_Checksums=0;
     TCP_Struct_Frame->Urgent_Pointer=0;
     TCP_Struct_Frame->CheckSum = NET_ipchecksum((uint8_t *)&TCP_Struct_Frame->Header_length);  //tinh checksum cho goi IO
     TCP_Struct_Frame->TCP_Checksums = TCP_checksum(TCP_Struct_Frame);

     NET_SendFrame((uint8_t *)TCP_Struct_Frame,len); //gui goi tin tcp reply
  }
}

void TCP_send(TCP_struct *TCP_Struct_Frame,uint16_t len,uint8_t *data)
{
    uint16_t data_length;
    strcpy(TCP_Struct_Frame->data,data);
    data_length=strlen(data);
    len+=data_length;
    TCP_Struct_Frame->TotoLength = swap16(swap16(TCP_Struct_Frame->TotoLength) + data_length); //make totolength
    TCP_Struct_Frame->CheckSum=0;
    TCP_Struct_Frame->TCP_Checksums=0;
    TCP_Struct_Frame->TCP_Flags = TCP_PSH|TCP_ACK;

    TCP_Struct_Frame->CheckSum = NET_ipchecksum((uint8_t *)&TCP_Struct_Frame->Header_length);  //tinh checksum cho goi IO
    TCP_Struct_Frame->TCP_Checksums = TCP_checksum(TCP_Struct_Frame);

    NET_SendFrame((uint8_t *)TCP_Struct_Frame,len);
}

/*	CHECKSUM

	Pseudo Header 		TCP/UPD Header 		TCP/UPD Body
	------------------------------------------------------------------
	-> TCP/UPD Checksum = Pseudo Header + TCP/UPD Header + TCP/UPD body

	TCP-Pseudo Header
		-------------------------------------------------------------------------------
							Pesudo IP header ( 12 bytes)
		-------------------------------------------------------------------------------
							Source IP (32 bits)
		-------------------------------------------------------------------------------
							Destination IP (32 bits)
		-------------------------------------------------------------------------------
		Fixed (8 bits)	|	Protocol Field (8 bits)		|	TCP Segment length (16 bit)
		-------------------------------------------------------------------------------
	
	-> So, the total size of the pseudo header(12 Bytes) = IP of the Source (32 bits) + IP of the Destination (32 bits) 
														 + TCP/UDP segment Length(16 bit) + Protocol(8 bits) + Fixed 8 bits


	TCP-Header
		+	| bit 0 - 15	    |bit 16 - 31
		----|---------------|-----------------
		0   |Source Port 	|Destination port
		32	|Length 	    |Checksum *
		--------------------------------------

	-> TCP-Header = Source port (16 bits) + Destination port (16 bits) + Length (16 bits) + Cheksum (16 bits)	
		
	TCP-Data
		------------------------------------
		|				TCP-Data			|
		|									|
		|		 ---------------------------
		|		| Padding (8bit) |
		------------------------
	-> TCP-Data =  length TCP  + Padding (8bits)

	Reference: http://www.tcpipguide.com/free/t_TCPChecksumCalculationandtheTCPPseudoHeader-2.htm
			   https://www.geeksforgeeks.org/calculation-of-tcp-checksum/
			   https://www.inacon.de/ph/data/TCP/Header_fields/TCP-Header-Field-Checksum_OS_RFC-793.htm

*/

uint16_t TCP_checksum(TCP_struct *TCP_Struct_Frame)
{
   uint32_t checksum;
   uint8_t *ptr;
   uint16_t length;
   length = swap16(TCP_Struct_Frame->TotoLength) - 20 + 8 ; //tinh length bat dau tu checksum
   ptr = (uint8_t *)&TCP_Struct_Frame->SourceIP;       //dia chi bat dau tinh checksum

   checksum=6 + length - 8;
   while(length>1) //cong het cac byte16 lai
    {
       checksum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
       ptr+=2;
       length-=2;
    };
    if(length) checksum+=((uint32_t)*ptr)<<8; //neu con le 1 byte
    while (checksum>>16) checksum=(uint16_t)checksum+(checksum>>16);
    //nghich dao bit
    checksum=~checksum;
    //hoan vi byte thap byte cao
    return swap16(checksum);
}


//--------------------------------------------------
//end file