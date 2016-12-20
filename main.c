// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* File              : xmodem.c
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
* Description       : Xmodem main routine
*
****************************************************************************/

/***************************************************************************
* modified for atmega328p and AVRGCC
* renamed to main.c
* uart_print procedures are copied from hekilledmywire.wordpress.com
* spi and m25p16 are from https://github.com/gheja/avr
***************************************************************************/

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "xmodem.h"
#include "flash.h"
#include "m25p16.h"
#include "usart_print.h"


volatile unsigned char buf[133];
//unsigned char rbuf[128];

struct global
{
  volatile unsigned char *recv_ptr;
  volatile unsigned char buffer_status;
  volatile unsigned char recv_error;
  volatile unsigned char t1_timed_out;
} gl;



void print_menu ()
{
  char s [128];
  uint32_t csize;


  
  usart_print_putstring ("\r\n*** lboardloader *** \r\n");


  sprintf (s, "*** config offset: 0x%.4lx\r\n", CONFIG_OFFSET);
  usart_print_putstring (s);

  csize = get_config_size();

  sprintf (s, "*** config size: %lu \r\n", csize);
  usart_print_putstring (s);

  usart_print_putstring ("r          reboot the FPGA chip\r\n");
  usart_print_putstring ("d          dump the first 2K of the stored file onto the screen\r\n");
  usart_print_putstring ("x          xmodem load\r\n");
  usart_print_putstring ("E          Erase the flash\r\n");
  //usart_print_putstring ("t          test\r\n");
  
  usart_print_putchar ('>');
}


void pin_test()
{
  int i = 10;
  while (i-- > 0)
    {

      PORTD |= (_BV(PD2));
      PORTD |= (_BV(PD3));
      //CCLCK
      PORTD |= (_BV(PD4));
      //CPROGB
      PORTD |= (_BV(PD5));
      PORTC |= (_BV(PC0));
      PORTC |= (_BV(PC1));
      PORTC |= (_BV(PC2));
      PORTC |= (_BV(PC3));
      PORTC |= (_BV(PC4));
      PORTC |= (_BV(PC5));


      //unset data lines
      PORTD &= ~(_BV(PD2));
      PORTD &= ~(_BV(PD3));
      //CCLCK
      PORTD &= ~(_BV(PD4));
      //CPROGB
      PORTD &= ~(_BV(PD5));
      PORTC &= ~(_BV(PC0));
      PORTC &= ~(_BV(PC1));
      PORTC &= ~(_BV(PC2));
      PORTC &= ~(_BV(PC3));
      PORTC &= ~(_BV(PC4));
      PORTC &= ~(_BV(PC5));
    }
}

