#include "flash.h"

#if 0
    // 扇区擦除
    flash_erase_sector(FLASH_START_ADDR);
    // 写入数据
    flash_program(FLASH_START_ADDR, tx_buf, sizeof(tx_buf));
    // 读取数据
    flash_read(FLASH_START_ADDR, rx_buf, sizeof(rx_buf));
#endif

/**
 * @brief  flash erase sector
 * @param  addr : sector address in flash
 * @retval None
 */
void flash_erase_sector(u8 addr)
{
    FLASH_ADDR = 0x3F;
    FLASH_ADDR = addr;
    FLASH_PASSWORD = FLASH_PASSWORD(0xB9); // 写入操作密码
    FLASH_CON = FLASH_SER_TRG(0x1);        // 触发扇区擦除

    while (!(FLASH_STA & FLASH_SER_FLG(0x1)))
        ; // 等待扇区擦除空闲
}

/**
 * @brief  flash program
 * @param  addr   : Write data address in flash
 * @param  p_data : Write data to flash
 * @param  len    : Data length
 * @retval None
 */
void flash_program(u8 addr, u8 *p_data, u8 len)
{
    FLASH_ADDR = 0x3F;
    FLASH_ADDR = addr;

    while (len >= 1)
    {
        while (!(FLASH_STA & FLASH_PROG_FLG(0x1)))
            ; // 等待烧录空闲
        FLASH_DATA = *(p_data++);
        FLASH_PASSWORD = FLASH_PASSWORD(0xB9); // 写入操作密码
        FLASH_CON = FLASH_PROG_TRG(0x1);       // 触发烧录

        len -= 1;
    }
}

/**
 * @brief  flash program
 * @param  addr   : Read data address in flash
 * @param  p_data : Read data to flash
 * @param  len    : Data length
 * @retval None
 */
void flash_read(u8 addr, u8 *p_data, u8 len)
{
    while (len != 0)
    {
        *(p_data++) = *((u8 code *)(0x3F00 + addr++));
        len--;
    }
}
