#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "m25p16.h"
#include "flash.h"

#define FILE_OFFSET 0x10000


/* 
   Serializes uint32_t into a char buffer (big endian). Buffer must have enough space (4 bytes) for conversion
   in case of m25p16/m25p32, only three bytes are needed: b1, b2 and b3 (b0 will contain 0x0)
*/
void uint32_to_char (uint32_t u32, unsigned char * pbuf)
{
  pbuf [0] = (0xFF000000 & u32) >> 24;
  pbuf [1] = (0xFF0000 & u32) >> 16;
  pbuf [2] = (0xFF00 & u32) >> 8;
  pbuf [3] = 0xFF & u32;
}


/* Deserializes char buffer into uint32_t */
uint32_t char_to_uint32 (unsigned char * pbuf)
{
  uint32_t u;

  u = (uint32_t)pbuf [0] << 24 | (uint32_t)pbuf [1] << 16 | (uint32_t)pbuf [2] << 8 | pbuf[3];
  return  u;
}



void flash_init ()
{
  m25p16_init();
}


void flash_format ()
{
  m25p16_init();
  m25p16_write_enable();
  
  m25p16_bulk_erase();
  m25p16_wait_for_operation();
  
  m25p16_write_disable();
}


/* config file size is stored at @0x0 */
uint32_t get_config_size ()
{
  unsigned char buf[4];
   uint32_t size;

  flash_init();
  
  m25p16_read_begin(0, 0, 0);

  buf[0] = m25p16_read_byte();
  buf[1] = m25p16_read_byte();
  buf[2] = m25p16_read_byte();
  buf[3] = m25p16_read_byte();

  m25p16_read_end();

  size = char_to_uint32 (buf);

  return size;
}


void set_config_size (uint32_t size)
{
  unsigned char tbuf[4];

  
  /*convert file size to char*/
  uint32_to_char (size, tbuf);

  flash_init();

  m25p16_write_enable();
    
  /*write file size*/
  m25p16_write_begin(0, 0, 0);
  
  m25p16_write_byte(tbuf[0]);
  m25p16_write_byte(tbuf[1]);
  m25p16_write_byte(tbuf[2]);
  m25p16_write_byte(tbuf[3]);

  m25p16_write_end();
  m25p16_wait_for_operation();

  m25p16_write_disable();
}





/*
  set *pos to 0 before the first call 
  addr a special care must be taken in order not to overwrite the page boundary
  buf must be a 256 bytes or longer
  
*/
void flash_page_write (uint32_t *pos, unsigned char *buf, uint8_t len)
{
  unsigned char addr_buf [4];
  uint8_t i;

  m25p16_init();
  m25p16_write_enable();

  uint32_to_char (*pos + CONFIG_OFFSET, addr_buf);
  m25p16_write_begin(addr_buf[1], addr_buf[2], addr_buf[3]);

  for (i = 0; i < len; i++)
    {
      m25p16_write_byte(buf[i]);
    }

  m25p16_write_end();
  m25p16_wait_for_operation();
  
  
  m25p16_write_disable();

  *pos = *pos + len;
}


/*
  buf size must not exceed 256 bytes
*/
void flash_read (uint32_t addr, unsigned char *buf, uint8_t len)
{
  unsigned char addr_buf [4];
  uint8_t i;

  uint32_to_char (addr, addr_buf);
  
  m25p16_init();
  
  m25p16_read_begin(addr_buf[1], addr_buf[2], addr_buf[3]);
  for (i = 0; i < len; i++)
    {
      buf[i] = m25p16_read_byte();
    }
  

  m25p16_read_end();
}
