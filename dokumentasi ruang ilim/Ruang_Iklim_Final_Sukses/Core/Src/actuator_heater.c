#include "actuator_heater.h"
#include "pinmap.h"
#include "system_status.h"
#include "debug_uart.h"
#include <string.h>

static bool heater_current_state = false;
static bool heater_requested_state = false;
static uint32_t heater_last_change_time = 0;

#define HEATER_MIN_ON_TIME_MS    2000
#define HEATER_MIN_OFF_TIME_MS   3000

void Actuator_Heater_Init(void) {
    HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_RESET);
    heater_current_state = false;
    heater_requested_state = false;
    heater_last_change_time = HAL_GetTick();
    sys_status.heater_on = false;

    Debug_Print("[HEATER] Initialized (OFF)\r\n");
}

int Actuator_Heater_Set(bool state) {
    uint32_t current_time = HAL_GetTick();
    uint32_t time_since_change = current_time - heater_last_change_time;

    if (state && !heater_current_state) {
        if (time_since_change < HEATER_MIN_OFF_TIME_MS) {
            Debug_Printf("[HEATER] Short-cycle blocked: %lums\r\n", time_since_change);
            return -1;
        }
    }
    else if (!state && heater_current_state) {
        if (time_since_change < HEATER_MIN_ON_TIME_MS) {
            Debug_Printf("[HEATER] Short-cycle blocked: %lums\r\n", time_since_change);
            return -1;
        }
    }
    else {
        return 0;
    }

    heater_requested_state = state;
    Debug_Printf("[HEATER] Request: %s\r\n", state ? "ON" : "OFF");
    return 0;
}

bool Actuator_Heater_Get(void) {
    return heater_current_state;
}

void Actuator_Heater_Task(void) {
    static uint32_t last_task_time = 0;
    uint32_t current_time = HAL_GetTick();

    if ((current_time - last_task_time) < 100) return;
    last_task_time = current_time;

    if (heater_requested_state != heater_current_state) {
        uint32_t time_since_change = current_time - heater_last_change_time;

        if (heater_requested_state) {
            if (time_since_change >= HEATER_MIN_OFF_TIME_MS) {
                HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_SET);
                heater_current_state = true;
                heater_last_change_time = current_time;
                sys_status.heater_on = true;
                Debug_Print("[HEATER] Turned ON\r\n");
            }
        }
        else {
            if (time_since_change >= HEATER_MIN_ON_TIME_MS) {
                HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_RESET);
                heater_current_state = false;
                heater_last_change_time = current_time;
                sys_status.heater_on = false;
                Debug_Print("[HEATER] Turned OFF\r\n");
            }
        }
    }
}
