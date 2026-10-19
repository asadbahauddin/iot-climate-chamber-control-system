#ifndef SENSOR_ADC_H
#define SENSOR_ADC_H

#include <stdint.h>

void Sensor_ADC_Init(void);
void Sensor_ADC_Start(void);
void Sensor_ADC_Task(void);
uint16_t Sensor_ADC_Get_Light(void);
uint16_t Sensor_ADC_Get_Sound(void);

#endif /* SENSOR_ADC_H */
