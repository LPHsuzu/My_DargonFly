#ifndef __FLASH_H
#define __FLASH_H
#include "stm32f4xx.h"

#define STM32_FLASH_BASE 0x08000000 // STM32 FLASH的起始地址

// FLASH 扇区的起始地址
#define ADDR_FLASH_SECTOR_0 ((u32)0x08000000) // 扇区0起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_1 ((u32)0x08004000) // 扇区1起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_2 ((u32)0x08008000) // 扇区2起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_3 ((u32)0x0800C000) // 扇区3起始地址, 16 Kbytes
#define ADDR_FLASH_SECTOR_4 ((u32)0x08010000) // 扇区4起始地址, 64 Kbytes
#define ADDR_FLASH_SECTOR_5 ((u32)0x08020000) // 扇区5起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_6 ((u32)0x08040000) // 扇区6起始地址, 128 Kbytes
#define ADDR_FLASH_SECTOR_7 ((u32)0x08060000) // 扇区7起始地址, 128 Kbytes

#define FLASH_SAVE_ADDR 0X08060000 // 设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区。否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.

uint32_t STMFLASH_ReadWord(uint32_t faddr);
void STMFLASH_Write(uint32_t WriteAddr, uint32_t *pBuffer, uint32_t NumToWrite);
void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead);

#endif
