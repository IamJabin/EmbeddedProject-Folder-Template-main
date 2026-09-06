/******************************************************************************
 * Copyright (C) 2026 EternalChip, Inc.(Gmbh) or its affiliates.
 *
 * All Rights Reserved.
 *
 * @file bsp_led.c
 *
 * @par dependencies
 * - bsp_led.h
 * - stdio.h
 * - stdint.h
 *
 * @author Bin
 *
 * @brief Provide the HAL APIs of Led and corresponding opetions.
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

#include "bsp_led.h"

#define LED_DEBUG_ENABLE 0
#define LED_BLINK_INTERVAL_MS 200U

#if LED_DEBUG_ENABLE
#define LED_LOG(...) printf(__VA_ARGS__)
#else
#define LED_LOG(...) ((void)0)
#endif

QueueHandle_t led_queue = NULL;
static osThreadId_t led_TaskHandle = NULL;

static const osThreadAttr_t led_Task_attributes = {
  .name = "led_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

led_status_t led_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /*Configure GPIO pin : LED_Pin */
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

    return LED_OK;
}

/**
  * @brief led control function
  * @param led_value: LED_OFF, LED_ON, LED_TOGGLE, LED_TOGGLE_3_times
  * @retval led_status_t: LED_OK, LED_ERRORPARAMETER
  */
led_status_t led_control(led_light_status_t led_value)
{
    switch (led_value)
    {
        case LED_OFF:
            LED_LOG("led_control: LED_OFF, tick: %lu\r\n", 
                      (unsigned long)xTaskGetTickCount());
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            break;
        case LED_ON:
            LED_LOG("led_control: LED_ON, tick: %lu\r\n", 
                     (unsigned long)xTaskGetTickCount());
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            break;
        case LED_TOGGLE:
            LED_LOG("led_control: LED_TOGGLE, tick: %lu\r\n", 
                         (unsigned long)xTaskGetTickCount());
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            break;
        case LED_TOGGLE_3_TIMES:
            LED_LOG("led_control: LED_TOGGLE_3_TIMES, tick: %lu\r\n", 
                                 (unsigned long)xTaskGetTickCount());
            for (int i = 0; i < 3; i++)
            {
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                vTaskDelay(pdMS_TO_TICKS(LED_BLINK_INTERVAL_MS));; // Delay for 200 ms
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                vTaskDelay(pdMS_TO_TICKS(LED_BLINK_INTERVAL_MS));; // Delay for 200 ms
            }
            break;
        default:
            return LED_ERRORPARAMETER; // Invalid parameter
    }
    return LED_OK;
}

/**
  * @brief led thread function
  * @param void *argument
  * @retval void
  */
void led_task_func(void *argument)
{
  // LED_LOG("led_task_func in\r\n");
  led_status_t led_state        =          LED_OK;
  led_light_status_t led_value  =         LED_OFF;
  led_queue = xQueueCreate(10, sizeof( led_light_status_t ));

  if (NULL == led_queue)
  {
    LED_LOG("led_queue create failed\r\n");
    // Handle the error, e.g., return or enter an error state
    vTaskDelete(NULL); // Delete the current task to prevent further execution
  }
  else
  {
    LED_LOG("led_queue create success\r\n");
  }

  for(;;)
  {
    //获取led_queue中的数据，然后放入到led_control函数中，控制led灯的状态
    if(NULL != led_queue)
    {
      if(pdTRUE == xQueueReceive(led_queue, &led_value, portMAX_DELAY))
      {
        LED_LOG("led_queue receive success, led_value = %d, tick: %lu\r\n", 
                            led_value, (unsigned long)xTaskGetTickCount());
        led_state = led_control(led_value);
        if(LED_OK != led_state)
        {
          LED_LOG("led_control failed\r\n");
        }
      }
      else
      {
        // LED_LOG("led_queue have not receive\r\n");
      }
    }
    else
    {
      LED_LOG("%s-%d, led_queue is NULL\r\n", __FUNCTION__, __LINE__);
    }
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}

led_status_t led_thread_init(void)
{
    // Create the LED thread
    led_TaskHandle = osThreadNew(led_task_func, NULL, &led_Task_attributes);
    if (led_TaskHandle == NULL)
    {
        LED_LOG("LED thread create failed\r\n");
        return LED_ERROR;
    }
    else
    {
        LED_LOG("LED thread create success\r\n");
        return LED_OK;
    }
}
