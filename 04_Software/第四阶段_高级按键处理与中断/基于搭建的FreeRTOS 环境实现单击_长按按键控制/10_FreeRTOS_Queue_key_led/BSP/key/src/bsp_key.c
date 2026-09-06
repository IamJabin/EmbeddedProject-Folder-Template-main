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

QueueHandle_t key_queue = NULL;
static osThreadId_t key_TaskHandle = NULL;

static const osThreadAttr_t key_Task_attributes = {
  .name = "key_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

// 状态0：等待稳定按下
// 状态1：已确认按下，等待稳定释放并判断长短按
typedef enum
{
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESSED
} key_state_machine_t;

#define KEY_SCAN_PERIOD_MS    10U
#define KEY_DEBOUNCE_MS       20U

static key_state_machine_t key_state = KEY_STATE_IDLE;
static TickType_t press_tick = 0;
static TickType_t long_press_ticks = 0;

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
    key_status_t      scan_result =          KEY_OK;
    key_press_status_t  key_value = KEY_NOT_PRESSED;
    TickType_t     last_wake_tick =               0;
    uint32_t       dropped_events =               0;

    key_queue = xQueueCreate(10, sizeof(key_press_status_t));

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

    last_wake_tick = xTaskGetTickCount();
    for (;;)
    {
        scan_result = key_short_long_scan(&key_value, 2000U);

        if (scan_result == KEY_OK)
        {
            if (pdTRUE == xQueueSend(key_queue, &key_value, 0))
            {
                printf("key_queue send success key_value: %d, tick: %lu\r\n",
                        key_value, (unsigned long)xTaskGetTickCount());
            }
            else
            {
                dropped_events++;
                printf("key_queue full, dropped: %lu\r\n",
                        (unsigned long)dropped_events);
            }
        }
        else if (scan_result != KEY_ERRORTIMEOUT)
        {
            printf("key scan error: %d\r\n", scan_result);
        }

        vTaskDelayUntil(&last_wake_tick,
                        pdMS_TO_TICKS(KEY_SCAN_PERIOD_MS));
    }
}

key_status_t key_scan(key_press_status_t *key_value)
{
    static key_press_status_t stable_value = KEY_NOT_PRESSED;
    static key_press_status_t candidate_value = KEY_NOT_PRESSED;
    static TickType_t candidate_tick = 0;

    key_press_status_t raw_value = KEY_NOT_PRESSED;
    TickType_t now_tick = 0;

    if (key_value == NULL)
    {
        return KEY_ERRORPARAMETER;
    }

    if (HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET)
    {
        raw_value = KEY_PRESSED;
    }

    now_tick = xTaskGetTickCount();

    if (raw_value != candidate_value)
    {
        candidate_value = raw_value;
        candidate_tick = now_tick;
    }
    else if ((TickType_t)(now_tick - candidate_tick) >=
            pdMS_TO_TICKS(KEY_DEBOUNCE_MS))
    {
        stable_value = candidate_value;
    }

    *key_value = stable_value;

    if (stable_value == KEY_PRESSED)
    {
        return KEY_OK;
    }

    return KEY_ERRORTIMEOUT;
}

//识别按键的长按和短按，长按还是短按临界值靠调用者传入
//现在需要将函数升级为状态机的形式
key_status_t key_short_long_scan(key_press_status_t *key_value,
                                   uint32_t long_press_threshold_ms)
{
    key_press_status_t stable_value = KEY_NOT_PRESSED;
    key_status_t scan_status = KEY_ERROR;
    TickType_t now_tick = 0;
    TickType_t threshold_ticks = 0;

    if (key_value == NULL)
    {
        return KEY_ERRORPARAMETER;
    }

    threshold_ticks = pdMS_TO_TICKS(long_press_threshold_ms);

    if (threshold_ticks == 0U)
    {
        return KEY_ERRORPARAMETER;
    }

    scan_status = key_scan(&stable_value);

    if ((scan_status != KEY_OK) &&
        (scan_status != KEY_ERRORTIMEOUT))
    {
        key_state = KEY_STATE_IDLE;
        return scan_status;
    }

    now_tick = xTaskGetTickCount();

    switch (key_state)
    {
        case KEY_STATE_IDLE:
            if (stable_value == KEY_PRESSED)
            {
                press_tick = now_tick;
                long_press_ticks = (TickType_t)threshold_ticks;
                key_state = KEY_STATE_PRESSED;
            }
            break;

        case KEY_STATE_PRESSED:
            if (stable_value == KEY_NOT_PRESSED)
            {
                if ((TickType_t)(now_tick - press_tick) >=
                    long_press_ticks)
                {
                    *key_value = KEY_LONG_PRESSED;
                }
                else
                {
                    *key_value = KEY_SHORT_PRESSED;
                }

                key_state = KEY_STATE_IDLE;
                return KEY_OK;
            }
            break;

        default:
            key_state = KEY_STATE_IDLE;
            return KEY_ERROR;
    }

    return KEY_ERRORTIMEOUT;
}
