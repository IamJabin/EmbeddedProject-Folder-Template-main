/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_key_irq.h
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

#ifndef __BSP_KEY_IRQ_H__
#define __BSP_KEY_IRQ_H__

//******************************** Includes *********************************//


#include <stdint.h>               //  the compiling lib.
#include <stdio.h>

#include "main.h"                 //  Core / OS layer
#include "cmsis_os.h"


#include "queue.h"                //  specific file for operations
#include "stm32f4xx_hal.h"        
#include "stm32f4xx_hal_gpio.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//

#define SHORT_PRESS_TIME    (500)              // Specify the short press time
#define KEY_CALLBACK        void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)

//*********************Thread_Func **********************//
extern osThreadId_t key_TaskHandle;
extern const osThreadAttr_t key_Task_attributes;
//*********************Thread_Func **********************//

//*********************Queue_Handler ********************//
extern QueueHandle_t              key_queue;
extern QueueHandle_t        inter_key_queue;
//*********************Queue_Handler ********************//

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
  KEY_SHORT_PRESSED     = 2,           /* Operation                          */
  KEY_LONG_PRESSED      = 3,           /* Operation                          */
} key_press_status_t;

typedef enum
{
  RASING                = 0,           /* Operation completed successfully.  */ 
  FAILING               = 1,           /* Operation failed                   */
} key_trigger_edge_t;

typedef struct
{
  key_trigger_edge_t    edge_type;     /* Operation completed successfully.  */
  uint32_t           trigger_tick;     /* Operation completed successfully.  */
} key_press_event_t;

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
key_status_t key_scan      (key_press_status_t * key_value);

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
                                       uint32_t      short_press_time);

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
void         key_task_func                 (void *argument);





//******************************** Declaring ********************************//




#endif // End of __BSP_KEY_IRQ_H__

