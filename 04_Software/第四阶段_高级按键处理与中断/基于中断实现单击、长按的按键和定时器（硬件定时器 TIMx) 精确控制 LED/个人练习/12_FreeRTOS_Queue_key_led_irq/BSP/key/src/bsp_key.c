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

#include "bsp_key.h"
osThreadId_t key_TaskHandle;

QueueHandle_t key_queue;

const osThreadAttr_t key_Task_attributes = {
  .name = "key_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
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

