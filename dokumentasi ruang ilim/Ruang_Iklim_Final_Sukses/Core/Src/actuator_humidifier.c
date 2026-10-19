#include "actuator_humidifier.h"
#include "pinmap.h"
#include "system_status.h"
#include "debug_uart.h"

extern TIM_HandleTypeDef htim4;

static uint8_t humidifier_pwm = 0;

void Actuator_Humidifier_Init(void) {
    // Start PWM timer
    HAL_TIM_PWM_Start(&htim4, HUMIDIFIER_TIM_CH);

    // Set initial duty cycle to 0%
    __HAL_TIM_SET_COMPARE(&htim4, HUMIDIFIER_TIM_CH, 0);

    humidifier_pwm = 0;
    sys_status.humidifier_pwm = 0;

    Debug_Print("[HUMIDIFIER] Initialized (PWM)\r\n");
}

void Actuator_Humidifier_Set(uint8_t pwm_percent) {
    if (pwm_percent > 100) pwm_percent = 100;

    humidifier_pwm = pwm_percent;
    sys_status.humidifier_pwm = pwm_percent;

    // Convert percentage to PWM compare value
    // TIM4 period = 1000, so 100% = 1000
    uint32_t compare_value = (pwm_percent * 1000) / 100;
    __HAL_TIM_SET_COMPARE(&htim4, HUMIDIFIER_TIM_CH, compare_value);

    Debug_Printf("[HUMIDIFIER] Set to %d%% (CCR=%lu)\r\n", pwm_percent, compare_value);
}

uint8_t Actuator_Humidifier_Get(void) {
    return humidifier_pwm;
}

void Actuator_Humidifier_Task(void) {
    // Currently just maintains state
    // Could add: soft-start, protection logic, etc.
}
