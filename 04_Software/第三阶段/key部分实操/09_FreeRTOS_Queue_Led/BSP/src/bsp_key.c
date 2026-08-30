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

key_status_t key_scan(key_press_status_t * key_value)
{
    uint32_t counter = 0;
    key_press_status_t key_value_temp = KEY_NOT_PRESSED;

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
