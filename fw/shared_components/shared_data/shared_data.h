#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#if defined (USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#endif
#include "main.h"

#pragma pack(push, 1)
typedef struct SharedDataStruct
{
	uint32_t chipId[3];
#if defined (STM32F407xx)
#endif
} SharedData;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct CanMsgStruct
{
    uint32_t  id;
    size_t    dlc;
    uint8_t   data[8];
} CanMsg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BootloaderCommandsStruct
{
    uint32_t boardId;
    uint32_t timestampId;
    uint32_t timestampAckId;
    uint32_t eraseId;
    uint32_t eraseAckId;
    uint32_t crcId;
    uint32_t crcAckId;
    uint32_t chunkId;
} BootloaderCommands;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct SharedInterfaceStruct
{
    // Can Handlers
    CAN_HandleTypeDef       *pCanHandler;
    uint32_t 				*pCanMailbox;
#if defined (USE_FREERTOS)
    // Can Mutex
    osSemaphoreId           *pCanMutex;
    // Can Queues
    osMessageQId            *pCanRxQueue;
    osMessageQId            *pCanTxQueue;
    // USART Debug Queue
    osMessageQId            *pUsartDbgTxQueue;
    // Semaphore & Pointer to SharedData
    osSemaphoreId           *pDataMutex;
#endif
    SharedData              *pData;
} SharedInterface;
#pragma pack(pop)

#if defined (USE_FREERTOS)
// Shared Data
extern osSemaphoreId 	xSharedDataMutexHandle;
extern osMessageQId 	xCAN1RxQueueHandle;
#endif
extern SharedData    	sharedData;
extern uint32_t         TxMailboxCan1;

#endif
