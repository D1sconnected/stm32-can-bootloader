// Flash.h
#ifndef FLASH_HEADER_FILE_H
#define FLASH_HEADER_FILE_H

#include "main.h"

extern uint8_t gIsErased;
extern uint8_t gIsCorrupted;

#define FLASH_COUNTDOWN_TO_START_MS     500 // ToDo: not ms

#if defined (STM32F103xB)
#define FLASH_USER_START_ADDR           0x08007000 // Start of User Firmware: VARS + DATA + FW
#define FLASH_FW_START_ADDR             0x08007000
#define FLASH_VASR_START_ADDR           0x0800F800
#define FLASH_CRC_START_ADDR            0x0800FC00

#define FLASH_BOOTLOADER_START_ADDR     0x08000000
#define FLASH_TOTAL_SIZE_IN_BYTES       65536

#define FLASH_STACKPOINTER              0x20005000
#define FLASH_TIMESTAMP_PAGE            34
#define FLASH_CRC_PAGE            		  35 // - bootloader pages

#define FLASH_TIMESTAMP_SIZE_IN_DWORDS       6
#define FLASH_CRC_SIZE_IN_DWORDS             48

#define FLASH_BOOTLOADER_SIZE_IN_BYTES  (FLASH_USER_START_ADDR - FLASH_BOOTLOADER_START_ADDR)
#define FLASH_USER_APP_SIZE_IN_BYTES    (FLASH_TOTAL_SIZE_IN_BYTES - FLASH_BOOTLOADER_SIZE_IN_BYTES)


#define PAGE_SIZE_IN_BYTES              1024
#define PAGE_SIZE_IN_DWORDS             256
#endif

#if defined (STM32F407xx)
#define FLASH_USER_START_ADDR           0x08008000 // Start of User Firmware: VARS + DATA + FW
#define FLASH_FW_START_ADDR             0x08008000
#define FLASH_VASR_START_ADDR           0x0801F800
#define FLASH_CRC_START_ADDR            0x0801FC00

#define FLASH_BOOTLOADER_START_ADDR     0x08000000
#define FLASH_TOTAL_SIZE_IN_BYTES       524288

#define FLASH_STACKPOINTER              0x20020000
#define FLASH_TIMESTAMP_PAGE            94
#define FLASH_CRC_PAGE                  95 // - bootloader pages

#define FLASH_TIMESTAMP_SIZE_IN_DWORDS       6
#define FLASH_CRC_SIZE_IN_DWORDS             48

#define FLASH_BOOTLOADER_SIZE_IN_BYTES  (FLASH_USER_START_ADDR - FLASH_BOOTLOADER_START_ADDR)
#define FLASH_USER_APP_SIZE_IN_BYTES    (FLASH_TOTAL_SIZE_IN_BYTES - FLASH_BOOTLOADER_SIZE_IN_BYTES)


#define PAGE_SIZE_IN_BYTES              1024
#define PAGE_SIZE_IN_DWORDS             256

#define ADDR_FLASH_SECTOR_0             ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1             ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2             ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3             ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4             ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5             ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6             ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7             ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8             ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9             ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10            ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11            ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
#endif

#define MAX_FW_SIZE_IN_PAGES            FLASH_USER_APP_SIZE_IN_BYTES / PAGE_SIZE_IN_BYTES
#define MAX_FW_SIZE_IN_BYTES            FLASH_TOTAL_SIZE_IN_BYTES - FLASH_BOOTLOADER_SIZE_IN_BYTES
#define MAX_FW_SIZE_IN_DWORDS           MAX_FW_SIZE_IN_BYTES / 4

typedef enum
{
    FLASH_OK           			= 0,
    FLASH_INIT_FAILED   		= 4,
    FLASH_DEINIT_FAILED 		= 5,
    FLASH_ERASE_FAILED  		= 6,
    FLASH_READ_FAILED   		= 7,
    FLASH_WRITE_FAILED  		= 8,
    FLASH_NO_FIRMWARE   		= 9,
    FLASH_INVALID_ARGS  		= 10,
	  FLASH_NOT_CHANGED   		= 11,
	  FLASH_FIRMWARE_CORRUPTED 	= 12,
} FlashStatus;

typedef enum
{
  FLASH_COMPUTE_CRC             = 0,
  FLASH_VERIFY_CRC              = 1,
} FlashCrcMode;

int Flash_Init(void);
int Flash_DeInit(void);
int Flash_Erase(uint8_t pageNumber);
int Flash_Read(uint32_t offset, uint32_t *pData);
#if defined (STM32FG0xx)
int Flash_Write(uint32_t offset, uint64_t data);
#else
int Flash_Write(uint32_t offset, uint32_t data);
#endif
int Flash_CheckFwPresented(void);
void Flash_JumpToApp(uint32_t appAddr);
int Flash_ProcessAppCrc(uint16_t pages, uint16_t *pCrc, FlashCrcMode mode);
int Flash_ReadStringFromMem(char *pBuf, size_t bufSize, uint32_t offset);
int Flash_EraseApp();
int Flash_UpdateChunk (uint8_t *pData, size_t size, uint8_t page);

#if defined (STM32F407xx)
uint32_t Flash_GetSectorSize(uint32_t Sector);
uint32_t Flash_GetSector(uint32_t Address);
#endif

#endif
