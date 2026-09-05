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
 * @author Bin
 *
 * @brief Provide the HAL APIs of led and corresponding opetions.
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

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

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
#define LED_Pin       GPIO_PIN_13
#define LED_GPIO_Port GPIOC

extern QueueHandle_t led_queue;
/*  函数返回状态枚举                    */
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
  LED_OFF               = 0,           /* LED is off */
  LED_ON                = 1,           /* LED is on */
  LED_TOGGLE            = 2,            /* LED is toggled */
  LED_TOGGLE_3_TIMES   = 3            /* LED is toggled 3 times */
} led_light_status_t;
//******************************** Defines **********************************//

//******************************** Declaring ********************************//
led_status_t led_gpio_init(void);
led_status_t led_thread_init(void);

//******************************** Declaring ********************************//


#endif // End of __BSP_LED_H__
