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
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Task delays
#define FREERTOS_MAIN_TASK_DELAY_MS         100
#define FREERTOS_CAN_RX_TASK_DELAY_MS       10
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
osThreadId MainTaskHandle;
osThreadId CAN1RxTaskHandle;
osMessageQId xCAN1RxQueueHandle;
osSemaphoreId xSharedDataMutexHandle;
osSemaphoreId xCanMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
uint32_t Filter(uint32_t *sum, uint32_t *buffer, uint32_t value, int index,int filterSize);
uint32_t GetVector(uint32_t joystickPositionY, uint32_t joystickPositionX, float yCoef, float xCoef);
int Clamp(int value, int min, int max);
int Map(int x, int inMin, int inMax, int outMin, int outMax);
unsigned int Root(uint32_t value);
/* USER CODE END FunctionPrototypes */

void StartMainTask(void const * argument);
void StartCAN1RxTask(void const * argument);

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

  /* definition and creation of xCanMutex */
  osSemaphoreDef(xCanMutex);
  xCanMutexHandle = osSemaphoreCreate(osSemaphore(xCanMutex), 1);

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

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    SharedInterface *pSharedInterface = (SharedInterface*) calloc(1, sizeof(SharedInterface));
    // ToDo: add separate instances
    pSharedInterface->pCanMutex   = &xCanMutexHandle;
    pSharedInterface->pCanRxQueue = &xCAN1RxQueueHandle;
    pSharedInterface->pDataMutex = &xSharedDataMutexHandle;
    pSharedInterface->pData = &sharedData;
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of MainTask */
  osThreadDef(MainTask, StartMainTask, osPriorityNormal, 0, 256);
  MainTaskHandle = osThreadCreate(osThread(MainTask), (void*) pSharedInterface);

  /* definition and creation of CAN1RxTask */
  osThreadDef(CAN1RxTask, StartCAN1RxTask, osPriorityNormal, 0, 256);
  CAN1RxTaskHandle = osThreadCreate(osThread(CAN1RxTask), (void*) pSharedInterface);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartMainTask */
/**
 * @brief  Function implementing the MainTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void const * argument)
{
  /* USER CODE BEGIN StartMainTask */
    if (argument == NULL)
    {
        vTaskDelete(MainTaskHandle);
    }
    SharedInterface *pSharedInterface = (SharedInterface*) argument;
    /* Infinite loop */
    for (;;)
    {
    	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        osDelay(FREERTOS_MAIN_TASK_DELAY_MS);
    }
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartCAN1RxTask */
/**
 * @brief Function implementing the CAN1RxTask thread.
 * @param argument: Not used
 * @retval None
 */
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
    for (;;)
    {
        CanMsg currentMsg = {0,0,{0}};
        if (xQueueReceive(*pSharedInterface->pCanRxQueue, &currentMsg, (TickType_t)0))
        {
            // 1. Handle Cmd
            switch (currentMsg.id)
            {
                case CAN_ID_BUS_CONTROL:
                {
                    if (currentMsg.data[0] == 1)
                    {
                        xSemaphoreGive(*pSharedInterface->pCanMutex);
                    }
                    else if (currentMsg.data[0] == 0)
                    {
                        xSemaphoreTake(*pSharedInterface->pCanMutex, (TickType_t)5000/portTICK_RATE_MS);
                    }
                }
                break;

                case CAN_ID_APP_SWITCH:
                {
                    if (currentMsg.data[0] == CAN_ID_APP_F407_BOARD_CODE && currentMsg.data[1] == CAN_ID_APP_SWITCH_BOOTLOADER)
                    {
                        NVIC_SystemReset();
                    }
                }
                break;
            }
        }
        osDelay(FREERTOS_CAN_RX_TASK_DELAY_MS);
    }
  /* USER CODE END StartCAN1RxTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* USER CODE END Application */
