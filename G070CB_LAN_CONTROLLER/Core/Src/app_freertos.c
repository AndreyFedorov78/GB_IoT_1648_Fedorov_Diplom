/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "common.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 300 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId LanTaskHandle;
uint32_t LanTaskBuffer[ 400 ];
osStaticThreadDef_t LanTaskControlBlock;
osThreadId RadioReceiverHandle;
uint32_t RadioReceiverBuffer[ 200 ];
osStaticThreadDef_t RadioReceiverControlBlock;
osThreadId ModBusHandle;
uint32_t myTask04Buffer[ 300 ];
osStaticThreadDef_t myTask04ControlBlock;
osThreadId SaveSettingsHandle;
uint32_t myTask05Buffer[ 128 ];
osStaticThreadDef_t myTask05ControlBlock;
osMessageQId PeriodsHandle;
uint8_t PeriodsBuffer[ 30 * sizeof( uint32_t ) ];
osStaticMessageQDef_t PeriodsControlBlock;
osMessageQId MQTT_MessagesHandle;
uint8_t MQTT_MessagesBuffer[ 10 * sizeof( MQTT_Packet ) ];
osStaticMessageQDef_t MQTT_MessagesControlBlock;
osMessageQId logQueueHandle;
uint8_t myQueue03Buffer[ 200 * sizeof( unsigned char ) ];
osStaticMessageQDef_t myQueue03ControlBlock;
osMessageQId modbusQueueHandle;
uint8_t modbusQueue04Buffer[ 5 * sizeof( uint64_t ) ];
osStaticMessageQDef_t myQueue04ControlBlock;
osTimerId LED_INDICATIONHandle;
osStaticTimerDef_t LED_INDICATIONControlBlock;
osTimerId everySecondHandle;
osStaticTimerDef_t everySecondControlBlock;
osTimerId modBusTimerHandle;
osStaticTimerDef_t myTimer03ControlBlock;
osSemaphoreId SemSaveSettingsHandle;
osStaticSemaphoreDef_t mySemSaveSettingsControlBlock;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartLanTask(void const * argument);
void StartRadioReceiver(void const * argument);
void modbus(void const * argument);
void SaveSettingsTask(void const * argument);
void led_indication(void const * argument);
void everySecondCallback(void const * argument);
void ModBusCallback(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

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
  /* definition and creation of SemSaveSettings */
  osSemaphoreStaticDef(SemSaveSettings, &mySemSaveSettingsControlBlock);
  SemSaveSettingsHandle = osSemaphoreCreate(osSemaphore(SemSaveSettings), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of LED_INDICATION */
  osTimerStaticDef(LED_INDICATION, led_indication, &LED_INDICATIONControlBlock);
  LED_INDICATIONHandle = osTimerCreate(osTimer(LED_INDICATION), osTimerPeriodic, NULL);

  /* definition and creation of everySecond */
  osTimerStaticDef(everySecond, everySecondCallback, &everySecondControlBlock);
  everySecondHandle = osTimerCreate(osTimer(everySecond), osTimerPeriodic, NULL);

  /* definition and creation of modBusTimer */
  osTimerStaticDef(modBusTimer, ModBusCallback, &myTimer03ControlBlock);
  modBusTimerHandle = osTimerCreate(osTimer(modBusTimer), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of Periods */
  osMessageQStaticDef(Periods, 30, uint32_t, PeriodsBuffer, &PeriodsControlBlock);
  PeriodsHandle = osMessageCreate(osMessageQ(Periods), NULL);

  /* definition and creation of MQTT_Messages */
  osMessageQStaticDef(MQTT_Messages, 10, MQTT_Packet, MQTT_MessagesBuffer, &MQTT_MessagesControlBlock);
  MQTT_MessagesHandle = osMessageCreate(osMessageQ(MQTT_Messages), NULL);

  /* definition and creation of logQueue */
  osMessageQStaticDef(logQueue, 200, unsigned char, myQueue03Buffer, &myQueue03ControlBlock);
  logQueueHandle = osMessageCreate(osMessageQ(logQueue), NULL);

  /* definition and creation of modbusQueue */
  osMessageQStaticDef(modbusQueue, 5, uint64_t, modbusQueue04Buffer, &myQueue04ControlBlock);
  modbusQueueHandle = osMessageCreate(osMessageQ(modbusQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityBelowNormal, 0, 300, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of LanTask */
  osThreadStaticDef(LanTask, StartLanTask, osPriorityBelowNormal, 0, 400, LanTaskBuffer, &LanTaskControlBlock);
  LanTaskHandle = osThreadCreate(osThread(LanTask), NULL);

  /* definition and creation of RadioReceiver */
  osThreadStaticDef(RadioReceiver, StartRadioReceiver, osPriorityIdle, 0, 200, RadioReceiverBuffer, &RadioReceiverControlBlock);
  RadioReceiverHandle = osThreadCreate(osThread(RadioReceiver), NULL);

  /* definition and creation of ModBus */
  osThreadStaticDef(ModBus, modbus, osPriorityBelowNormal, 0, 300, myTask04Buffer, &myTask04ControlBlock);
  ModBusHandle = osThreadCreate(osThread(ModBus), NULL);

  /* definition and creation of SaveSettings */
  osThreadStaticDef(SaveSettings, SaveSettingsTask, osPriorityRealtime, 0, 128, myTask05Buffer, &myTask05ControlBlock);
  SaveSettingsHandle = osThreadCreate(osThread(SaveSettings), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  //
  osTimerStart(everySecondHandle, 1000);

  osTimerStart(LED_INDICATIONHandle, 50);


  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
__weak void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartLanTask */
/**
* @brief Function implementing the LanTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLanTask */
__weak void StartLanTask(void const * argument)
{
  /* USER CODE BEGIN StartLanTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartLanTask */
}

/* USER CODE BEGIN Header_StartRadioReceiver */
/**
* @brief Function implementing the RadioReceiver thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRadioReceiver */
__weak void StartRadioReceiver(void const * argument)
{
  /* USER CODE BEGIN StartRadioReceiver */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartRadioReceiver */
}

/* USER CODE BEGIN Header_modbus */
/**
* @brief Function implementing the ModBus thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_modbus */
__weak void modbus(void const * argument)
{
  /* USER CODE BEGIN modbus */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END modbus */
}

/* USER CODE BEGIN Header_SaveSettingsTask */
/**
* @brief Function implementing the SaveSettings thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SaveSettingsTask */
__weak void SaveSettingsTask(void const * argument)
{
  /* USER CODE BEGIN SaveSettingsTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END SaveSettingsTask */
}

/* led_indication function */
__weak void led_indication(void const * argument)
{
  /* USER CODE BEGIN led_indication */

  /* USER CODE END led_indication */
}

/* everySecondCallback function */
__weak void everySecondCallback(void const * argument)
{
  /* USER CODE BEGIN everySecondCallback */

  /* USER CODE END everySecondCallback */
}

/* ModBusCallback function */
__weak void ModBusCallback(void const * argument)
{
  /* USER CODE BEGIN ModBusCallback */

  /* USER CODE END ModBusCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

