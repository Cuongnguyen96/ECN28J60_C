#include "enc28j60.h"
/*
	struct command : 3 bit Opcode + 5 bit Argument + bit data
				SPI INTRUCTION SET FOR THE ENC28J60
									|		Byte 0		 |     Byte 1	
	Name and Mnemoic				| Opcode |  Argument | 	    Data
	Read Control Register 	(RCR)	| 0 0 0  | a a a a a |		N/A
	Read Buffer Menmory		(RBM)	| 0 0 1  | 1 1 0 1 0 |		N/A
	Write Control Register	(WCR)	| 0 1 0  | a a a a a | d d d d d d d d 
	Write Buffer Memmnory	(WBM)	| 0 1 1	 | 1 1 0 1 0 | d d d d d d d d 
	Bit Field Set 			(BFS)	| 1 0 0  | a a a a a | d d d d d d d d 
	Bit Field Clear			(BFC)	| 1 0 1  | a a a a a | d d d d d d d d 
	System command 			(SC)	| 1 1 1  | 1 1 1 1 1 |		N/A
	a: Control Register Address
	d: data payload
*/

/*
				CENC28J60 CONTROL REGISTER MAP BANK
	
	Bank0			 Bank1			 Bank2			 Bank3
	Addr 	name 	|Addr 	name	|Addr 	Name 	|Addr 	name
	00h		ERDPTL	|00h	EHT0	|00h	MACON1	|00h	MAADR1
	01h		ERDPTH	|				|				|0Ah	MISTAT
	02h		EWRPTL	|				|02h	MACON3	|
	03h		EWRPTH 	|				|04h	MABBIPG	|
	06h		ETXNDL	|				|06h	MAIPGL	|
	07h		ETXNDH 	|				|07h	MAIPH	|
	08h		ERXSTL	|				|0Ah	MAMXFLL	|
	09h		ERXSTH	|				|0Bh	MAMXFLH |
	0Ch		ERXRDPTL|				|12h	MICMD 	|
	0Dh		ERXRDPTH|				|14h	MIREGADR|...
	...				|...			|				|15h	ECOCON
					|				|16h	MIWRL	|
					|				|17h	MIWRH	|
					|18h	ERXFCON	|18h	MIRDL	|
					|19h	EPKTCNT	|19h	MIRDH	|
	1Bh		EIE 	|1Bh	EIE 	|1Bh 	EIE 	|1Bh	EIE 	
	1Dh		ESTAT 	|1Dh	ESTAT 	|1Dh 	ESTAT 	|1Dh 	ESTAT
	1Eh		ECON2	|1Eh	ECON2	|1Eh	ECON2	|1Eh	ECON2		
	1Fh		ECON1	|1Fh	ECON1	|1Fh	ECON1	|1Fh	ECON1

		One address but for 4 type command, so that chip is seprate is command type
	in the register ECON1 is address 1F
		BANK 0
			ETXST = ETXSTL (0x04) + ETXSTH (0x05) Transmit Start
			ETXND = ETXNDL (0x06) + ETXNDH (0x07) Transmit End
			ERXST = ERXSTL (0x08) + ERXSTH (0x09) Receive Start
			ERXND = ERXNDL (0x0A) + ERXNDH (0x0B) Receive End
			ERXRDPT = ERXRDPTL (0x0C) + ERXRDPTH (Ox0D) Receive Reset pointer
			ERXWRPT = ERXWRPTL (0x0E) + ERXWRPTH (0x0F) Transmit Reset pointer
		BANK 2
			MAIPG = MAIPGL (0x06) + MAIPGH(0x07) Non-Back-to-Back Inter-Packet Byte 
			MABBIPG Back-to-Back Inter-Packet 
			MAMXFL = MAMXFLL (0x0A) + MAMXFLH (0x0B) Maximun Frame Length

		Because range address command form 00h -> 1Fh, you need 5bits 
	thus can you add at 6bit and 7bit for select Bank
		ERDPTL	0x00
		EHT0	0x00|0x20
		MACON1	0x00|0x40
		MAADR1  0x00|0x60
		
		Now you must be seprate for get two bit 6 and bit 7 after that shift right 
	5, because when is transfer for ENC28J60 that is at posiotion bit 1 and bit 0.
		0xx0 0000 >> 5	--> 0000 00xx
		but each other bank is exits ECON1 you can set resgister ECON1 is method
		Bit Field Set 	Opcode 0x80
		Bit Field Clear Opcode 0xA0		

*/

