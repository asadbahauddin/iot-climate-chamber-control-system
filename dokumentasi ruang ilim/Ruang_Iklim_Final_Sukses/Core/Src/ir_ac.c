#include "ir_ac.h"
#include "pinmap.h"
#include "stm32f4xx_hal.h"

void IR_AC_Init(void)
{
    // PNP transistor → OFF = GPIO HIGH
    HAL_GPIO_WritePin(IR_AC_PORT, IR_AC_PIN, GPIO_PIN_SET);
}

void IR_AC_Test_ON(void)
{
    // GPIO LOW → transistor ON → IR LED ON
    HAL_GPIO_WritePin(IR_AC_PORT, IR_AC_PIN, GPIO_PIN_RESET);
}

void IR_AC_Test_OFF(void)
{
    HAL_GPIO_WritePin(IR_AC_PORT, IR_AC_PIN, GPIO_PIN_SET);
}
