#ifndef ACTUATOR_HEATER_H
#define ACTUATOR_HEATER_H

#include <stdbool.h>

void Actuator_Heater_Init(void);
int Actuator_Heater_Set(bool state);
bool Actuator_Heater_Get(void);
void Actuator_Heater_Task(void);

#endif /* ACTUATOR_HEATER_H */
