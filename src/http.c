/*	HTTP:: HyperText Transfer Protocol
	
	ETHERNET FRAME
	Ethernet type II	IP 			TCP		HTTP
	14 byte				20 byte		length	length

	HTTP Response
		HTTP/1.1 200 OK
			1.1 revison of HTTP
			200	Status Code hint receive sucessful.
		Content-Type: text/html
		status line is finish with method \r\n\r\n

	PROCESS
		1. Client (Browes) send packet connect SYN to server, server response and finish handshake
		2. Client send http request to srever PSH|ACK
		3. Server receive http and feedback http, http is allocated FLASH
		4. After disconnect 

	Send data when client GET
		Because header start 0 -> IP->Urgent_Pionter is sizeof 54 byte
		The buffer declear 512 byte -> 512 - 54 = 458 byte for the data http.
		If data > 458 byte you must be seprate packet
*/

#include "http.h"

extern const uint8_t macaddr[6];
extern const uint8_t ip[4];
extern uint8_t debug_string[60];




//--------------------------------------------------