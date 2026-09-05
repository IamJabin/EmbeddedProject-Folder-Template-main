/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file key.c
 *
 * @par dependencies
 * - bsp_key.h
 * - stdio.h
 * - stdint.h
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
  uint32_t countr_tick = 0;

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
    countr_tick++;

    key_state = key_scan( & key_value);

    if(KEY_OK == key_state)
    {
      if(KEY_PRESSED == key_value)
      {
        printf("key pressed\r\n");
        if(pdTRUE == xQueueSend(key_queue, &countr_tick, 0))
        {
          printf("key_queue send success\r\n");
        }
        else
        {
          printf("key_queue send failed\r\n");
        }
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
    uint32_t counter = 0;
    key_press_status_t key_value_temp = KEY_NOT_PRESSED;

    if (key_value == NULL)
    {
        return KEY_ERRORPARAMETER;
    }

    while (counter < 1000)
    {
        if(HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET)
        {
            key_value_temp = KEY_PRESSED;
            *key_value = key_value_temp;
            return KEY_OK;
        }
        counter++;
    }
    *key_value = key_value_temp;

    return KEY_ERRORTIMEOUT;
}
