#ifndef ACTUATOR_HUMIDIFIER_H
#define ACTUATOR_HUMIDIFIER_H

#include <stdint.h>

void Actuator_Humidifier_Init(void);
void Actuator_Humidifier_Set(uint8_t pwm_percent);
uint8_t Actuator_Humidifier_Get(void);
void Actuator_Humidifier_Task(void);

#endif /* ACTUATOR_HUMIDIFIER_H */
