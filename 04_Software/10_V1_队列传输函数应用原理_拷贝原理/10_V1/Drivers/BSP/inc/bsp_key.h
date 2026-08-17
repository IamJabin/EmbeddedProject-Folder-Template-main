#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "main.h"

typedef enum
{
    KEY_PRESS,
    KEY_RELEASE
} key_state_t;

typedef enum{
    KEY_NO_PRESS,
    KEY_SHORT_PRESS,
    KEY_LONG_PRESS,
}key_result_t;

typedef enum{
    No_Check,
    Check_During,
    Check_Complete,
}key_check_state_t;

typedef struct 
{
    uint32_t triggle_tick;
    key_state_t state;
    key_check_state_t checkstate;
}key_event_t;

key_state_t key_Scan(void);

#endif  //__BSP_KEY_H
