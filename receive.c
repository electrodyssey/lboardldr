// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : receive.c
* Compiler          : IAR EWAAVR 4.11a
* Revision          : $Revision: 2.0 $
* Date              : $Date: 23. august 2005 15:57:40 $
* Updated by        : $Author: jtyssoe $
*
* Support mail      : avr@atmel.com
*
* Supported devices : ATmega48/88/168
*
* AppNote           : AVR350: XmodemCRC Receive Utility for AVR
*                     Migrated to mega48 and corrected several bugs in original AT90S8515 code
*
* Description       : Receive function
*
****************************************************************************/

/***************************************************************************
* modified for atmega328p and AVRGCC
***************************************************************************/
#include <avr/pgmspace.h>
#include <stdio.h>
#include "m25p16.h"
#include "xmodem.h"
#include "flash.h"

extern struct global
{
  volatile unsigned char *recv_ptr;
  volatile unsigned char buffer_status;
  volatile unsigned char recv_error;
  volatile unsigned char t1_timed_out;
} gl;


void receive(volatile unsigned char *bufptr1)
{
  unsigned char packet = 0;    // status flag
  unsigned char packet_number; // represents 'last successfully received packet'
  uint32_t flash_addr = 0;    //address of data in a flash mem

  uint8_t config_end = 0;


  //erase the flash
  flash_init();
  flash_format();

  packet_number = 0x00;        // initialise to first xmodem packet number - 1
  gl.recv_ptr = bufptr1;       // point to recv buffer

  sendc();                     // send a 'c' until the buffer gets full


  flash_addr = 0;
    
  while (packet != end)        // get remainder of file
  {
    recv_wait();               // wait for error or buffer full
    packet = validate_packet((unsigned char*)bufptr1,&packet_number);  // validate the packet
    gl.recv_ptr = bufptr1;     // re-initialize buffer pointer before acknowledging
    switch(packet)
    {
      case good:
        // insert a data handler here,
        // for example, write buffer to a flash device
	
	flash_page_write (&flash_addr, (unsigned char*)bufptr1 + 3, 128);
	
	//increment flash_addr
	//flash_addr += 128;
	
        break;
      case dup:
        // a counter for duplicate packets could be added here, to enable a
        // for example, exit gracefully if too many consecutive duplicates,
        // otherwise do nothing, we will just ack this
        break;
      case end:
	//config file size should be written
	config_end = 1;
        break;
      default:
        // bad, timeout or error -
        // if required, insert an error handler of some description,
        // for example, exit gracefully if too many errors
        break; // statement just to eliminate compiler warning
    }
    respond(packet);                  // ack or nak
  }// end of file transmission


  if (config_end || 0 < flash_addr)
    {
      set_config_size(flash_addr);
    }
  
}
