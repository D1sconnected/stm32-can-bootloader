#include "flash.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")
uint8_t gIsErased = 0;     // Flag to prevent double erase, and detect erase/update event
uint8_t gIsCorrupted = 0;  // Flag to prevent loading/updating, if something happend during usage of flash

int Flash_Init(void)
{
    HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();
    if(halStatus != HAL_OK)
    {
        return FLASH_INIT_FAILED;
    }
    return FLASH_OK;
}

int Flash_DeInit(void)
{
    HAL_StatusTypeDef halStatus = HAL_FLASH_Lock();
    if(halStatus != HAL_OK)
    {
        return FLASH_DEINIT_FAILED;
    }
    return FLASH_OK;
}

int Flash_Erase(uint8_t pageNumber)
{
    if (pageNumber >= MAX_FW_SIZE_IN_PAGES)
    {
        return FLASH_INVALID_ARGS;
    }

    HAL_StatusTypeDef status;
    uint32_t pageError;

    FLASH_EraseInitTypeDef eraseInit;

#if defined (STM32FG0xx)
    eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
    eraseInit.Page = pageNumber + FLASH_USER_START_ADDR/FLASH_PAGE_SIZE;
    eraseInit.NbPages     = 1;
#elif defined (STM32F103xB)
    eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
    eraseInit.PageAddress = pageNumber * FLASH_PAGE_SIZE + FLASH_USER_START_ADDR;
    eraseInit.NbPages     = 1;
#endif


    status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
    if(status != HAL_OK)
    {
        return FLASH_ERASE_FAILED;
    }

    return FLASH_OK;
}

int Flash_Read(uint32_t offset, uint32_t *pData)
{
    if (pData == NULL)
    {
        return FLASH_INVALID_ARGS;
    }

    if (offset >= MAX_FW_SIZE_IN_BYTES)
    {
        return FLASH_INVALID_ARGS;
    }

    *pData = *((volatile uint32_t*)(FLASH_USER_START_ADDR + offset));
    return FLASH_OK;
}

#if defined (STM32FG0xx)
int Flash_Write(uint32_t offset, uint64_t data)
{
    if (offset >= MAX_FW_SIZE_IN_BYTES)
    {
        return INVALID_PARAMETERS;
    }

    uint32_t address = FLASH_USER_START_ADDR + offset;

    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    if(halStatus != HAL_OK)
    {
        return FLASH_WRITE_FAILED;
    }

    return FLASH_OK;
}
#else
int Flash_Write(uint32_t offset, uint32_t data)
{
    if (offset >= MAX_FW_SIZE_IN_BYTES)
    {
        return FLASH_INVALID_ARGS;
    }

    uint32_t address = FLASH_USER_START_ADDR + offset;

    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
    if(halStatus != HAL_OK)
    {
        return FLASH_WRITE_FAILED;
    }

    return FLASH_OK;
}
#endif


int Flash_CheckFwPresented(void)
{
    // Check that stackPointer presented in memory
    uint32_t stackPointer;
    FlashStatus status = Flash_Read(FLASH_FW_START_ADDR - FLASH_USER_START_ADDR, &stackPointer);
    if(status != FLASH_OK)
    {
        return status;
    }

    if (stackPointer != FLASH_STACKPOINTER)
    {
        return FLASH_NO_FIRMWARE;
    }

    // Check that 6-dword timestamp presented in memory
    for (int i = 0; i < 6; i++)
    {
        uint32_t timeStampDword = 0;
        status = Flash_Read(FLASH_VASR_START_ADDR - FLASH_USER_START_ADDR + i*4, &timeStampDword);
        if(status != FLASH_OK)
        {
            return status;
        }

        if (timeStampDword == 0xFFFFFFFF)
        {
            return FLASH_NO_FIRMWARE;
        }
    }
    return status;
}

//void Flash_JumpToApp(uint32_t appAddr)
//{
//
//    uint32_t appJumpAddress;
//    void (*GoToApp)(void);
//
//    __disable_irq(); // will be enabled in main application
//
//    // Stop SysTick
//	uint32_t temp = SysTick->CTRL;
//	temp &= ~0x02;
//	SysTick->CTRL = temp;
//
//	/* Clear Interrupt Enable Register & Interrupt Pending Register */
//	for (int i=0;i<5;i++)
//	{
//		NVIC->ICER[i]=0xFFFFFFFF;
//		NVIC->ICPR[i]=0xFFFFFFFF;
//	}
//
//    // Disable RCC & HAL
//    LL_RCC_DeInit();
//    HAL_DeInit();
//
//    SCB->VTOR = appAddr;
//    __set_MSP(*((volatile uint32_t*) appAddr));
//    __set_PSP(*((volatile uint32_t*) appAddr));
//
//    appJumpAddress = *((volatile uint32_t*)(appAddr + 4));
//    GoToApp = (void (*)(void))appJumpAddress;
//    GoToApp();
//}