/*	CONFIGURATION MAC ADDRESS
	MACON1:: MAC CONTROL REGISTER 1
		- 	- 	- 	r 	TXPAUS 	RXPAUS 	PASSALL	 MARXEN
		bit 7	<------------------------------	 bit0

		MARXEN: MAC Receive Enable bit
		PASSALL: Pass ALl Receive Enable bit
		RXPAUS: Pause Control Frame Reception Enable bit
		TXPAUS: Pause Control Frame Transmission Enable bit

	MACON3:: MAC CONTROL REGISTER 3
		PADCFG2		PADCFG1		PADCFG0 	TXCRCEN 	PHDREN 		HFRMEN 		FRMLNEN 	FULDPX
		bit 7	<-----------------------------------------------------------------------	bit0
		
		PADCFG2:PADCFG0: Automatic Pad and CRC Configuration bits
		TXCRCEN: Transmit CRC Enable bits
		FRMLNEN: Frame Length Checking Enable bits

	1. Set bit MARXEN TXPAUS RXPAUS PASSALL in register MACON1
	2. Clear register MACON2 // dose not exist 
	3. Set bit PADCFG0 TXCRCEN FRMLNEN tin register MACON3. 
	4. Set MAIPG =  0xC12
	5. Set MABBIPG = 0x12 
	6. Set MAMXFL <= 1518

	All Address |0x80 for purpose MAC or MII.
	Beacause read/write register MAC/MII the fist byte data is dummy, 2st is data
	
*/

/*
			ETHERNET BUFFER ORGANIZATION

	Transmit Buffer Start 	(ETXSTH:ETXSTL)		0000h
	Buffer Write Pointer	(EWRPTH:EWRPTL)		AAh		Transmit Buffer Data WBM
	Transmit Buffer End 	(ETXNDH:ETXNDL)				Transmit Buffer
	Receive Buffer Start	(ERXSTH:ERXSTL)				Receive Buffer Cricular FIFO
	Buffer Read Pointer		(ERDPTH:ERDPTL)		55h		Receive Buffer Data RBM
	Receive Buffer End 		(ERXNDH:ERXNDL)		1FFh

*/

