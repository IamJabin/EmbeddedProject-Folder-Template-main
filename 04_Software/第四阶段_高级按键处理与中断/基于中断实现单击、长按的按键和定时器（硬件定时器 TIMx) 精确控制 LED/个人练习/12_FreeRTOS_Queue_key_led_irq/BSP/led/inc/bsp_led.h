/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_led.h
 * 
 * @par dependencies 
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
 * @version V1.0 2023-12-03
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

//******************************** Includes *********************************//
#include <stdint.h>               // the compiling lib.
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
//*********************Thread_Func **********************//
extern osThreadId_t led_TaskHandle;
extern const osThreadAttr_t led_Task_attributes;
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t led_queue;

//*********************Queue_Handler ********************//
/*  function resturn status          */
typedef enum
{
  LED_OK                = 0,           /* Operation completed successfully.  */
  LED_ERROR             = 1,           /* Run-time error without case matched*/
  LED_ERRORTIMEOUT      = 2,           /* Operation failed with timeout      */
  LED_ERRORRESOURCE     = 3,           /* Resource not available.            */
  LED_ERRORPARAMETER    = 4,           /* Parameter error.                   */
  LED_ERRORNOMEMORY     = 5,           /* Out of memory.                     */
  LED_ERRORISR          = 6,           /* Not allowed in ISR context         */
  LED_RESERVED          = 0x7FFFFFFF   /* Reserved                           */
} led_status_t;

typedef enum
{
  LED_ON                = 0,           /* Operation completed successfully.  */
  LED_OFF               = 1,           /* Operation completed successfully.  */
  LED_TOGGLE            = 2,           /* Operation completed successfully.  */
  LED_BLINK_3_TIMES     = 3,           /* Operation completed successfully.  */
  LED_INITED_VALUE      = 0xFF         /* Inited value                    .  */
} led_operation_t;

//******************************** Defines **********************************//

//******************************** Declaring ********************************//
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
led_status_t led_on_off(led_operation_t led_operation);


/**
 * @brief led_task_func for thread to run.
 * 
 * Steps:
 *  1.doing the specific operations
 *  
 * @param[in] void 
 * 
 * @return led_handler_status_t : Status of the function.
 * 
 * */
void         led_task_func                 (void *argument);

//******************************** Declaring ********************************//




#endif // End of __BSP_LED_H__

