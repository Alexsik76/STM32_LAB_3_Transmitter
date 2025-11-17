/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "rtos_tasks.h"
#include "messaging.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern IWDG_HandleTypeDef hiwdg;
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for radio_task */
osThreadId_t radio_taskHandle;
const osThreadAttr_t radio_task_attributes = {
  .name = "radio_task",
  .stack_size = 160 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for display_task */
osThreadId_t display_taskHandle;
const osThreadAttr_t display_task_attributes = {
  .name = "display_task",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for keypad_task */
osThreadId_t keypad_taskHandle;
const osThreadAttr_t keypad_task_attributes = {
  .name = "keypad_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for logic_task */
osThreadId_t logic_taskHandle;
const osThreadAttr_t logic_task_attributes = {
  .name = "logic_task",
  .stack_size = 192 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for radioTxQueueHandle */
osMessageQueueId_t radioTxQueueHandleHandle;
const osMessageQueueAttr_t radioTxQueueHandle_attributes = {
  .name = "radioTxQueueHandle"
};
/* Definitions for displayQueueHandle */
osMessageQueueId_t displayQueueHandleHandle;
const osMessageQueueAttr_t displayQueueHandle_attributes = {
  .name = "displayQueueHandle"
};
/* Definitions for keyEventQueueHandle */
osMessageQueueId_t keyEventQueueHandleHandle;
const osMessageQueueAttr_t keyEventQueueHandle_attributes = {
  .name = "keyEventQueueHandle"
};
/* Definitions for i2cTxDoneSemHandle */
osSemaphoreId_t i2cTxDoneSemHandleHandle;
const osSemaphoreAttr_t i2cTxDoneSemHandle_attributes = {
  .name = "i2cTxDoneSemHandle"
};
/* Definitions for radioIrqSemHandle */
osSemaphoreId_t radioIrqSemHandleHandle;
const osSemaphoreAttr_t radioIrqSemHandle_attributes = {
  .name = "radioIrqSemHandle"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void radio_task_entry(void *argument);
void display_task_entry(void *argument);
void keypad_task_entry(void *argument);
void logic_task_entry(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
	HAL_IWDG_Refresh(&hiwdg);
}
/* USER CODE END 2 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	display_init();
	radio_init();



  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of i2cTxDoneSemHandle */
  i2cTxDoneSemHandleHandle = osSemaphoreNew(1, 1, &i2cTxDoneSemHandle_attributes);

  /* creation of radioIrqSemHandle */
  radioIrqSemHandleHandle = osSemaphoreNew(1, 1, &radioIrqSemHandle_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of radioTxQueueHandle */
  radioTxQueueHandleHandle = osMessageQueueNew (2, 32, &radioTxQueueHandle_attributes);

  /* creation of displayQueueHandle */
  displayQueueHandleHandle = osMessageQueueNew (4, 40, &displayQueueHandle_attributes);

  /* creation of keyEventQueueHandle */
  keyEventQueueHandleHandle = osMessageQueueNew (8, 1, &keyEventQueueHandle_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of radio_task */
  radio_taskHandle = osThreadNew(radio_task_entry, NULL, &radio_task_attributes);

  /* creation of display_task */
  display_taskHandle = osThreadNew(display_task_entry, NULL, &display_task_attributes);

  /* creation of keypad_task */
  keypad_taskHandle = osThreadNew(keypad_task_entry, NULL, &keypad_task_attributes);

  /* creation of logic_task */
  logic_taskHandle = osThreadNew(logic_task_entry, NULL, &logic_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_radio_task_entry */
/**
* @brief Function implementing the radio_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_radio_task_entry */
void radio_task_entry(void *argument)
{
  /* USER CODE BEGIN radio_task_entry */
  /* Infinite loop */
	radio_run_task();
  /* USER CODE END radio_task_entry */
}

/* USER CODE BEGIN Header_display_task_entry */
/**
* @brief Function implementing the display_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_display_task_entry */
void display_task_entry(void *argument)
{
  /* USER CODE BEGIN display_task_entry */
  /* Infinite loop */
	display_run_task();
  /* USER CODE END display_task_entry */
}

/* USER CODE BEGIN Header_keypad_task_entry */
/**
* @brief Function implementing the keypad_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_keypad_task_entry */
void keypad_task_entry(void *argument)
{
  /* USER CODE BEGIN keypad_task_entry */
  /* Infinite loop */
	keypad_run_task();
  /* USER CODE END keypad_task_entry */
}

/* USER CODE BEGIN Header_logic_task_entry */
/**
* @brief Function implementing the logic_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_logic_task_entry */
void logic_task_entry(void *argument)
{
  /* USER CODE BEGIN logic_task_entry */
  /* Infinite loop */
	logic_run_task();
  /* USER CODE END logic_task_entry */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