/*	
	ERXFCON:: ETHERNET RECIVER FILTER CONTROL REGISTER
		UCEN 	ANDOR 	CRCEN 	PMEN 	MPEN 	HTEN 	MCEN 	BCEN
		bit 7	<-------------------------------------------	bit0		

	ESTAT:: ETHERNET STATUS REGISTER
		INT 	BUFFER 	 	r 		LATECOL 	RXBUSY 		TXABRT 	CLKRDY
		bit 7	<-------------------------------------------------	bit0

		CLKRDY: Clock Ready bit


	ECON1:: ETHERNET CONTROL REGISTER 1
		TXRST	RXRST	DMAST	CSUMEN	TXRTS	RXEN	BSEL1	BSEL0
		bit 7	<--------------------------------------------	bit 0

		TXRST: Transmit logic Reset bit
		RXRST: Receive logic Reset bit
		DMAST: DMA Start and buys Status bit
		CSUMEN: DMA Check Sum Enable bit
		TXRTS: Transmit Request to Send bit
		RXEN: Receive Enable bit
		BSEL: Bank Select bit
	
	ECON2:: ETHERNET CONTROL REGISTER 2
		AUTOINC		PKTDEC		PWRSV	r	VRPS	-	-	-
		bit 7	<----------------------------------------	bit 0	

		PKTDEC: Packet Decement bit	

	PHY REGISTER
	MICMD:: MII COMMAND REGISTER
		-		-	-	-	-	-	MIISCAN		MIIRD
		bit7	<--------------------------		bit0	

	MISTAT:: MII STATUS REGISTER
		-	-	-	-	r 	NVAID	SCAN	BUSY
		bit7	<-----------------------	bit0

	PHCON2:: PHY CONTROL REGISTER 2
		-		FRCLNK	TXDIS	r 	r 	JABBER	r 	HDLDIS
		bit15	<--------------------------------	bit8
		r 		r 		r 		r 	r 	r 		r 	r
		bit7	<--------------------------------	bit0
		
		PHCON2.HDLDIS bit to prevent automatic loopback of the data which is transmitted.
		HDLDIS: Haft-Duplex Loopback Disable bit
			PHCON1<8> = 1 or PHCON1<14> = 1 ->this bit is ignored.
			PHCON1<8> = 0 or PHCON1<14> = 0
				1 = Transmitted data will only be sent out on the twisted-pair interface
				0 = Transmitted data will be looped back to the MAC and sent out on the twisted-pair interface
		
	PHLCON:: PHY MODULE LED CONTROL REGISTER
		r		r		r		r 		LACFG3 	LACFG2	LACFG1 	LACFGO
		bit15	<---------------------------------------------	bit8
		LACFG3 	LACFG2 	LACFG1	LACFGO 	LFRQ1 	LFRQ0 	STRCH 	r
		bit7	<--------------------------------------------	bit0

		LACFGO-LACFG3: Led A Configuration bits		
		LBCFGO-LBCFG3: Led B Configuration bits
*/


/* 	EIE:: ETHERNET INTERRUPT ENABLE REGISTER
		INTIE 	PKTIE 	DMAIE 	LINKIE 	TXIE	r 	TXERIE	RXERIE
		bit7	<-----------------------------------------	bit0

		INTIE: Global INT Interrupt Enable bit 
		PKTIE: Receive Packet Pending Interrupt Enable bit
*/

/*	ARP
	How to packet send to device exctualy in LAN. This is need MAC address of the physical device, 
	thus ARP use method resolve IP -> MAC
*/

static uint16_t NextPacketPtr;

