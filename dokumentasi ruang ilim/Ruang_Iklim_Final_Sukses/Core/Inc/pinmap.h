#ifndef PINMAP_H
#define PINMAP_H

#include "stm32f4xx_hal.h"

/* =========== DIMMER AC =========== */
#define DIMMER_ZERO_CROSS_PORT   GPIOA
#define DIMMER_ZERO_CROSS_PIN    GPIO_PIN_0
#define DIMMER_ZERO_CROSS_IRQn   EXTI0_IRQn

#define DIMMER_CH1_PORT          GPIOB
#define DIMMER_CH1_PIN           GPIO_PIN_5
#define DIMMER_CH1_ACTIVE_LOW

#define DIMMER_CH2_PORT          GPIOA
#define DIMMER_CH2_PIN           GPIO_PIN_6
#define DIMMER_CH2_ACTIVE_LOW

/* =========== HEATER =========== */
#define HEATER_PORT              GPIOA
#define HEATER_PIN               GPIO_PIN_7
#define HEATER_MIN_ON_TIME_MS    500   // Reduced for testing
#define HEATER_MIN_OFF_TIME_MS   500   // Reduced for testing

/* =========== HUMIDIFIER =========== */
#define HUMIDIFIER_PORT          GPIOB
#define HUMIDIFIER_PIN           GPIO_PIN_8
#define HUMIDIFIER_TIM           TIM4
#define HUMIDIFIER_TIM_CH        TIM_CHANNEL_3

/* =========== INFRARED =========== */
#define IR_AC_PORT               GPIOC
#define IR_AC_PIN                GPIO_PIN_7
#define IR_AC_ACTIVE_STATE       GPIO_PIN_RESET  // LOW = IR ON

#define IR_SOUND_PORT            GPIOA
#define IR_SOUND_PIN             GPIO_PIN_8
#define IR_SOUND_ACTIVE_STATE    GPIO_PIN_RESET

/* =========== SENSORS =========== */
#define DHT22_PORT               GPIOA
#define DHT22_PIN                GPIO_PIN_10
#define DHT22_SAMPLE_INTERVAL_MS 2000

#define ADC1_INSTANCE            ADC1
#define ADC1_DMA_STREAM          DMA2_Stream0
#define ADC1_DMA_CHANNEL         DMA_CHANNEL_0
#define ADC1_BUFFER_SIZE         2
#define ADC_LDR_CHANNEL          ADC_CHANNEL_4
#define ADC_MIC_CHANNEL          ADC_CHANNEL_5

/* =========== COMMUNICATION =========== */
#define DEBUG_UART_INSTANCE      USART2
#define ESP32_UART_INSTANCE      USART3

/* =========== WATCHDOG =========== */
#define WATCHDOG_INSTANCE        IWDG

#endif /* PINMAP_H */
