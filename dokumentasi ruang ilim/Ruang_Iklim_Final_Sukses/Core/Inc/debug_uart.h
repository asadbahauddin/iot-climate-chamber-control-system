#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include <stdint.h>
#include <stdarg.h>

void Debug_UART_Init(void);
void Debug_Print(const char *message);
void Debug_Printf(const char *format, ...);
void Debug_Process_Commands(void);
void Debug_Show_Help(void);
void Debug_Show_Status(void);
void Debug_Show_All_Sensors(void);
void Debug_Show_All_Actuators(void);

#endif /* DEBUG_UART_H */
