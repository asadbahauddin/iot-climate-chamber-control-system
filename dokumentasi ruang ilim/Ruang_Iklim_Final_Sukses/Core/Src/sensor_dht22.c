#include "sensor_dht22.h"
#include "pinmap.h"
#include "system_status.h"
#include "debug_uart.h"
#include <string.h>

// ========== PRIVATE VARIABLES ==========
static uint32_t dht_last_read_time = 0;
static DHT_Status_t dht_status = DHT_ERROR_NO_RESPONSE;

// ========== PRIVATE FUNCTIONS ==========

// Simple microsecond delay (approximate)
static void DHT22_Delay_us(uint32_t microseconds) {
    uint32_t loops = microseconds * (SystemCoreClock / 1000000) / 4;
    for(volatile uint32_t i = 0; i < loops; i++) {
        __NOP();
    }
}

// ========== PUBLIC FUNCTIONS ==========

void Sensor_DHT22_Init(void) {
    // Initialize as output high (idle state)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);

    dht_last_read_time = HAL_GetTick() - 2500; // Allow immediate read
    dht_status = DHT_ERROR_NO_RESPONSE;

    Debug_Print("[DHT22] Initialized (simple debug mode)\r\n");
}

bool Sensor_DHT22_Read_Now(void) {
    uint32_t current_time = HAL_GetTick();

    // Check minimum interval (2 seconds)
    if ((current_time - dht_last_read_time) < 2000) {
        Debug_Print("[DHT22] Too soon, wait 2s\r\n");
        return false;
    }

    Debug_Print("\r\n=== DHT22 DEBUG READ ===\r\n");

    uint8_t data[5] = {0};

    // ====== STEP 1: SEND START SIGNAL ======
    Debug_Print("1. Setting output LOW (1ms)...\r\n");

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);  // 1ms exactly

    // ====== STEP 2: RELEASE BUS ======
    Debug_Print("2. Releasing bus HIGH...\r\n");
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);

    // Short delay ~30µs
    DHT22_Delay_us(30);

    // ====== STEP 3: SWITCH TO INPUT ======
    Debug_Print("3. Switching to INPUT...\r\n");
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);

    // ====== STEP 4: CHECK RESPONSE ======
    Debug_Print("4. Waiting for DHT22 response...\r\n");

    // Wait for LOW (max 100µs)
    uint32_t timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
        timeout++;
        if (timeout > 2000) {  // ~100µs @16MHz
            Debug_Print("   ERROR: No LOW response (timeout)\r\n");
            Debug_Print("   Check: 1) 5V power 2) Pull-up 4.7kΩ 3) Wiring\r\n");

            // Check pin state
            GPIO_PinState state = HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN);
            Debug_Printf("   Pin is %s (should be LOW)\r\n",
                        state == GPIO_PIN_SET ? "HIGH" : "LOW");

            // Restore idle
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);
            HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);

            dht_status = DHT_ERROR_NO_RESPONSE;
            return false;
        }
    }

    Debug_Print("   ✓ Got LOW response\r\n");

    // Wait for HIGH (max 100µs)
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_RESET) {
        timeout++;
        if (timeout > 2000) {
            Debug_Print("   ERROR: Stuck LOW\r\n");
            goto cleanup;
        }
    }

    Debug_Print("   ✓ Got HIGH response\r\n");

    // ====== STEP 5: READ 40 BITS ======
    Debug_Print("5. Reading 40 bits of data...\r\n");

    for (int byte = 0; byte < 5; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            // Wait for LOW (each bit starts with 50µs LOW)
            timeout = 0;
            while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
                if (timeout++ > 1000) {
                    Debug_Printf("   ERROR: Bit %d timeout (wait LOW)\r\n", bit);
                    goto cleanup;
                }
            }

            // Wait for HIGH
            timeout = 0;
            while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_RESET) {
                if (timeout++ > 1000) {
                    Debug_Printf("   ERROR: Bit %d timeout (wait HIGH)\r\n", bit);
                    goto cleanup;
                }
            }

            // Measure HIGH pulse width
            uint32_t start = DWT->CYCCNT;
            while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
                if ((DWT->CYCCNT - start) > 2000) {  // ~125µs max
                    break;
                }
            }
            uint32_t pulse_width = DWT->CYCCNT - start;

            // Bit 0: 26-28µs, Bit 1: 70µs (16MHz = 16 cycles/µs)
            // Threshold at 40µs = 640 cycles
            if (pulse_width > 640) {
                data[byte] |= (1 << (7 - bit));
            }
        }
    }

    // ====== STEP 6: PRINT RAW DATA ======
    Debug_Printf("6. Raw data: %02X %02X %02X %02X %02X\r\n",
                data[0], data[1], data[2], data[3], data[4]);

    // ====== STEP 7: VERIFY CHECKSUM ======
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];

    if (checksum != data[4]) {
        Debug_Printf("7. Checksum FAIL: %d != %d\r\n", checksum, data[4]);
        dht_status = DHT_ERROR_CHECKSUM;
        goto cleanup;
    }

    // ====== STEP 8: CONVERT DATA ======
    int16_t hum_raw = (data[0] << 8) | data[1];
    int16_t temp_raw = (data[2] << 8) | data[3];

    float temperature = temp_raw * 0.1f;
    // Handle negative temperature
    if (temp_raw & 0x8000) {
        temperature = -((temp_raw & 0x7FFF) * 0.1f);
    }

    float humidity = hum_raw * 0.1f;

    // ====== STEP 9: VALIDATE RANGE ======
    if (temperature < -40.0f || temperature > 80.0f ||
        humidity < 0.0f || humidity > 100.0f) {
        Debug_Printf("8. Range ERROR: %.1fC %.1f%%\r\n", temperature, humidity);
        dht_status = DHT_ERROR_CHECKSUM;
        goto cleanup;
    }

    // ====== STEP 10: UPDATE SYSTEM ======
    sys_status.temperature = temperature;
    sys_status.humidity = humidity;
    dht_last_read_time = current_time;
    dht_status = DHT_OK;

    Debug_Printf("9. SUCCESS: %.1f°C %.1f%%\r\n", temperature, humidity);
    Debug_Print("=== DHT22 READ COMPLETE ===\r\n");

cleanup:
    // ====== CLEANUP ======
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);

    return (dht_status == DHT_OK);
}

float Sensor_DHT22_Get_Temperature(void) {
    if (dht_status == DHT_OK) {
        return sys_status.temperature;
    }
    return -100.0f;
}

float Sensor_DHT22_Get_Humidity(void) {
    if (dht_status == DHT_OK) {
        return sys_status.humidity;
    }
    return -1.0f;
}

DHT_Status_t Sensor_DHT22_Get_Status(void) {
    return dht_status;
}

// ========== COMPATIBILITY FUNCTIONS ==========
void Sensor_DHT22_Task(void) {
    // Empty - not used in debug mode
}

bool Sensor_DHT22_Start_Measurement(void) {
    return Sensor_DHT22_Read_Now();
}
