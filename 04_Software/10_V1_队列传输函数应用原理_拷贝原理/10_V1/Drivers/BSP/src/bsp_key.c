#include "bsp_key.h"

key_state_t key_Scan(void)
{
    if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET)
    {
        HAL_Delay(50);
        if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET)
        {
            return KEY_PRESS;
        }
        else
        {
            return KEY_RELEASE;
        }
    }
    else
    {
        HAL_Delay(5);
        return KEY_RELEASE;
    }
}
