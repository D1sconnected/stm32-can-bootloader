/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define FREERTOS_CAN_RX_TASK_DELAY_MS 		10
#define FREERTOS_USART_DBG_TX_TASK_DELAY_MS 100

#define LOG_ENABLE 0
#define STATUS_LED 1

static uint8_t pUpdatePageBuf[PAGE_SIZE_IN_BYTES] __attribute__ ((aligned (1)));
static uint16_t pCrcBuf[FLASH_TIMESTAMP_PAGE] __attribute__ ((aligned (1)));
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId CAN1RxTaskHandle;
osThreadId UsartDbgTxTaskHandle;
osMessageQId xCAN1RxQueueHandle;
osMessageQId xUsartDbgTxQueueHandle;
osSemaphoreId xSharedDataMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartCAN1RxTask(void const * argument);
void StartUsartDbgTxTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of xSharedDataMutex */
  osSemaphoreDef(xSharedDataMutex);
  xSharedDataMutexHandle = osSemaphoreCreate(osSemaphore(xSharedDataMutex), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of xCAN1RxQueue */
  osMessageQDef(xCAN1RxQueue, 10, CanMsg);
  xCAN1RxQueueHandle = osMessageCreate(osMessageQ(xCAN1RxQueue), NULL);

  /* definition and creation of xUsartDbgTxQueue */
  osMessageQDef(xUsartDbgTxQueue, 5, char);
  xUsartDbgTxQueueHandle = osMessageCreate(osMessageQ(xUsartDbgTxQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    SharedInterface *pSharedInterface = (SharedInterface*) calloc(1, sizeof(SharedInterface));
    // ToDo: add separate instances
    pSharedInterface->pCanRxQueue = &xCAN1RxQueueHandle;
    pSharedInterface->pUsartDbgTxQueue = &xUsartDbgTxQueueHandle;
    pSharedInterface->pDataMutex = &xSharedDataMutexHandle;
    pSharedInterface->pData = &sharedData;
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of CAN1RxTask */
  osThreadDef(CAN1RxTask, StartCAN1RxTask, osPriorityNormal, 0, 512);
  CAN1RxTaskHandle = osThreadCreate(osThread(CAN1RxTask), (void*) pSharedInterface);

  /* definition and creation of UsartDbgTxTask */
  osThreadDef(UsartDbgTxTask, StartUsartDbgTxTask, osPriorityIdle, 0, 256);
  UsartDbgTxTaskHandle = osThreadCreate(osThread(UsartDbgTxTask), (void*) pSharedInterface);

  /* USER CODE BEGIN RTOS_THREADS */
#if LOG_ENABLE
  Common_PlaceLogMessage("INF", "F407 Bootloader is starting", strlen("INF") + strlen("F407 Bootloader is starting"), &xUsartDbgTxQueueHandle);
#endif
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartCAN1RxTask */
/**
 * @brief Function implementing the CAN1RxTask thread.
 * @param argument: Not used
 * @retval None
 */
static int CAN1Task_CmdHandler(SharedInterface *pSharedInterface, CanMsg *pCurrentMsg, BootloaderCommands *pCommand)
{
    static uint8_t page = 0;        // Reset on START & END Events
    static int index = 0;

    int status = -1;
    if (pSharedInterface == NULL || pCurrentMsg == NULL)
    {
        return status;
    }

    // Handler
    if (pCurrentMsg->id == pCommand->chunkId)
    {
        // 4. Copy 8 bytes to buffer
        if (index < PAGE_SIZE_IN_BYTES)
        {
            memcpy(&pUpdatePageBuf[index], pCurrentMsg->data, pCurrentMsg->dlc);
            index += pCurrentMsg->dlc;
        }
    }
    else if (pCurrentMsg->id == pCommand->eraseId)
    {
        // Erase if currentPage is 0
        if (!gIsErased)
        {
            status = Flash_EraseApp();
            if (status != FLASH_OK)
            {
                gIsCorrupted = 1;
            }
            else
            {
                gIsErased = 1;
                page = 0;
                index = 0;
                memset(pCrcBuf, 0xFFFF, sizeof(pCrcBuf));
            }
        }

        // 1. Prepare Can Response Message with confirmation
        uint8_t txData[8] = { FLASH_CRC_PAGE, 0, 0, 0, 0, 0, 0, 0 };
        CAN_TxHeaderTypeDef txHeader = { .StdId = pCommand->eraseAckId,
                                         .ExtId = 0,
                                         .RTR = CAN_RTR_DATA,
                                         .IDE = CAN_ID_STD,
                                         .DLC = 1,
                                         .TransmitGlobalTime = 0 };

        // 2. Send message
        if (HAL_CAN_AddTxMessage(pSharedInterface->pCanHandler, &txHeader, txData, &TxMailboxCan1) != HAL_OK)
        {
        }

        // 3. Place Log
#if LOG_ENABLE
        Common_PlaceLogMessage("INF", "App is erased", strlen("INF") + strlen("App is erased"), &xUsartDbgTxQueueHandle);
#endif
    }
    else if(pCurrentMsg->id == pCommand->crcId)
    {
        // 1. Prepare Can Response Message with confirmation
        uint8_t txData[8] = { 0 };
        CAN_TxHeaderTypeDef txHeader = { .StdId = pCommand->crcAckId,
                                         .ExtId = 0, .RTR = CAN_RTR_DATA,
                                         .IDE = CAN_ID_STD,
                                         .DLC = pCurrentMsg->dlc,
                                         .TransmitGlobalTime = 0 };

        // 2. Calculate CRC of pUpdatePageBuf
        uint16_t crcOfPage = Common_Crc16(pUpdatePageBuf, index);
        pCrcBuf[page] = crcOfPage; // save for later
        txData[0] = (uint8_t)(crcOfPage & 0xFF);
        txData[1] = (uint8_t)((crcOfPage >> 8) & 0xFF);

        // 3. Flash if CRC matched
        if (pCurrentMsg->data[0] == txData[0] && pCurrentMsg->data[1] == txData[1] && !gIsCorrupted)
        {
            status = Flash_UpdateChunk(pUpdatePageBuf, PAGE_SIZE_IN_BYTES, page);
            if (status != FLASH_OK)
            {
                gIsCorrupted = 1;
            }
            else
            {
                page++;
                index = 0;
                memset(pUpdatePageBuf, 0xFF, sizeof(pUpdatePageBuf));

                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

                // Place Log
#if LOG_ENABLE
                char pMessage[128] = {0};
                snprintf(&pMessage[strlen(pMessage)], sizeof(pMessage), "%d", page - 1);
                strncat(pMessage, " page is programmed", sizeof(pMessage));
                Common_PlaceLogMessage("INF", pMessage, strlen("INF") + strlen(pMessage), pSharedInterface->pUsartDbgTxQueue);
#endif
            }
        }

        // 4. Send message
        if (HAL_CAN_AddTxMessage(pSharedInterface->pCanHandler, &txHeader, txData, &TxMailboxCan1) != HAL_OK)
        {
        }
    }
    else if(pCurrentMsg->id == pCommand->timestampId)
    {
        // 1. Prepare Can Response Message with confirmation
        uint8_t txData[8] = { 0 };
        CAN_TxHeaderTypeDef txHeader = { .StdId = pCommand->timestampAckId,
                                         .ExtId = 0,
                                         .RTR = CAN_RTR_DATA,
                                         .IDE = CAN_ID_STD,
                                         .DLC = pCurrentMsg->dlc,
                                         .TransmitGlobalTime = 0 };

        // 2. Read timestamp from mem
        uint32_t timeStampDword = 0;
        uint8_t index = pCurrentMsg->data[0];
        status = Flash_Read(FLASH_VASR_START_ADDR - FLASH_USER_START_ADDR + index * 4, &timeStampDword);

        // 3. Prepare message
        txData[0] = index;
        txData[1] = (uint8_t) (timeStampDword & 0xFF);
        txData[2] = (uint8_t) ((timeStampDword >> 8) & 0xFF);
        txData[3] = (uint8_t) ((timeStampDword >> 16) & 0xFF);
        txData[4] = (uint8_t) ((timeStampDword >> 24) & 0xFF);

        // 4. Send message
        if (HAL_CAN_AddTxMessage(pSharedInterface->pCanHandler, &txHeader, txData, &TxMailboxCan1) != HAL_OK)
        {
        }
    }
    else if(pCurrentMsg->id == CAN_ID_APP_SWITCH)
    {
        // Jump to App
        if (pCurrentMsg->data[0] == pCommand->boardId &&
        	pCurrentMsg->data[1] == CAN_ID_APP_SWITCH_MAIN)
        {
            // Verify CRC
            status = Flash_ProcessAppCrc(FLASH_TIMESTAMP_PAGE, pCrcBuf, FLASH_VERIFY_CRC);
            if (status == FLASH_OK)
            {
                // Save CRC to last page
                status = Flash_UpdateChunk((uint8_t*) pCrcBuf, sizeof(pCrcBuf), FLASH_CRC_PAGE);

#if LOG_ENABLE
                char pMessage[512] = {0};
                strncat(pMessage, "Save CRC to last page: ", sizeof(pMessage));
                for (int i = 0; i < FLASH_TIMESTAMP_PAGE; i++)
                {
                    snprintf(&pMessage[strlen(pMessage)], sizeof(pMessage), "%04X ", pCrcBuf[i]);
                }
                Common_PlaceLogMessage("INF", pMessage, strlen("INF") + strlen(pMessage), pSharedInterface->pUsartDbgTxQueue);
                vTaskDelay(FLASH_COUNTDOWN_TO_START_MS);
#endif
                if (status == FLASH_OK)
                {
                    // We received new FW, so we need to jump from bootloader to main application
                    Flash_JumpToApp(FLASH_FW_START_ADDR);
                }
                else
                {
                    NVIC_SystemReset();
                }
            }
        }
        // Reset to bootloader
        else if (pCurrentMsg->data[0] == pCommand->boardId &&
        		 pCurrentMsg->data[1] == CAN_ID_APP_SWITCH_BOOTLOADER)
        {
            NVIC_SystemReset();
        }
    }
    return status;
}
/* USER CODE END Header_StartCAN1RxTask */
void StartCAN1RxTask(void const * argument)
{
  /* USER CODE BEGIN StartCAN1RxTask */
    if (argument == NULL)
    {
        vTaskDelete(CAN1RxTaskHandle);
    }
    SharedInterface *pSharedInterface = (SharedInterface*) argument;
    /* Infinite loop */
    uint32_t countToStart = 0;

    // Here you can create a set of RX Commands for different boards in your project
    BootloaderCommands commandList = {0};
    commandList.boardId = CAN_ID_APP_F407_BOARD_CODE;
    commandList.timestampId = CAN_ID_UPDATE_F407_BOARD_TIMESTAMP;
    commandList.timestampAckId = CAN_ID_UPDATE_F407_BOARD_TIMESTAMP_ACK;
    commandList.eraseId = CAN_ID_UPDATE_F407_BOARD_ERASE;
    commandList.eraseAckId = CAN_ID_UPDATE_F407_BOARD_ERASE_ACK;
    commandList.crcId = CAN_ID_UPDATE_F407_BOARD_CRC;
    commandList.crcAckId = CAN_ID_UPDATE_F407_BOARD_CRC_ACK;
    commandList.chunkId = CAN_ID_UPDATE_F407_BOARD_CHUNK;

    pSharedInterface->pCanHandler = &hcan1;
    pSharedInterface->pCanMailbox = &TxMailboxCan1;
    HAL_CAN_Start(pSharedInterface->pCanHandler);
    HAL_CAN_ActivateNotification(pSharedInterface->pCanHandler, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE);

    for (;;)
    {
        // 1. Check if we have FW to boot
        if (countToStart > FLASH_COUNTDOWN_TO_START_MS && !gIsErased && !gIsCorrupted)
        {
            if (Flash_CheckFwPresented() == FLASH_OK)
            {
                char pStrFromMem[256] = { 0 };
                if (Flash_ReadStringFromMem(pStrFromMem, sizeof(pStrFromMem), (FLASH_CRC_START_ADDR - FLASH_USER_START_ADDR)) == FLASH_OK)
                {
                    if (Flash_ProcessAppCrc(FLASH_TIMESTAMP_PAGE, (uint16_t*) pStrFromMem, FLASH_VERIFY_CRC) == FLASH_OK)
                    {
                        // Read timestamp
                        memset(pStrFromMem, 0, sizeof(pStrFromMem));
                        Flash_ReadStringFromMem(pStrFromMem, sizeof(pStrFromMem), FLASH_VASR_START_ADDR - FLASH_USER_START_ADDR);

                        // Place Log
#if LOG_ENABLE
                        char pMessage[128] = {0};
                        strncat(pMessage, "Found App to boot, timestamp: ", sizeof(pMessage));
                        strncat(pMessage, pStrFromMem, 128);
                        Common_PlaceLogMessage("INF", pMessage, strlen("INF") + strlen(pMessage), pSharedInterface->pUsartDbgTxQueue);
                        vTaskDelay(FLASH_COUNTDOWN_TO_START_MS);
#endif
                        Flash_JumpToApp(FLASH_FW_START_ADDR);
                    }
#if LOG_ENABLE
                    else
                    {
                        // Place Log
                        char pMessage[128] = {0};
                        Common_PlaceLogMessage("INF", "Found App, but CRC is corrupted", strlen("INF") + strlen("Found App, but CRC is corrupted"), pSharedInterface->pUsartDbgTxQueue);
                    }
#endif
                }
            }
#if LOG_ENABLE
            else
            {
                // Place Log
                Common_PlaceLogMessage("INF", "App Sector is Empty or Corrupted", strlen("INF") + strlen("App Sector is Empty or Corrupted"), pSharedInterface->pUsartDbgTxQueue);
            }
#endif
            countToStart = 0;
        }
        else
        {
            countToStart++;
#if LOG_ENABLE
            if (countToStart % 100 == 0 && !gIsErased && !gIsCorrupted)
            {
                char pMessage[128] = {0};
                strncat(pMessage, "Seconds passed before boot: ", sizeof(pMessage));
                snprintf(&pMessage[strlen(pMessage)], sizeof(pMessage), "%d", countToStart/100);
                Common_PlaceLogMessage("INF", pMessage, strlen("INF") + strlen(pMessage), pSharedInterface->pUsartDbgTxQueue);
            }
#endif
#if STATUS_LED
            // ToDo: refactor
            if (countToStart % 20 == 0)
            {
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
#endif
        }

        // 2. Handle CAN messages
        CanMsg currentMsg = { 0, 0, { 0 } };
        if (HAL_CAN_GetTxMailboxesFreeLevel(pSharedInterface->pCanHandler) == 0)
        {
            osDelay(FREERTOS_CAN_RX_TASK_DELAY_MS);
        }
        else
        {
            if (xQueueReceive(*pSharedInterface->pCanRxQueue, &currentMsg, (TickType_t) 0))
            {
                CAN1Task_CmdHandler(pSharedInterface, &currentMsg, &commandList);
            }
            else
            {
                osDelay(FREERTOS_CAN_RX_TASK_DELAY_MS);
            }
        }
    }
  /* USER CODE END StartCAN1RxTask */
}

/* USER CODE BEGIN Header_StartUsartDbgTxTask */
/**
* @brief Function implementing the UsartDbgTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUsartDbgTxTask */
void StartUsartDbgTxTask(void const * argument)
{
  /* USER CODE BEGIN StartUsartDbgTxTask */
    if (argument == NULL)
    {
        vTaskDelete(UsartDbgTxTaskHandle);
    }
    SharedInterface *pSharedInterface = (SharedInterface*) argument;
  /* Infinite loop */
    for(;;)
    {
        osDelay(FREERTOS_USART_DBG_TX_TASK_DELAY_MS);
    }
  /* USER CODE END StartUsartDbgTxTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */
