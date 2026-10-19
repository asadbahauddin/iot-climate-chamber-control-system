#ifndef ACTUATOR_DIMMER_H
#define ACTUATOR_DIMMER_H

#include <stdint.h>

void Actuator_Dimmer_Init(void);
void Actuator_Dimmer_Set(uint8_t channel, uint8_t percent);
uint8_t Actuator_Dimmer_Get(uint8_t channel);
void Actuator_Dimmer_Task(void);
void Actuator_Dimmer_ZeroCross_Callback(void);

#endif /* ACTUATOR_DIMMER_H */
