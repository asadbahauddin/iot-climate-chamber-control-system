#ifndef SENSOR_DHT22_H
#define SENSOR_DHT22_H

#include <stdbool.h>

typedef enum {
    DHT_OK = 0,
    DHT_ERROR_NO_RESPONSE,
    DHT_ERROR_TIMEOUT,
    DHT_ERROR_CHECKSUM,
    DHT_ERROR_NOT_READY
} DHT_Status_t;

void Sensor_DHT22_Init(void);
bool Sensor_DHT22_Read_Now(void);
float Sensor_DHT22_Get_Temperature(void);
float Sensor_DHT22_Get_Humidity(void);
DHT_Status_t Sensor_DHT22_Get_Status(void);

// Compatibility
void Sensor_DHT22_Task(void);
bool Sensor_DHT22_Start_Measurement(void);

#endif /* SENSOR_DHT22_H */
