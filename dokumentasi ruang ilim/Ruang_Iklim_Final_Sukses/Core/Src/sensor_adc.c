#include "sensor_adc.h"
#include "pinmap.h"
#include "system_status.h"
#include "debug_uart.h"
#include <string.h>

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

static volatile uint16_t adc_buffer[2] = {0};
static uint32_t last_adc_update = 0;

void Sensor_ADC_Init(void) {
    memset((void*)adc_buffer, 0, sizeof(adc_buffer));
    last_adc_update = 0;

    // Start ADC with DMA
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 2) != HAL_OK) {
        Debug_Print("[ADC] DMA Start Error\r\n");
    }

    sys_status.light_level = 0;
    sys_status.sound_level = 0;

    Debug_Print("[ADC] Initialized (DMA mode)\r\n");
}

void Sensor_ADC_Start(void) {
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 2) != HAL_OK) {
        Debug_Print("[ADC] Restart Error\r\n");
    }
}

void Sensor_ADC_Task(void) {
    static uint32_t last_task_time = 0;
    uint32_t current_time = HAL_GetTick();

    // Update every 100ms
    if ((current_time - last_task_time) < 100) return;
    last_task_time = current_time;

    // Update system status
    sys_status.light_level = adc_buffer[0];
    sys_status.sound_level = adc_buffer[1];

    last_adc_update = current_time;
}

uint16_t Sensor_ADC_Get_Light(void) {
    return adc_buffer[0];
}

uint16_t Sensor_ADC_Get_Sound(void) {
    return adc_buffer[1];
}

// DMA Complete Callback (called from HAL)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        // DMA transfer complete, data is already in adc_buffer
    }
}
