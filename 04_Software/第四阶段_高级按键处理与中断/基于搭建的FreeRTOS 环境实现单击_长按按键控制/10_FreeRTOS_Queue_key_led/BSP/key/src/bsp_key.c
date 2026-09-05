/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bsp_key.c
 *
 * @par dependencies
 * - bsp_key.h
 *
 * @author Bin
 *
 * @brief Provide the HAL APIs of Key and corresponding opetions.
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.0 2026-08-24
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#include "bsp_key.h"

extern QueueHandle_t key_queue = NULL;
static osThreadId_t key_TaskHandle = NULL;

static const osThreadAttr_t key_Task_attributes = {
  .name = "key_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

key_status_t key_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin : Key_Pin */
    GPIO_InitStruct.Pin = Key_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(Key_GPIO_Port, &GPIO_InitStruct);

    return KEY_OK;
}

key_status_t key_thread_init(void)
{
    key_TaskHandle = osThreadNew(key_task_func, NULL, &key_Task_attributes);
    if (key_TaskHandle == NULL)
    {
        printf("key thread create failed\r\n");
        return KEY_ERROR;
    }
    else
    {
        printf("key thread create success\r\n");
        return KEY_OK;
    }
}

/**
  * @brief key thread function
  * @param void *argument
  * @retval void
  */
void key_task_func(void *argument)
{
  // printf("key_task_func in\r\n");
  key_status_t key_state        =          KEY_OK;
  key_press_status_t key_value  = KEY_NOT_PRESSED;
  key_queue = xQueueCreate(10, sizeof( uint32_t ));

  if (NULL == key_queue)
  {
    printf("key_queue create failed\r\n");
    // Handle the error, e.g., return or enter an error state
    vTaskDelete(NULL); // Delete the current task to prevent further execution
  }
  else
  {
    printf("key_queue create success\r\n");
  }

  for(;;)
  {
    key_state = key_short_long_scan(&key_value, 2000); // 2000ms长按阈值

    if(KEY_OK == key_state)
    {
      printf("key pressed\r\n");
      if(pdTRUE == xQueueSend(key_queue, &key_value, pdMS_TO_TICKS(5000)))
      {
        printf("key_queue send success key_value: %d, tick: %lu\r\n", 
                                     key_value, xTaskGetTickCount());
      }
      else
      {
        printf("key_queue send failed\r\n");
      }
    }
    else
    {
    //   printf("have not key\r\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

key_status_t key_scan(key_press_status_t * key_value)
{
    key_press_status_t key_value_temp = KEY_NOT_PRESSED;

    if (key_value == NULL)
    {
        return KEY_ERRORPARAMETER;
    }

    if(HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET)
    {
        key_value_temp = KEY_PRESSED;
        *key_value = key_value_temp;
        return KEY_OK;
    }

    *key_value = key_value_temp;

    return KEY_ERRORTIMEOUT;
}

//识别按键的长按和短按，长按还是短按临界值靠调用者传入
key_status_t key_short_long_scan(key_press_status_t * key_value, 
                                 uint32_t long_press_threshold_ms)
{
    uint32_t                  counter   =                 0;
    key_press_status_t key_value_temp   =   KEY_NOT_PRESSED;
    key_status_t            key_state   =         KEY_ERROR;

    if (key_value == NULL)
    {
        return KEY_ERRORPARAMETER;
    }

    key_state = key_scan( & key_value_temp);

    if (key_state == KEY_OK && key_value_temp == KEY_PRESSED)
    {
        //按键按下，开始计数
        while (key_scan( & key_value_temp) == KEY_OK && key_value_temp == KEY_PRESSED)
        {
            counter++;
            osDelay(10); //延时10ms
        }

        if (counter >= long_press_threshold_ms / 10) //长按阈值为指定毫秒（long_press_threshold_ms / 10ms）
        {
            *key_value = KEY_LONG_PRESSED;
            return KEY_OK;
        }
        else if (counter > 0) //短按阈值为0.1秒（10 * 10ms）
        {
            *key_value = KEY_SHORT_PRESSED;
            return KEY_OK;
        }
    }
    else
    {
      /* code */
    }
    


    return KEY_ERRORTIMEOUT;
}
