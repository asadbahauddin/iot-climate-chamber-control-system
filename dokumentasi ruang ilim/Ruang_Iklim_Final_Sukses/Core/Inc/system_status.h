#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // Sensor Readings
    float temperature;
    float humidity;
    uint16_t light_level;
    uint16_t sound_level;

    // Setpoints
    float target_temp;
    float target_humidity;
    uint8_t light_mode;
    uint8_t sound_mode;

    // Actuator States
    bool heater_on;
    uint8_t dimmer1_percent;
    uint8_t dimmer2_percent;
    uint8_t humidifier_pwm;
    bool ac_on;
    uint8_t ac_temp;
    bool sound_on;

    // System Flags
    bool esp32_connected;
    bool sensor_error;
    uint32_t system_uptime_ms;
    uint32_t last_esp32_msg_ms;

} SystemStatus_t;

extern SystemStatus_t sys_status;

void SystemStatus_Init(void);
void SystemStatus_Update_Uptime(void);
void SystemStatus_Print_Summary(void);
#endif /* SYSTEM_STATUS_H */
