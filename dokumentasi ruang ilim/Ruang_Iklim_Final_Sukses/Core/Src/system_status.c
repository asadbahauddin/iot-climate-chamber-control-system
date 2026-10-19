#include "system_status.h"
#include <string.h>

SystemStatus_t sys_status;

void SystemStatus_Init(void) {
    memset(&sys_status, 0, sizeof(SystemStatus_t));

    // Default values
    sys_status.target_temp = 25.0f;
    sys_status.target_humidity = 50.0f;
    sys_status.light_mode = 0;
    sys_status.sound_mode = 0;

    sys_status.heater_on = false;
    sys_status.dimmer1_percent = 0;
    sys_status.dimmer2_percent = 0;
    sys_status.humidifier_pwm = 0;
    sys_status.ac_on = false;
    sys_status.ac_temp = 25;
    sys_status.sound_on = false;

    sys_status.esp32_connected = false;
    sys_status.sensor_error = false;
    sys_status.system_uptime_ms = 0;
    sys_status.last_esp32_msg_ms = 0;

    // Invalid sensor readings
    sys_status.temperature = -100.0f;
    sys_status.humidity = -1.0f;
    sys_status.light_level = 0;
    sys_status.sound_level = 0;
}

void SystemStatus_Update_Uptime(void) {
    sys_status.system_uptime_ms += 10; // Called every 10ms
}
