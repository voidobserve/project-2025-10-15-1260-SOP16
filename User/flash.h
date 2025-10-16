#ifndef __FLASH_H
#define __FLASH_H

#include "my_config.h"
#include "include.h"

#define FLASH_START_ADDR            (0x00)          // 起始地址128byte对齐
// #define FLASH_DATA_LEN              (128)

void flash_erase_sector(u8 addr);
void flash_program(u8 addr, u8 *p_data, u8 len);
void flash_read(u8 addr, u8 *p_data, u8 len);

 

#endif 