void boot_fpga()
{
  unsigned char c;
  uint32_t i;
  uint8_t t = 0;
  uint32_t csize;
  unsigned char tbuf [4];

  

  usart_print_putstring ("attempting fpga configuration\r\n");

  //set CCLCK high (debug)
  //  PORTD |= _BV(PD4);


  //unset CCLCK
  PORTD &= ~(_BV(PD4));

  usart_print_putstring ("CPROGB reset..\r\n");
  //unset CPROGB to perform an async reset of FPGA
  PORTD &= ~(_BV(PD5));
  _delay_ms (10);
  PORTD |= _BV(PD5);


  usart_print_putstring ("CINITB..\r\n");
  //wait until CINITB goes high
  for (i = 0; i < 1000; i++)
    {
      _delay_ms(1);
      
      if (PIND & _BV(PD7))
	break;
    }
  
  csize = get_config_size();

  if (0 == csize || 0xFFFFFFFF == csize)
    {
      usart_print_putstring ("Wrong file size. Skipping config.\r\n");
      return;
    }
  

  uint32_to_char (CONFIG_OFFSET, tbuf);
  
  m25p16_read_fast_begin(tbuf[1], tbuf[2], tbuf[3]);


  c = 0xFF;

  usart_print_putstring ("Select-Map config..\r\n");
  for (i = 0; i < csize; i++)
    {
      //unset CCLK
      PORTD &= ~(_BV(PD4));
      PORTB &= ~(_BV(PB0));

      //zero t
      t = 0;


      //read byte from the flash
      SPDR = 0x0;

      // wait until transmission complete
      while (!(SPSR & (1<<SPIF)))
	{
	}
      
      
      c = SPDR;

      //bit 0 - toogle bit 7 - PD3
      if ((c) & 1)
	PORTD |= (_BV(PD3));
      else
	PORTD &= ~(_BV(PD3));


      //bit 1 - toogle bit 6 - PD2
      if ((c >> 1) & 1)
	PORTD |= (_BV(PD2));
      else
	 PORTD &= ~(_BV(PD2));

      //bit 2 - toogle bit 5 - PC5
      if ((c >> 2) & 1)
	t |= (_BV(PC5));
      

      //bit 3 - toogle bit 4 - PC4
      if ((c >> 3) & 1)
	t |= (_BV(PC4));


      //bit 4 - toogle bit 3 - PC3
      if ((c >> 4) & 1)
	t |= (_BV(PC3));



      //bit 5 - toogle bit 2 - PC2
      if ((c >> 5) & 1)
	t |= (_BV(PC2));


      //bit 6 - toogle bit 1 - PC1
      if ((c >> 6) & 1)
	t |= (_BV(PC1));



      //bit 7 - toogle bit 0 - PC0
      if ((c >> 7) & 1)
	t |= (_BV(PC0));

      //assign PORTC
      PORTC = t;

      


      //set CCLK
      PORTD |= _BV(PD4);

      //LED
      PORTB |= _BV(PB0);

    }
  
  
  m25p16_read_end();

  //unset CCLK
  PORTD &= ~(_BV(4));

  usart_print_putstring ("Select-Map config end..\r\n");

  //strobe CCLK until CDONE is 1
  for (i = 0; i < 2048; i++)
    {
      PORTD |= _BV(4);
      
      PORTD &= ~(_BV(4));
      
      if (PINB & _BV(PD6))
	break;
    }
  
  //8 CCLK strobes to finish the configuration
  for (i = 0; i < 9; i++)
    {
      PORTD |= _BV(4);
      
      PORTD &= ~(_BV(4));
    }
	 

  //check if CDONE is set
  if (PINB & _BV(PD6))
    {
      PORTB |= _BV(PB0);
    }

  usart_print_putstring ("boot sequence end..\r\n");
}





//__C_task void main(void)
int main (void)
{
  //CLKPR = (1 << CLKPCE);  // enable prescaler update
  //CLKPR = 0;              // set maximum clock frequency

  //CBUSY is input
  DDRB &= ~(_BV(DDB1));

  //CDONE
  DDRD &= ~(_BV(DDD6));

  //CINITB
  DDRD &= ~(_BV(DDD7));  
  
  
  DDRC = (1<<DDC0) | (1<<DDC1) | (1<<DDC2) | (1<<DDC3) | (1<<DDC4) | (1<<DDC5) | (0<<DDC6);
  DDRD = (0<<DDD0) | (1<<DDD1) | (1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5);

  init();     // low level hardware initialization

  unsigned char r = 0;
  uint32_t i = 0;
  unsigned char tbuf[4];

  print_menu ();
  //attempt fpga boot
  boot_fpga();
  
  for (;;)
  {
    print_menu ();

    r = usart_print_getchar();

    switch (r)
      {
      case '\r':
	break;
      case '\n':
	break;

    case 'r':
      boot_fpga();
      break;
	
    case 'x':
      m25p16_init();
      usart_print_putstring ("erasing the flash and sleeping for a 10 seconds before starting the xmodem (around 30 sec of idle time)\r\n");
      _delay_ms(10000);
      sei();
      receive(&buf[0]);
      cli();
      break;

      case 'd':
      usart_print_putstring ("dump config file..\r\n");
      flash_init();

      uint32_to_char (CONFIG_OFFSET, tbuf);
      
      m25p16_read_fast_begin(tbuf[1], tbuf[2], tbuf[3]);
      for (i = 0; i < 2048; i++)
	{
	  usart_print_putchar(m25p16_read_byte());
	}
  
      
      m25p16_read_end();


      
      usart_print_putstring ("\r\n\r\n");
      break;

      case 'D':
      usart_print_putstring ("dump 0x0..\r\n");
      flash_init();

      m25p16_read_fast_begin(0, 0, 0);
      for (i = 0; i < 2048; i++)
	{
	  usart_print_putchar(m25p16_read_byte());
	}
  
      
      m25p16_read_end();
      
      usart_print_putstring ("\r\n\r\n");
      break;

    case 'E':
      usart_print_putstring ("erasing the flash \r\n");
      flash_init();
      flash_format();
      break;

    case 't':
      pin_test();

    default:
      usart_print_putstring ("\r\nunknown command: ");
      usart_print_putchar(r);
      usart_print_putstring ("\r\n");
      
    }
  }

  return 0;
} // main