//--------------------------------------------------------------------------
void ENC29J60_ini(void)
{

	UART_putString("Dang khoi tao ENC28J60 ...\r\n");
	ENC28J60_write_command(ENC28J60_SOFT_RESET,0x1F,0); //soft reset
	delay_ms(2);
 	while(!ENC28J60_read_command(ENC28J60_READ_CTRL_REG,ESTAT)&ESTAT_CLKRDY); //cho bit CLKRDY duoc set
	if(ENC28J60_readByte(ERDPT) != 0xFA)ENC28J60_error(); //khoi tao that bai
	else
	 {
	    NextPacketPtr=RXSTART_INIT;
	    //cau hinh kich thuoc bo dem truyen nhan
	    ENC28J60_writeByte16(ERXST,RXSTART_INIT);
	    ENC28J60_writeByte16(ERXND,RXSTOP_INIT);

	    ENC28J60_writeByte16(ETXST,TXSTART_INIT);
	    ENC28J60_writeByte16(ETXND,TXSTOP_INIT);

	    //reset con tro RX
	    ENC28J60_writeByte16(ERXRDPT,RXSTART_INIT);
	    ENC28J60_writeByte16(ERXWRPT,RXSTART_INIT);

	    //rx buffer filters
	    //ENC28J60_writeByte(ERXFCON,ERXFCON_UCEN|ERXFCON_ANDOR|ERXFCON_CRCEN); //set 3 bit

	    //cau hinh MAC
	    ENC28J60_writeByte(MACON1,MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	    ENC28J60_writeByte(MACON2,0x00);
	    ENC28J60_write_command(ENC28J60_BIT_FIELD_SET,MACON3,MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	    ENC28J60_writeByte16(MAIPG,0x0C12);
	    ENC28J60_writeByte(MABBIPG,0x12);

	    //Set maximum frame length
	    ENC28J60_writeByte(MAMXFL,1500);

	    //Set Mac addr
	    ENC28J60_writeByte(MAADR5,macaddr[0]);
	    ENC28J60_writeByte(MAADR4,macaddr[1]);
	    ENC28J60_writeByte(MAADR3,macaddr[2]);
	    ENC28J60_writeByte(MAADR2,macaddr[3]);
	    ENC28J60_writeByte(MAADR1,macaddr[4]);
	    ENC28J60_writeByte(MAADR0,macaddr[5]);

	    //read MAC
	    mymac[0]=ENC28J60_readByte(MAADR5);
	    mymac[1]=ENC28J60_readByte(MAADR4);
	    mymac[2]=ENC28J60_readByte(MAADR3);
	    mymac[3]=ENC28J60_readByte(MAADR2);
	    mymac[4]=ENC28J60_readByte(MAADR1);
	    mymac[5]=ENC28J60_readByte(MAADR0);

	    sprintf(debug_string,"\r\nYour MAC addr = %02X:%02X:%02X:%02X:%02X:%02X\r\n",mymac[0],mymac[1],mymac[2],mymac[3],mymac[4],mymac[5]);
	    UART_putString(debug_string);

	    //**********Advanced Initialisations************//
	    ENC28J60_writePhy(PHCON2,PHCON2_HDLDIS);
	    ENC28J60_writePhy(PHLCON,PHLCON_LACFG2|PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|PHLCON_LFRQ0|PHLCON_STRCH);
	    ENC28J60_SetBank (ECON1);
	    ENC28J60_write_command (ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
	    ENC28J60_writeByte(ECOCON,0x02);
	    delay_us(15);
	    ENC28J60_write_command (ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN); // cho phép nhan gói
	 }
}

void ENC28J60_error (void)
{
  UART_putString("Khoi tao module ENC28J60 that bai !\r\n");
}


static void ENC28J60_error (void)
{
  UART_putString("Khoi tao module ENC28J60 that bai !");
}

//--------------------------------------------------
// Send 1 byte data for spi
void SPI_SendByte(uint8_t _byte)
{
 spi(_byte);
}
//--------------------------------------------------
// Recive 1 byte data form spi
uint8_t SPI_ReceiveByte(void)
{
 uint8_t _byte = spi(0xFF);
 return _byte;
}

/*
*	add 3 bits Opcode +  5 bit Argument => 1 byte
*	but before add you shoule be reset 3bit at first for Opcode addres&0x1F
*	0x1F = 0001 11111
*/
void ENC28J60_write_command(uint8_t op,uint8_t addres, uint8_t data)
{
 SS_SELECT();
 SPI_SendByte(op|(addres&0x1F));
 SPI_SendByte(data);
 SS_DESELECT();
}

uint8_t ENC28J60_read_command(uint8_t op,uint8_t addres)
{
 uint8_t result;
 SS_SELECT();
 SPI_SendByte(op|(addres&0x1F));
 if(addres & 0x80) SPI_ReceiveByte();
 result=SPI_ReceiveByte();
 SS_DESELECT();
 return result;
}

/*
* 1. Clear data bank BSEL1 and BSEL0 with method Bit Field Clear
* 2. Set data bank BSEL1 and BSEL0 with menthod Bit Field Set
*/

static uint8_t Enc28j60Bank;
static void ENC28J60_SetBank(uint8_t addres)
{
 if ((addres&0x60)!=Enc28j60Bank) //neu bank hien tai khac bank da cai dat
 {
  ENC28J60_write_command(ENC28J60_BIT_FIELD_CLR,ECON1,ECON1_BSEL1|ECON1_BSEL0);
  Enc28j60Bank = addres&0x60;    //luu lai de lan sau kiem tra
  ENC28J60_write_command(ENC28J60_BIT_FIELD_SET,ECON1,Enc28j60Bank>>5);
 }
}

static void ENC28J60_writeByte(uint8_t addres,uint8_t data)
{
 ENC28J60_SetBank(addres);
 ENC28J60_write_command(ENC28J60_WRITE_CTRL_REG,addres,data);
}
//--------------------------------------------------
static uint8_t ENC28J60_readByte(uint8_t addres)
{
 ENC28J60_SetBank(addres);
 return ENC28J60_read_command(ENC28J60_READ_CTRL_REG,addres);
}

void ENC28J60_writeByte16(uint8_t addres,uint16_t data)
{
 ENC28J60_writeByte(addres, data);
 ENC28J60_writeByte(addres+1, data>>8);
}

/*	3.3 Data sheet
	WRITE PHY REGISTER
		1. write the address of the PHY register to write into the MIREADR
		2. write the lower 8 bits of data to write into the MIWRL
		3. write the upper 8 bits of data to write into the MIWRH
		4. Writting to this register automatically begins the MIIM transaction MISTAT.BUSY is set
		5. after 10.24us writte is complete MISTAT.BUSY auto reset.
*/
static void ENC28J60_writePhy(uint8_t addres,uint16_t data) //ham ghi PHY
{
  ENC28J60_writeByte(MIREGADR, addres);
  ENC28J60_writeByte16(MIWR, data);
  while(ENC28J60_readByte(MISTAT)&MISTAT_BUSY); //cho het ban
}

/*	
	READING PHY REGISTER
		1. write the address of the PHY register to read from into the MIREGADR
		2. Set the MICMD.MIIRD bit. The read opreation begins and MISTAT.BUSY is set
		3. Wait 10.24us. Poll the MISTAT.BUSY bit to be certain that the operation is complete.
		4. Clear the MICMD.MIIRD bit
		5. Read data from the MIRDL and MIRDH register. 
*/
static uint16_t ENC28J60_readPhy(uint8_t addres) //ham doc PHY
{
  uint16_t value;
  ENC28J60_writeByte(MIREGADR, addres);
  ENC28J60_writeByte(MICMD, MICMD_MIIRD);       //set bit MICMD_MIIRD bat dau doc
  while(ENC28J60_readByte(MISTAT)&MISTAT_BUSY); //cho het ban
  ENC28J60_writeByte(MICMD,0x00);               //clear bit MICMD_MIIRD
  value = ENC28J60_readByte(MIRDL) + ((uint16_t)ENC28J60_readByte(MIRDH)<<8) ;  //doc gia tri trong 2 thanh gho MIWR
  return value; //tra ve gia tri
}

//**********Advanced Initialisations************//
ENC28J60_writePhy(PHCON2,PHCON2_HDLDIS);	// using haft-duplex
//configuration led
ENC28J60_writePhy(PHLCON,PHLCON_LACFG2|PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|PHLCON_LFRQ0|PHLCON_STRCH);	
ENC28J60_SetBank (ECON1);

/*	ENABLING RECEPTION
*	Assuming that the receive buffer has been initialized, the MAC has been properly configured and
*	the receive Ethernet packets.
*	1. if an interrupt is desired whenever a packet is received EIE.PKTIE and EIE.INTIE
*	2. If an interrupt is desired whenever a packet is dropped due to insufficient buffer space, clear
*	   EIR.RXERIF and set both EIE.RXERIE and EIE.INTIE
*	3. Enable reception by setting ECON1.RXEN.
*/
ENC28J60_write_command (ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
ENC28J60_writeByte(ECOCON,0x02);
delay_us(15);
ENC28J60_write_command (ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN); // cho phép nhận gói


/* read 1 byte */
uint8_t ENC28J60_read_Byte_Buffer(void)
{
   return ENC28J60_read_command(ENC28J60_READ_BUF_MEM,0);
}

/* 	read more than 1 byte 
*	1. CS Enable
*	2. Opcode Read Buffer Menmory
*	3. while read data from spi protocol
*	4. CS Disable
*/
void ENC28J60_ReadBuffer(uint16_t len, uint8_t* data)
{
  SS_SELECT();
  SPI_SendByte(ENC28J60_READ_BUF_MEM); //gui Opcode
  while(len)
  {
        len--;
        // read data
        *data = (uint8_t)SPI_ReceiveByte();
        data++;
  }
 *data='\0'; //set end buffer
 SS_DESELECT();
}

/*
* 	1. Set pointer read ERDPT at the first of the packet
*	2. Read 2 byte head in the buffer. This is start of the next packet and finish of the current packet
*	3. Read 2 byte next in the buffer. This is the lenght of the packet [Status vector 0:15]
*	4. Read 2 byte next in the buffer. This is the infor receive of the packet [Status vector 16:31]
*	5. At this point is the data packet, you can minus 4byte of chusum if you no need use.
*	6. Set ERXRDP for the next packet
*	7. after that set bit ECON2.PKTDEC for purpose EPKTCNT decrease 1
*/
uint16_t ENC28J60_read_packet(uint8_t *buf,uint16_t buflen)
{
  uint16_t status;
  uint16_t len=0;
  if(ENC28J60_readByte(EPKTCNT)>0)  // neu co goi tin
  {
       //Thiet lap con tro de doc du lieu tu goi tin nhan duoc
      ENC28J60_writeByte16(ERDPT,NextPacketPtr);

      //Doc gia tri con tro cua goi tin tiep theo
      NextPacketPtr=ENC28J60_read_Byte_Buffer();
      NextPacketPtr|=ENC28J60_read_Byte_Buffer()<8;

      // Doc kich thuoc cua goi tin
      len  = ENC28J60_read_Byte_Buffer();
	  len |= ENC28J60_read_Byte_Buffer()<<8;

      len-=4; // xoa 4byte checksum o cuoi
      if(len>buflen) len=buflen;

      //Doc trang thai cua bo nhan
      status  = ENC28J60_read_Byte_Buffer();
	  status |= ENC28J60_read_Byte_Buffer()<<8;

      if ((status & 0x80)==0)len=0; //kiem tra bit Received Ok, neu err thi khong doc goi nay
      else
      {
        //doc du lieu trong bo dem
        ENC28J60_ReadBuffer(len, buf);
      }

      // Chuyen con tro du lieu nhan toi phan dau cua goi tin tiep theo.
      if(NextPacketPtr-1>RXSTOP_INIT)ENC28J60_writeByte16(ERXRDPT,RXSTOP_INIT);
      else ENC28J60_writeByte16(ERXRDPT,NextPacketPtr);
      ENC28J60_write_command(ENC28J60_BIT_FIELD_SET,ECON2,ECON2_PKTDEC);
  }
  return len;
}


static void ENC28J60_writeBuf(uint16_t len,uint8_t* data)
{
  SS_SELECT();
  SPI_SendByte(ENC28J60_WRITE_BUF_MEM);
  while(len--)
  SPI_SendByte(*data++);
  SS_DESELECT();
}

/*
* 	1. Check ECON1.TXRTS reset is ready trasmit
*	2. EWRPT 16 bit is address save trasmit buffer
*	3. ETXND 186 bit is address byte tail in buffer
*	4. transmit byte conntrol  = 0x00 in buffer
*	5. transmit data packetfor chip
*	6. set ECON1.TXRTS for start purpose trasmit
*/
void ENC28J60_send_packet(uint8_t *buf,uint16_t buflen)
{
   //cho qua trinh truyen truoc do hoan tat
   while(ENC28J60_readByte(ECON1)&ECON1_TXRTS);

   // Thiet lap con tro bat dau khong gian bo dem truyen
   ENC28J60_writeByte16(EWRPT,TXSTART_INIT);

   //Thiet lap con tro toi vi tri cua byte cuoi cua goi tin
   ENC28J60_writeByte16(ETXND,TXSTART_INIT+buflen);

   //truyen byte dau tien la 0x00 vao bo dem
   ENC28J60_write_command(ENC28J60_WRITE_BUF_MEM,0,0);

   //truyen goi tin vao bo dem
   ENC28J60_writeBuf(buflen,buf);

   //set bit TXRTS cho enc28j60 truyen goi tin
   ENC28J60_write_command(ENC28J60_BIT_FIELD_SET,ECON1,ECON1_TXRTS);
}

//--------------------------------------------------------------------------
//End file