#include "ir_transmitter.h"
#include "pinmap.h"
#include "ir_commands.h"
#include <string.h>
#include <stdio.h>

// ===== PRIVATE VARIABLES =====
static volatile uint8_t ir_busy = 0;
static uint8_t dwt_initialized = 0;

// ===== PRIVATE FUNCTIONS =====
static void IR_Init_DWT(void) {
    if (!dwt_initialized) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        dwt_initialized = 1;
    }
}

static void IR_Delay_us(uint32_t microseconds) {
    IR_Init_DWT();

    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = microseconds * 16;

    while ((DWT->CYCCNT - start) < cycles) {
        __NOP();
    }
}

static void IR_Send_Pulse(uint32_t duration_us, GPIO_TypeDef* port, uint16_t pin) {
    uint32_t total_cycles = duration_us / 26;

    for (uint32_t i = 0; i < total_cycles; i++) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
        uint32_t on_start = DWT->CYCCNT;
        while ((DWT->CYCCNT - on_start) < (9 * 16)) { __NOP(); }

        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        uint32_t off_start = DWT->CYCCNT;
        while ((DWT->CYCCNT - off_start) < (17 * 16)) { __NOP(); }
    }
}

static void IR_Send_Space(uint32_t duration_us) {
    IR_Delay_us(duration_us);
}

// ===== PUBLIC FUNCTIONS =====
void IR_Transmitter_Init(void) {
    IR_Init_DWT();
    ir_busy = 0;
    HAL_GPIO_WritePin(IR_AC_PORT, IR_AC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IR_SOUND_PORT, IR_SOUND_PIN, GPIO_PIN_SET);
}

void IR_Send_Raw(const int32_t *timings, uint16_t length, GPIO_TypeDef* port, uint16_t pin) {
    if (ir_busy) return;
    ir_busy = 1;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t start_time = HAL_GetTick();

    for (uint16_t i = 0; i < length; i++) {
        int32_t timing = timings[i];
        if ((HAL_GetTick() - start_time) > 200) break;

        if (timing > 0) {
            IR_Send_Pulse((uint32_t)timing, port, pin);
        } else {
            IR_Send_Space((uint32_t)(-timing));
        }
    }

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    __set_PRIMASK(primask);
    ir_busy = 0;
}

void IR_Send_AC_Command(uint8_t temperature, uint8_t power_on) {
    (void)temperature;
    (void)power_on;
    // Placeholder for now
}

void IR_Send_Sound_Command(uint8_t command) {
    (void)command;
    // Placeholder for now
}
