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
#include "queue.h"
#include "bsp_key.h"
#include "bsp_led.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityIdle,
};
/* Definitions for Task_ScanKey */
osThreadId_t Task_ScanKeyHandle;
const osThreadAttr_t Task_ScanKey_attributes = {
  .name = "Task_ScanKey",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Task_Led */
osThreadId_t Task_LedHandle;
const osThreadAttr_t Task_Led_attributes = {
  .name = "Task_Led",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Queue_Key */
osMessageQueueId_t Queue_KeyHandle;
const osMessageQueueAttr_t Queue_Key_attributes = {
  .name = "Queue_Key"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void AppTask_ScanKey(void *argument);
void AppTask_Led(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Queue_Key */
  Queue_KeyHandle = osMessageQueueNew (10, sizeof(key_result_t), &Queue_Key_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Task_ScanKey */
  Task_ScanKeyHandle = osThreadNew(AppTask_ScanKey, NULL, &Task_ScanKey_attributes);

  /* creation of Task_Led */
  Task_LedHandle = osThreadNew(AppTask_Led, NULL, &Task_Led_attributes);

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
    for (;;)
    {
        osDelay(1);
    }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_AppTask_ScanKey */
/**
 * @brief Function implementing the Task_ScanKey thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_AppTask_ScanKey */
void AppTask_ScanKey(void *argument)
{
  /* USER CODE BEGIN AppTask_ScanKey */
    key_event_t key_event = \
    {                       
      .triggle_tick = 0,    \
      .state = KEY_RELEASE, \
      .checkstate = No_Check,
    };
    uint32_t key_interval_tick;
		uint32_t key_release_tick;
    key_result_t key_result;
    /* Infinite loop */
    for (;;)
    {
      key_event.state = key_Scan();
      
      switch(key_event.checkstate)
      {
        case No_Check:
          if(KEY_PRESS == key_event.state)
          {
            key_event.triggle_tick = HAL_GetTick();
            key_event.checkstate = Check_During;
          }
          else
          {
            //No action
          }
        break;

        case Check_During:
          if(KEY_RELEASE == key_event.state)
          {
            key_event.checkstate = Check_Complete;
            key_release_tick = HAL_GetTick();
          }
          else
          {
            //No action
          }
        break;

        case Check_Complete:
          key_event.checkstate = No_Check;
          key_interval_tick = key_release_tick - key_event.triggle_tick;
          if( key_release_tick < key_event.triggle_tick)
          {
              /*Clear tick
                Data crosses the line
              */
              key_release_tick = 0;
          }
          else if (key_interval_tick < 2000)
          {
              //Clear tick
              key_release_tick = 0;
              // if key is pressed after release in 200ms
              key_result = KEY_SHORT_PRESS;
              xQueueSend(Queue_KeyHandle, &key_result, portMAX_DELAY);
          }
          else if (key_interval_tick < 5000)
          {
              //Clear tick
              key_release_tick = 0;
              // send result to queue
              key_result = KEY_LONG_PRESS;
              xQueueSend(Queue_KeyHandle, &key_result, portMAX_DELAY);
          }
          else
          {
              //Clear tick
              key_release_tick = 0;
              // send result to queue
              key_result = KEY_NO_PRESS;
              xQueueSend(Queue_KeyHandle, &key_result, portMAX_DELAY);
          }
        break;

        default:
          // No action
        break;
      }
    }
  /* USER CODE END AppTask_ScanKey */
}

/* USER CODE BEGIN Header_AppTask_Led */
/**
 * @brief Function implementing the Task_Led thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_AppTask_Led */
void AppTask_Led(void *argument)
{
  /* USER CODE BEGIN AppTask_Led */
  /* Infinite loop */
  key_result_t KeyValue;
  for (;;)
  {
    xQueueReceive(Queue_KeyHandle, &KeyValue, portMAX_DELAY);
    switch (KeyValue)
    {
        case KEY_SHORT_PRESS:
            Led_Toggle();;
            vTaskDelay(500);
            Led_Toggle();;
        break;

        case KEY_LONG_PRESS:
            // blink 3 times
            for (char i = 0; i < 6; i++)
            {
                Led_Toggle();
                vTaskDelay(500);
            }
        break;

        case KEY_NO_PRESS:
            // No action
        break;

        default:
            // No action
        break;
    }
  }
  /* USER CODE END AppTask_Led */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

