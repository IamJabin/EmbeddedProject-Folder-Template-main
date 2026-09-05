/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_key.h
 * 
 * @par dependencies 
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
 *************************************************************************/

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

//******************************** Includes *********************************//


#include <stdint.h>               // 编译器提供的通用库包含部分
#include <stdio.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

//******************************** Includes *********************************//
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "queue.h"


//******************************** Defines **********************************//
#define Key_Pin       GPIO_PIN_0
#define Key_GPIO_Port GPIOA

extern QueueHandle_t key_queue;

/*  函数返回状态枚举                    */
typedef enum
{
  KEY_OK                = 0,           /* Operation completed successfully.  */
  KEY_ERROR             = 1,           /* Run-time error without case matched*/
  KEY_ERRORTIMEOUT      = 2,           /* Operation failed with timeout      */
  KEY_ERRORRESOURCE     = 3,           /* Resource not available.            */
  KEY_ERRORPARAMETER    = 4,           /* Parameter error.                   */
  KEY_ERRORNOMEMORY     = 5,           /* Out of memory.                     */
  KEY_ERRORISR          = 6,           /* Not allowed in ISR context         */
  KEY_RESERVED          = 0x7FFFFFFF   /* Reserved                           */
} key_status_t;

typedef enum
{
  KEY_PRESSED           = 0,           /* Operation completed successfully.  */
  KEY_NOT_PRESSED       = 1,           /* Run-time error without case matched*/
} key_press_status_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

key_status_t key_gpio_init(void);
key_status_t key_thread_init(void);
key_status_t key_scan(key_press_status_t * key_value);
extern void key_task_func(void *argument);

//******************************** Declaring ********************************//


#endif // End of __BSP_KEY_H__