//void Flash_JumpToApp(uint32_t appAddr)
//{
//    uint32_t appJumpAddress;
//    void (*GoToApp)(void);
//
//    __disable_irq(); // will be enabled in main application
//
////    // Disable SysTick
////    SysTick->CTRL = 0;
//
//        /* Clear Interrupt Enable Register & Interrupt Pending Register */
//        for (int i=0;i<5;i++)
//        {
//            NVIC->ICER[i]=0xFFFFFFFF;
//            NVIC->ICPR[i]=0xFFFFFFFF;
//        }
//
////        stm32_systick_disable();
//        uint32_t temp = SysTick->CTRL;
//        temp &= ~0x02;
//        SysTick->CTRL = temp;
//
//    // Disable RCC & HAL
//    LL_RCC_DeInit();
//    HAL_DeInit();
//
////    __disable_irq(); // will be enabled in main application
//
////    /* Clear Interrupt Enable Register & Interrupt Pending Register */
////    for (int i=0;i<5;i++)
////    {
////        NVIC->ICER[i]=0xFFFFFFFF;
////        NVIC->ICPR[i]=0xFFFFFFFF;
////    }
//
//    SCB->VTOR = appAddr;
//    __set_MSP(*((volatile uint32_t*) appAddr));
//    __set_PSP(*((volatile uint32_t*) appAddr));
//
//    appJumpAddress = *((volatile uint32_t*)(appAddr + 4));
//    GoToApp = (void (*)(void))appJumpAddress;
//    GoToApp();
//}

void Flash_JumpToApp(uint32_t appAddr)
{
    uint32_t appJumpAddress;
    void (*GoToApp)(void);

    // Disable RCC
    LL_RCC_DeInit();
    HAL_DeInit();

    // Disable systick timer and reset it to default values
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Disable all interrupts
    __disable_irq();

	/* Clear Interrupt Enable Register & Interrupt Pending Register */
	for (int i=0;i<7;i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}

	// Set addresses for Jump
    SCB->VTOR = appAddr;
    __set_MSP(*((volatile uint32_t*) appAddr));
    __set_PSP(*((volatile uint32_t*) appAddr));

    appJumpAddress = *((volatile uint32_t*)(appAddr + 4));
    GoToApp = (void (*)(void))appJumpAddress;
    GoToApp();
}


int Flash_ProcessAppCrc(uint16_t pages, uint16_t *pCrc, FlashCrcMode mode)
{
    if (pCrc == NULL)
    {
        return FLASH_INVALID_ARGS;
    }

	int j = 0;
	while (j < pages)
	{
		// Read Chunk
		uint8_t fwChunk[PAGE_SIZE_IN_BYTES] = {0};
        for (int i = 0; i < PAGE_SIZE_IN_BYTES/4; i++)
        {
    		uint32_t flashDword = 0;
            int status = Flash_Read(j*PAGE_SIZE_IN_BYTES + i*4, &flashDword);
            if(status != FLASH_OK)
            {
                return status;
            }
            // Split DWORD by uint8_t
            fwChunk[i*4 + 0] = (uint8_t)(flashDword & 0xFF);
            fwChunk[i*4 + 1] = (uint8_t)((flashDword >> 8) & 0xFF);
            fwChunk[i*4 + 2] = (uint8_t)((flashDword >> 16) & 0xFF);
            fwChunk[i*4 + 3] = (uint8_t)((flashDword >> 24) & 0xFF);
        }
        // Get CRC16 of Chunk
        uint16_t chunkCrc = Common_Crc16(fwChunk, PAGE_SIZE_IN_BYTES);
        // Compare CRC
        if (mode == FLASH_VERIFY_CRC)
        {
            if (chunkCrc != pCrc[j])
            {
                return FLASH_FIRMWARE_CORRUPTED;
            }
        }
        // Calculate CRC
        else
        {
            pCrc[j] = chunkCrc;
        }
        j++;
	}
	return FLASH_OK;
}

