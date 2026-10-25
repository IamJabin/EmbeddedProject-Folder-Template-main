/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led.c
 * 
 * @par dependencies 
 * - bsp_key.h
 * - stdio.h
 * - stdint.h
 * 
 * @author Jack | R&D Dept. | EternalChip ¡¢–æ«∂»Î Ω
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

#include "bsp_led.h"

//******************************** Defines **********************************//

//*********************Thread_Func **********************//
osThreadId_t              led_TaskHandle;
const osThreadAttr_t led_Task_attributes = {
  .name = "led_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
QueueHandle_t led_queue;

//*********************Queue_Handler ********************//



/**
 * @brief Instantiates the bsp_led_handler_t target.
 * 
 * Steps:
 *  1.doing the specific operations
 *  
 * @param[in] void 
 * 
 * @return led_handler_status_t : Status of the function.
 * 
 * */
led_status_t led_on_off(led_operation_t led_operation)
{
    led_status_t ret = LED_OK;
    
    if ( LED_ON            == led_operation )
    {
        //1. Make the LED ON
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); 
    }
    
    if ( LED_OFF           == led_operation )
    {
        //1. Make the LED OFF
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }
    
    if ( LED_TOGGLE        == led_operation )
    {
        //1. Make the LED toggle
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
    if ( LED_BLINK_3_TIMES == led_operation )
    {
        //1. Make the LED blink 3 times
        for(int i = 0; i < 6; i++)
        {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            vTaskDelay(300);
        }
    }

    return ret;
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
void led_task_func(void *argument)
{
    
  led_status_t      led_ret          =          LED_OK;
  led_operation_t led_value          =          LED_ON;

  led_queue = xQueueCreate( 10, sizeof( led_operation_t ) );
  if (NULL == led_queue )
  {
      printf("led_queue created failed \r\n");
  } 
  else
  {
      printf("led_queue created successfully \r\n");
  }
  for(;;)
  {
    printf("Hello led thread\r\n");
    
    if( led_queue != 0 )
	{
		// Receive a message on the created queue.  Block for 10 ticks if a
		// message is not immediately available.
		if( pdTRUE == xQueueReceive(                led_queue, 
                                               &( led_value ), 
                                        ( TickType_t ) 0 ) )
		{
			// pcRxedMessage now points to the struct AMessage variable posted
			// by vATask.
            printf("received led_queue value = [%d] at time [%d] \r\n " , 
                                                                     led_value,
                                                                HAL_GetTick());
            led_ret = led_on_off(led_value);
            if ( LED_OK == led_ret )
            {
                printf("led_on_off successfully at time [%d] \r\n", \
                                                               HAL_GetTick() );
            }
		}
	}
    
    osDelay(100);
  }
  /* USER CODE END 5 */
}


//******************************** Defines **********************************//
