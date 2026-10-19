#include "actuator_dimmer.h"
#include "pinmap.h"
#include "system_status.h"
#include "debug_uart.h"
#include <string.h>

static volatile uint8_t dimmer1_percent = 0;
static volatile uint8_t dimmer2_percent = 0;
static volatile bool zero_cross_detected = false;
static volatile uint32_t last_zero_cross_time = 0;
static volatile uint32_t zero_cross_count = 0;

void Actuator_Dimmer_Init(void) {
    // Initialize DWT for microsecond timing
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    
    // Set TRIAC pins to OFF (HIGH for optocoupler)
    HAL_GPIO_WritePin(DIMMER_CH1_PORT, DIMMER_CH1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIMMER_CH2_PORT, DIMMER_CH2_PIN, GPIO_PIN_SET);
    
    dimmer1_percent = 0;
    dimmer2_percent = 0;
    zero_cross_detected = false;
    zero_cross_count = 0;
    
    sys_status.dimmer1_percent = 0;
    sys_status.dimmer2_percent = 0;
    
    Debug_Print("[DIMMER] Initialized (Zero-Cross Mode)\r\n");
}

static void Dimmer_Fire_TRIAC(uint8_t channel, uint32_t delay_us) {
    GPIO_TypeDef* port;
    uint16_t pin;
    
    if (channel == 1) {
        port = DIMMER_CH1_PORT;
        pin = DIMMER_CH1_PIN;
    } else {
        port = DIMMER_CH2_PORT;
        pin = DIMMER_CH2_PIN;
    }
    
    // Microsecond delay using DWT
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = delay_us * 16; // 16MHz = 16 cycles per us
    
    while ((DWT->CYCCNT - start) < cycles) {
        __NOP();
    }
    
    // Fire TRIAC (ACTIVE LOW for optocoupler)
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    
    // Hold for minimum triac latching time (50us)
    start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < (50 * 16)) {
        __NOP();
    }
    
    // Turn off trigger
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void Actuator_Dimmer_Set(uint8_t channel, uint8_t percent) {
    if (percent > 100) percent = 100;
    
    if (channel == 1) {
        dimmer1_percent = percent;
        sys_status.dimmer1_percent = percent;
        Debug_Printf("[DIMMER] CH1 set to %d%%\r\n", percent);
    } else {
        dimmer2_percent = percent;
        sys_status.dimmer2_percent = percent;
        Debug_Printf("[DIMMER] CH2 set to %d%%\r\n", percent);
    }
}

uint8_t Actuator_Dimmer_Get(uint8_t channel) {
    return (channel == 1) ? dimmer1_percent : dimmer2_percent;
}

void Actuator_Dimmer_Task(void) {
    static uint32_t last_task_time = 0;
    uint32_t now = HAL_GetTick();
    
    // Run every 10ms
    if ((now - last_task_time) < 10) return;
    last_task_time = now;
    
    // This task is minimal - real work happens in ISR
}

void Actuator_Dimmer_ZeroCross_Callback(void) {
    // This is called from EXTI0_IRQHandler in stm32f4xx_it.c
    static uint32_t last_zc_time = 0;
    uint32_t now = DWT->CYCCNT;
    
    // Debounce: minimal 5ms between zero-cross (50Hz = 10ms per half)
    if ((now - last_zc_time) > (5000 * 16)) {
        zero_cross_detected = true;
        last_zero_cross_time = HAL_GetTick();
        zero_cross_count++;
        
        // Calculate firing delays based on percentage
        // 100% = fire immediately, 0% = never fire
        // 50Hz AC: half-cycle = 10,000us
        
        // CHANNEL 1
        if (dimmer1_percent > 0) {
            uint32_t delay_us = (100 - dimmer1_percent) * 100; // 0-100% -> 10000-0us
            if (delay_us > 0) {
                Dimmer_Fire_TRIAC(1, delay_us);
            }
        }
        
        // CHANNEL 2
        if (dimmer2_percent > 0) {
            uint32_t delay_us = (100 - dimmer2_percent) * 100;
            if (delay_us > 0) {
                Dimmer_Fire_TRIAC(2, delay_us);
            }
        }
        
        last_zc_time = now;
    }
}