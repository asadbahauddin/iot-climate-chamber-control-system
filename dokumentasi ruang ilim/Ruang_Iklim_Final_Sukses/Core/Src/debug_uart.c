#include "debug_uart.h"
#include "pinmap.h"
#include "system_status.h"
#include "actuator_heater.h"
#include "actuator_dimmer.h"
#include "actuator_humidifier.h"
#include "sensor_adc.h"
#include "sensor_dht22.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart2;

static uint8_t rx_buffer[128];
static uint16_t rx_index = 0;

void Debug_UART_Init(void) {
    rx_index = 0;
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

void Debug_Print(const char *message) {
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), 100);
}

void Debug_Printf(const char *format, ...) {
    char buffer[128];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Debug_Print(buffer);
}

static void Debug_Execute_Command(const char *command) {
    Debug_Printf("\r\n> %s\r\n", command);

    char cmd[32];
    char param1[32] = "";
    char param2[32] = "";

    sscanf(command, "%31s %31s %31s", cmd, param1, param2);

    if (strcmp(cmd, "HELP") == 0) {
        Debug_Show_Help();
    }
    else if (strcmp(cmd, "STATUS") == 0) {
        Debug_Show_Status();
    }
    else if (strcmp(cmd, "SENSORS") == 0) {
        Debug_Show_All_Sensors();
    }
    else if (strcmp(cmd, "ACTUATORS") == 0) {
        Debug_Show_All_Actuators();
    }
    else if (strcmp(cmd, "H0") == 0) {
        if (Actuator_Heater_Set(false) == 0) {
            Debug_Print("[CMD] Heater OFF\r\n");
        }
    }
    else if (strcmp(cmd, "H1") == 0) {
        if (Actuator_Heater_Set(true) == 0) {
            Debug_Print("[CMD] Heater ON\r\n");
        }
    }
    else if (strcmp(cmd, "D1") == 0) {
        if (strlen(param1) > 0) {
            int percent = atoi(param1);
            if (percent >= 0 && percent <= 100) {
                Actuator_Dimmer_Set(1, percent);
                Debug_Printf("[CMD] Dimmer1: %d%%\r\n", percent);
            }
        } else {
            Debug_Print("[CMD] Usage: D1 0-100\r\n");
        }
    }
    else if (strcmp(cmd, "D2") == 0) {
        if (strlen(param1) > 0) {
            int percent = atoi(param1);
            if (percent >= 0 && percent <= 100) {
                Actuator_Dimmer_Set(2, percent);
                Debug_Printf("[CMD] Dimmer2: %d%%\r\n", percent);
            }
        } else {
            Debug_Print("[CMD] Usage: D2 0-100\r\n");
        }
    }
    else if (strcmp(cmd, "HUM") == 0) {
        if (strlen(param1) > 0) {
            int pwm = atoi(param1);
            if (pwm >= 0 && pwm <= 100) {
                Actuator_Humidifier_Set(pwm);
                Debug_Printf("[CMD] Humidifier: %d%%\r\n", pwm);
            }
        } else {
            Debug_Print("[CMD] Usage: HUM 0-100\r\n");
        }
    }
    else if (strcmp(cmd, "LDR") == 0) {
        Debug_Printf("[SENSOR] Light: %d\r\n", Sensor_ADC_Get_Light());
    }
    else if (strcmp(cmd, "MIC") == 0) {
        Debug_Printf("[SENSOR] Sound: %d\r\n", Sensor_ADC_Get_Sound());
    }
    else if (strcmp(cmd, "TEMP") == 0) {
        float temp = Sensor_DHT22_Get_Temperature();
        if (temp > -100) {
            Debug_Printf("[SENSOR] Temp: %.1f°C\r\n", temp);
        } else {
            Debug_Print("[SENSOR] Temperature not available\r\n");
        }
    }
    else if (strcmp(cmd, "HUMID") == 0) {
        float hum = Sensor_DHT22_Get_Humidity();
        if (hum >= 0) {
            Debug_Printf("[SENSOR] Humidity: %.1f%%\r\n", hum);
        } else {
            Debug_Print("[SENSOR] Humidity not available\r\n");
        }
    }
    else if (strcmp(cmd, "ALL") == 0) {
        Debug_Show_All_Sensors();
        Debug_Show_All_Actuators();
    }
    else if (strcmp(cmd, "RESET") == 0) {
        Debug_Print("[CMD] Software reset\r\n");
        HAL_NVIC_SystemReset();
    }
    else if (strcmp(cmd, "ESP32") == 0) {
        Debug_Print("[ESP32] Simulating ESP32 connection\r\n");
        sys_status.esp32_connected = true;
        sys_status.target_temp = 25.0f;
        sys_status.target_humidity = 50.0f;
        Debug_Print("[ESP32] Setpoint: 25°C, 50% RH\r\n");
    }
    else {
        Debug_Printf("[ERR] Unknown command: %s\r\n", command);
        Debug_Print("Type 'HELP' for commands\r\n");
    }
}