int Flash_ReadStringFromMem(char *pBuf, size_t bufSize, uint32_t offset)
{
    if (pBuf == NULL)
    {
        return FLASH_INVALID_ARGS;
    }

    // Unlock FLASH
    int status = Flash_Init();
    if (status != FLASH_OK)
    {
        return FLASH_INIT_FAILED;
    }

    for (int i = 0; i < bufSize/4; i++)
    {
        status = Flash_Read(offset + i*4, ((uint32_t*)(&pBuf[i*4])));
        if(status != FLASH_OK)
        {
            return status;
        }
    }

    // Lock FLASH
    status = Flash_DeInit();
    if (status != FLASH_OK)
    {
        return FLASH_DEINIT_FAILED;
    }

    return FLASH_OK;
}

int Flash_EraseApp()
{
    // Unlock FLASH
    int status = Flash_Init();
    if (status != FLASH_OK)
    {
        return FLASH_INIT_FAILED;
    }

#if defined (STM32FG0xx) || defined (STM32F103xB)
    for (int i = 0; i <= FLASH_CRC_PAGE; i++)
    {
        status = Flash_Erase(i);
        if (status != FLASH_OK)
        {
            return FLASH_ERASE_FAILED;
        }
    }
#elif defined (STM32F407xx)
    FLASH_EraseInitTypeDef eraseInit;

    uint32_t firstSector = Flash_GetSector(FLASH_USER_START_ADDR);
    uint32_t numOfSectors = Flash_GetSector(ADDR_FLASH_SECTOR_11  +  Flash_GetSectorSize(ADDR_FLASH_SECTOR_11) -1) - firstSector + 1;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = firstSector;
    eraseInit.NbSectors = numOfSectors;

    uint32_t sectorError = 0;
    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
    {
        Flash_DeInit();
        return FLASH_ERASE_FAILED;
    }
#endif

    // Lock FLASH
    status = Flash_DeInit();
    if (status != FLASH_OK)
    {
        return FLASH_DEINIT_FAILED;
    }

    return FLASH_OK;
}

int Flash_UpdateChunk (uint8_t *pData, size_t size, uint8_t page)
{
    if (pData == NULL)
    {
        return FLASH_INVALID_ARGS;
    }

    // Unlock FLASH
    FlashStatus status = Flash_Init();
    if (status != FLASH_OK)
    {
        return FLASH_INIT_FAILED;
    }

    // Write CHUNK to USER_MEM
#if defined (STM32FG0xx)
    for (int dword = 0; dword < size/8; dword++)
    {
        uint64_t dWord64 = (uint64_t)pData[dword*8] |
                          ((uint64_t)pData[dword*8 + 1] << 8)  |
                          ((uint64_t)pData[dword*8 + 2] << 16) |
                          ((uint64_t)pData[dword*8 + 3] << 24) |
                          ((uint64_t)pData[dword*8 + 4] << 32) |
                          ((uint64_t)pData[dword*8 + 5] << 40) |
                          ((uint64_t)pData[dword*8 + 6] << 48) |
                          ((uint64_t)pData[dword*8 + 7] << 56);
        uint32_t offset = PAGE_SIZE_IN_BYTES*page + 8*dword;
        status = Flash_Write(offset, dWord64);
        if (status != FLASH_OK)
        {
            return FLASH_WRITE_FAILED;
        }
    }
#else
    for (int dword = 0; dword < size/4; dword++)
    {
        uint32_t word = pData[dword*4] | (pData[dword*4 + 1] << 8) | (pData[dword*4 + 2] << 16) | (pData[dword*4 + 3] << 24);
        uint32_t offset = PAGE_SIZE_IN_BYTES*page + 4*dword;
        status = Flash_Write(offset, word);
        if (status != FLASH_OK)
        {
            return FLASH_WRITE_FAILED;
        }
    }
#endif

    // Lock FLASH
    status = Flash_DeInit();
    if (status != FLASH_OK)
    {
        return FLASH_DEINIT_FAILED;
    }

    return FLASH_OK;
}

#if defined (STM32F407xx)
uint32_t Flash_GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;

  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
  {
    sectorsize = 16 * 1024;
  }
  else if(Sector == FLASH_SECTOR_4)
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }
  return sectorsize;
}

uint32_t Flash_GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;
  }

  return sector;
}
#else
#endif
#pragma GCC pop_options
