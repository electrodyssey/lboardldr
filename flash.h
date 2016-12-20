#ifndef FLASH_H_
#define FLASH_H_


#define CONFIG_SIZE_MAX  350000
#define CONFIG_OFFSET 0x10000

void uint32_to_char (uint32_t u32, unsigned char * pbuf);
uint32_t char_to_uint32 (unsigned char * pbuf);



void flash_init ();
void flash_format ();
void flash_page_write (uint32_t *pos, unsigned char *buf, uint8_t len);
void flash_read (uint32_t addr, unsigned char *buf, uint8_t len);
uint32_t get_config_size ();
void set_config_size (uint32_t size);
void set_config_size_t (uint32_t size);


#endif