void Debug_Process_Commands(void) {
    uint8_t byte;

    if (HAL_UART_Receive(&huart2, &byte, 1, 0) == HAL_OK) {
        if (byte == '\r' || byte == '\n') {
            if (rx_index > 0) {
                rx_buffer[rx_index] = '\0';
                Debug_Execute_Command((char*)rx_buffer);
                rx_index = 0;
                memset(rx_buffer, 0, sizeof(rx_buffer));
                Debug_Print("> ");
            }
        }
        else if (rx_index < (sizeof(rx_buffer) - 1)) {
            rx_buffer[rx_index++] = byte;
        }
    }
}

void Debug_Show_Help(void) {
    Debug_Print("\r\n=== COMMANDS ===\r\n");
    Debug_Print("HELP       - This menu\r\n");
    Debug_Print("STATUS     - System status\r\n");
    Debug_Print("SENSORS    - Read all sensors\r\n");
    Debug_Print("ACTUATORS  - Check all actuators\r\n");
    Debug_Print("ALL        - Sensors + Actuators\r\n");
    Debug_Print("H0/H1      - Heater OFF/ON\r\n");
    Debug_Print("D1 0-100   - Dimmer1 percentage\r\n");
    Debug_Print("D2 0-100   - Dimmer2 percentage\r\n");
    Debug_Print("HUM 0-100  - Humidifier PWM\r\n");
    Debug_Print("LDR        - Read light sensor\r\n");
    Debug_Print("MIC        - Read microphone\r\n");
    Debug_Print("TEMP       - Read temperature\r\n");
    Debug_Print("HUMID      - Read humidity\r\n");
    Debug_Print("ESP32      - Simulate ESP32 connect\r\n");
    Debug_Print("RESET      - Software reset\r\n");
    Debug_Print("===================\r\n");
}

void Debug_Show_Status(void) {
    Debug_Print("\r\n=== SYSTEM STATUS ===\r\n");
    Debug_Printf("Uptime: %lu seconds\r\n", sys_status.system_uptime_ms / 1000);
    Debug_Printf("Heater: %s\r\n", sys_status.heater_on ? "ON" : "OFF");
    Debug_Printf("Dimmer1: %d%%\r\n", sys_status.dimmer1_percent);
    Debug_Printf("Dimmer2: %d%%\r\n", sys_status.dimmer2_percent);
    Debug_Printf("Humidifier: %d%%\r\n", sys_status.humidifier_pwm);
    Debug_Printf("Temperature: %.1f°C\r\n", sys_status.temperature);
    Debug_Printf("Humidity: %.1f%%\r\n", sys_status.humidity);
    Debug_Printf("Light: %d\r\n", sys_status.light_level);
    Debug_Printf("Sound: %d\r\n", sys_status.sound_level);
    Debug_Printf("ESP32: %s\r\n", sys_status.esp32_connected ? "Connected" : "Disconnected");
    Debug_Print("=====================\r\n");
}

void Debug_Show_All_Sensors(void) {
    Debug_Print("\r\n=== ALL SENSORS ===\r\n");

    // DHT22
    float temp = Sensor_DHT22_Get_Temperature();
    float hum = Sensor_DHT22_Get_Humidity();
    if (temp > -100) {
        Debug_Printf("Temperature: %.1f°C\r\n", temp);
    } else {
        Debug_Print("Temperature: Not available\r\n");
    }
    if (hum >= 0) {
        Debug_Printf("Humidity: %.1f%%\r\n", hum);
    } else {
        Debug_Print("Humidity: Not available\r\n");
    }

    // ADC
    Debug_Printf("Light Sensor: %d\r\n", Sensor_ADC_Get_Light());
    Debug_Printf("Sound Sensor: %d\r\n", Sensor_ADC_Get_Sound());

    Debug_Print("====================\r\n");
}

void Debug_Show_All_Actuators(void) {
    Debug_Print("\r\n=== ALL ACTUATORS ===\r\n");

    // Read GPIO states
    GPIO_PinState heater_state = HAL_GPIO_ReadPin(HEATER_PORT, HEATER_PIN);
    GPIO_PinState dimmer1_state = HAL_GPIO_ReadPin(DIMMER_CH1_PORT, DIMMER_CH1_PIN);
    GPIO_PinState dimmer2_state = HAL_GPIO_ReadPin(DIMMER_CH2_PORT, DIMMER_CH2_PIN);
    GPIO_PinState ir_ac_state = HAL_GPIO_ReadPin(IR_AC_PORT, IR_AC_PIN);
    GPIO_PinState ir_sound_state = HAL_GPIO_ReadPin(IR_SOUND_PORT, IR_SOUND_PIN);

    Debug_Printf("Heater (PA7): %s\r\n", heater_state ? "HIGH" : "LOW");
    Debug_Printf("Dimmer1 (PB5): %s\r\n", dimmer1_state ? "HIGH" : "LOW");
    Debug_Printf("Dimmer2 (PA6): %s\r\n", dimmer2_state ? "HIGH" : "LOW");
    Debug_Printf("Humidifier PWM: %d%%\r\n", Actuator_Humidifier_Get());
    Debug_Printf("IR AC (PC7): %s\r\n", ir_ac_state ? "HIGH (OFF)" : "LOW (ON)");
    Debug_Printf("IR Sound (PA8): %s\r\n", ir_sound_state ? "HIGH (OFF)" : "LOW (ON)");

    Debug_Print("=====================\r\n");
}
