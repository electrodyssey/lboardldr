#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include "usart_print.h"

void usart_print_putchar( unsigned char data)
{
 while(!(UCSR0A & (1<<UDRE0)));
 UDR0 = data;
}

unsigned char usart_print_getchar(void)
{
 while(!(UCSR0A & (1<<RXC0)));
 return UDR0;
}

void usart_print_putstring(char* StringPtr)
{
  while(*StringPtr != 0x00)
  {
    //Here we check if there is still more chars to send,
    //this is done checking the actual char and see if it is different from the null char
    usart_print_putchar(*StringPtr);
    StringPtr++;
  }
}

/*
  copied from jnk0le softwre usart lib
*/

/*
void HexDump16(uint8_t *bufptr, uint16_t ByteCount)
{
  uint16_t i;
  char buff[17];
  buff[16] = 0;
  
  // Process every byte in the data.
  for (i = 0; i < ByteCount; i++)
    {
      // Multiple of 16 means new line (with line offset).
      
      if ((i % 16) == 0)
	{
	  // Just don't print ASCII for the zeroth line.
	  if (i != 0)
	    {
	      uart_puts("  ");
	      uart_puts(buff);
	      uart_puts("\r\n");
	    }
	  
	  // Output the offset.
	  uart_puts("  ");
	  
	  uart_puthex(i>>2);
	  uart_puthex(i);
	  
	  uart_putc(' ');
        }
      
      // Now the hex code for the specific character.
      uart_putc(' ');
      
      uart_puthex(bufptr[i]);
      
      // And store a printable ASCII character for later.
      if ((bufptr[i] < 0x20) || (bufptr[i] > 0x7e))
	buff[i % 16] = '.';
      else
	buff[i % 16] = bufptr[i];
    }
  
  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0)
    {
      buff[i % 16] = 0;
      uart_puts("   ");
      i++;
    }
  
  // And print the final ASCII bit.
  uart_puts("  ");
  uart_puts(buff);
  uart_puts("\r\n");
}
*/
