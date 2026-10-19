#ifndef IR_TRANSMITTER_H
#define IR_TRANSMITTER_H

#include "main.h"  // Tetap butuh untuk GPIO_TypeDef
#include "ir_raw.h"

// IR Transmission Configuration
#define IR_CARRIER_FREQ      38000  // 38 kHz
#define IR_DUTY_CYCLE        33     // 33% duty cycle
#define IR_TIMEOUT_MS        200    // Max transmission time

// Function Prototypes
void IR_Transmitter_Init(void);
void IR_Send_Raw(const int32_t *timings, uint16_t length, GPIO_TypeDef* port, uint16_t pin);
void IR_Send_AC_Command(uint8_t temperature, uint8_t power_on);
void IR_Send_Sound_Command(uint8_t command);
void IR_Test_All_Commands(void);

#endif /* IR_TRANSMITTER_H */
