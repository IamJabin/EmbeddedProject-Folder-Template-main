/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
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
 * @author Jack | R&D Dept. | EternalChip 立芯嵌入式
 * 
 * @brief Provide the HAL APIs of Key and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2024-10-30
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#include "bsp_key_irq.h"

#define SHORT_PRESS_TIME 500U  /* 长短按阈值，单位 ms */

osThreadId_t key_TaskHandle;

QueueHandle_t key_queue;
QueueHandle_t inter_key_queue = NULL;

const osThreadAttr_t key_Task_attributes = {
  .name = "key_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


/**
 * @brief Instantiates the bsp_led_handler_t target.
 * 
 * Steps:
 *  1.doing the specific operations
 *  
 * @param[in] void 
 * 
 * @return KEY_OK            : the key has been pressed
 *         KEY_ERRORTIMEOUT  : the key has been released
 * */
key_status_t key_scan(key_press_status_t * key_value)
{
    key_press_status_t key_status_value = KEY_NOT_PRESSED;
    
    if(HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET)
    {
        key_status_value = KEY_PRESSED;
        *key_value = key_status_value;
        return KEY_OK;
    }

    *key_value = key_status_value;
    
    return KEY_ERRORTIMEOUT;//始终没有按键被按下，返回超时
}

/**
 * @brief this function will check the key status between long press and 
 *        shrot press.
 * 
 * Steps:
 *  1.doing the specific operations
 *  
 * @param[in] void 
 * 
 * @return KEY_SHORT_PRESSED : key has been pressed with short time.
 *         KEY_LONG_PRESSED  : key has been pressed with long time.
 * 
 * */
key_status_t key_scan_short_long_press(key_press_status_t *  key_value, 
                                       uint32_t      short_press_time)
{
    /**Variables */
    key_status_t       ret_key_status =          KEY_OK;    /* return value  */
    key_press_status_t key_value_temp = KEY_NOT_PRESSED;    /* temporary     */
    uint32_t             counter_tick =               0;    /* counter       */
    /**Variables */

    // 1. check if the key is pressed
    ret_key_status = key_scan(&key_value_temp);
    // TBD : critical section


    // 1.1 if the key is pressed, then check if it is short pressed
    if ( KEY_OK == ret_key_status )
    {
        if ( KEY_PRESSED == key_value_temp )
        {
            // get the timestamp of the first key press
            counter_tick = HAL_GetTick(); 

            // 1.1.1 check the short press time
            while( HAL_GetTick() <  counter_tick + short_press_time )
                ;
            
            // 1.1.2 now could check if the user pressed the key 
            ret_key_status = key_scan(&key_value_temp);
            
            if ( KEY_NOT_PRESSED == key_value_temp )
            {
                // 1.1.3 the key is pressed with short press
                *key_value = KEY_SHORT_PRESSED;
                return KEY_OK;
            } 
            else 
            {
                // 1.2 the key is pressed with long press
                *key_value = KEY_LONG_PRESSED;
                
                // 1.2.1 keep focuing on the status of key to avoid the short
                // press
                while( KEY_OK == key_scan(&key_value_temp) ) // key is pressed
                    ;
                
                return KEY_OK;
            }

        }
    }

    return ret_key_status;
}

/**
 * @brief key_task_func for thread to run.
 * 
 * Steps:
 *  1.doing the specific operations
 *  
 * @param[in] void 
 * 
 * @return led_handler_status_t : Status of the function.
 * 
 * */
void key_task_func(void *argument)
{
    /*
        待实现流程：

        1.  按键按下或松开，产生中断。
        1.1 在中断中根据标志位区分当前是下降沿还是上升沿：
            下降沿对应按下，上升沿对应松开。
            处理后更新标志位，并配置下一次的中断触发沿。
        1.2 获取事件发生时间 HAL_GetTick()，
            通过 inter_key_queue 将“边沿类型 + 发生时间”
            发送给 key task，中断中使用 FromISR 接口。

        2.  在 key task 中获取中断发来的队列消息。
        2.1 计划使用最大阻塞等待消息，没有消息时任务休眠。
            获取到消息说明发生了一个边沿事件，
            可能是按下、松开，也可能是抖动。
        2.2 收到按下事件时，保存按下时间。
            收到松开事件时，计算按下持续时间，
            按教程过滤过短的间隔，再判断是长按还是短按。
            打印事件发生时间和当前处理时间，方便观察。
        2.3 短按向 key_queue 发送 KEY_SHORT_PRESSED；
            长按向 key_queue 发送 KEY_LONG_PRESSED。
            按键结果消息暂时不需要附带时间。

        3.  附加练习：APP 获取 key_queue 中的按键结果。
        3.1 短按向 led_queue 发送 LED_TOGGLE；
            长按向 led_queue 发送 LED_BLINK_3_TIMES。

        4.  led task 等待 led_queue 中的消息。
            收到消息后调用 led_on_off 执行对应动作，
            并打印当前时间。
    */
    
    uint32_t event_index = 0;
    uint32_t frist_trigger_tick = 0;
    uint32_t short_press_time = SHORT_PRESS_TIME;
    key_press_event_t key_press_event =
    {
        .edge_type = RASING,
        .trigger_tick = 0
    };

    key_queue = xQueueCreate( 10, sizeof( key_press_status_t ) );
    inter_key_queue = xQueueCreate( 10, sizeof( key_press_event_t ) );
    if((NULL == key_queue) || (NULL == inter_key_queue))
    {
        printf("key_queue or inter_key_queue created failed \r\n");
    } 
    else
    {
        printf("key_queue and inter_key_queue created successfully \r\n");
    }

    for (;;)
    {
        /*
            1.  从 inter_key_queue 接收按键边沿事件。
                状态 0：等待按下。
                状态 1：已按下，等待松开。
            1.1 状态为 0，收到上升沿：
                没有对应的按下事件，丢弃，保持状态 0。
            1.2 状态为 0，收到下降沿：
                保存按下时间，切换到状态 1。
            1.3 状态为 1，收到上升沿：
                计算按下持续时间。
                小于消抖阈值则丢弃，否则判断长按或短按，
                向 key_queue 发送按键结果。
                本次处理结束，回到状态 0。
            1.4 状态为 1，再收到下降沿：
                忽略重复事件，保留第一次按下时间，保持状态 1。
        */
        printf("key_task_func running at [%d] tick\r\n", HAL_GetTick());

        if (pdTRUE == xQueueReceive(inter_key_queue, &key_press_event, 0))
        {
            printf("key_press_event.trigger_tick = [%d]\r\n",
                key_press_event.trigger_tick);
            printf("inter_key_queue receive key event at [%d] tick\r\n",
                HAL_GetTick());

            /* 在这里依次加入 3.3 的三个判断。 */
            if ((RASING == key_press_event.edge_type) && (0 == event_index))
            {
                printf("key RASING fetched! error! at [%d] tick\r\n", HAL_GetTick());
            }

            if ((FAILING == key_press_event.edge_type) && (0 == event_index))
            {
                printf("key FAILING fetched! first at [%d] tick\r\n", HAL_GetTick());
                event_index += 1;
                frist_trigger_tick = key_press_event.trigger_tick;
            }

            if ((RASING == key_press_event.edge_type) && (1 == event_index))
            {
                printf("key RASING after the falling \r\n");

                if (key_press_event.trigger_tick - frist_trigger_tick < 10)
                {
                    printf("Invalid key fetched at [%d] tick\r\n", HAL_GetTick());
                    continue;
                }

                /* 在这里加入 3.4 的短按、长按结果发送。 */
                if (key_press_event.trigger_tick - frist_trigger_tick <= short_press_time)
                {
                    key_press_status_t key_result = KEY_SHORT_PRESSED;

                    if (pdTRUE == xQueueSendToFront(key_queue, &key_result, 0))
                    {
                        printf("key_result send short press successfully at [%d] tick\r\n",
                            HAL_GetTick());
                        /* 阶段 10 再加入状态清零。 */
                        event_index = 0;
                    }
                    else
                    {
                        printf("key_result send short press failed at [%d] tick\r\n",
                            HAL_GetTick());
                    }
                }
                else
                {
                    key_press_status_t key_result = KEY_LONG_PRESSED;

                    if (pdTRUE == xQueueSendToFront(key_queue, &key_result, 0))
                    {
                        printf("key_result send long press successfully at [%d] tick\r\n",
                            HAL_GetTick());
                        /* 阶段 10 再加入状态清零。 */
                        event_index = 0;
                    }
                    else
                    {
                        printf("key_result send long press failed at [%d] tick\r\n",
                            HAL_GetTick());
                    }
                }
            }
        }
        osDelay(100);
    }
}

#if 0
void key_task_func(void *argument)
{
    
  key_status_t  ket_ret          =          KEY_OK;
  key_press_status_t key_statues = KEY_NOT_PRESSED;
  key_queue = xQueueCreate( 10, sizeof( uint32_t ) );
  uint32_t counter_tick = 0;
  if (NULL == key_queue )
  {
      printf("key_queue created failed \r\n");
  } 
  else
  {
      printf("key_queue created successfully \r\n");
  }
  for(;;)
  {
    counter_tick++;
    //printf("Hellow Key thread\r\n");
    
    ket_ret = key_scan(&key_statues);
    
    if( KEY_OK == ket_ret)
    {
        if ( KEY_PRESSED == key_statues )
        {
            printf("Key_Pressed\r\n");
            if ( pdTRUE == xQueueSendToFront(key_queue,&counter_tick,0))
            {
                printf("send successfully\r\n");
            }
            
        }
    }
    if( KEY_OK != ket_ret)
    {
        printf("Key_not_Pressed\r\n");
    }        
    osDelay(100);
  }
  /* USER CODE END 5 */
}
#endif


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken;
    static key_trigger_edge_t expected_edge = FAILING;

    if (GPIO_Pin != Key_Pin)
    {
        return;
    }

    if (NULL == inter_key_queue)
    {
        return;
    }

    if (FAILING == expected_edge)
    {
        key_press_event_t key_press_event =
        {
            .edge_type = FAILING,
            .trigger_tick = HAL_GetTick()
        };
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        if (pdTRUE == xQueueSendToBackFromISR(inter_key_queue,
                                               &key_press_event,
                                               &xHigherPriorityTaskWoken))
        {
            printf("FAILING send successfully at [%d] tick \r\n", HAL_GetTick());
        }
        else
        {
            printf("FAILING send failed at [%d] tick \r\n", HAL_GetTick());
        }

        expected_edge = RASING;

        GPIO_InitStruct.Pin = Key_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(Key_GPIO_Port, &GPIO_InitStruct);
    }
    else if (RASING == expected_edge)
    {
        key_press_event_t key_press_event =
        {
            .edge_type = RASING,
            .trigger_tick = HAL_GetTick()
        };
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        if (pdTRUE == xQueueSendToBackFromISR(inter_key_queue,
                                               &key_press_event,
                                               &xHigherPriorityTaskWoken))
        {
            printf("RASING send successfully at [%d] tick \r\n", HAL_GetTick());
        }
        else
        {
            printf("RASING send failed at [%d] tick \r\n", HAL_GetTick());
        }

        expected_edge = FAILING;

        GPIO_InitStruct.Pin = Key_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(Key_GPIO_Port, &GPIO_InitStruct);
    }
}