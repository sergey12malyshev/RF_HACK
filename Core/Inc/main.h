/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_iwdg.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
bool CC1101_reinit(void);

void checkResetSourse(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RED_LED_Pin LL_GPIO_PIN_13
#define RED_LED_GPIO_Port GPIOC
#define LCD_DC_Pin LL_GPIO_PIN_2
#define LCD_DC_GPIO_Port GPIOA
#define LCD_RESET_Pin LL_GPIO_PIN_3
#define LCD_RESET_GPIO_Port GPIOA
#define LCD_CS_Pin LL_GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define LCD_SCK_Pin LL_GPIO_PIN_5
#define LCD_SCK_GPIO_Port GPIOA
#define LCD_LED_Pin LL_GPIO_PIN_6
#define LCD_LED_GPIO_Port GPIOA
#define LCD_SDI_Pin LL_GPIO_PIN_7
#define LCD_SDI_GPIO_Port GPIOA
#define T_IRQ_Pin LL_GPIO_PIN_0
#define T_IRQ_GPIO_Port GPIOB
#define T_IRQ_EXTI_IRQn EXTI0_IRQn
#define T_CS_Pin LL_GPIO_PIN_1
#define T_CS_GPIO_Port GPIOB
#define CC_GDO_Pin LL_GPIO_PIN_12
#define CC_GDO_GPIO_Port GPIOB
#define CC_GDO_EXTI_IRQn EXTI15_10_IRQn
#define NSS_CS_Pin LL_GPIO_PIN_13
#define NSS_CS_GPIO_Port GPIOB
#define BUZZER_Pin LL_GPIO_PIN_15
#define BUZZER_GPIO_Port GPIOA
#define T_OUT_Pin LL_GPIO_PIN_4
#define T_OUT_GPIO_Port GPIOB
#define ENCODER_SW_Pin LL_GPIO_PIN_8
#define ENCODER_SW_GPIO_Port GPIOB
#define ENCODER_SW_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

#define SOFTWARE_VERSION_MAJOR  0
#define SOFTWARE_VERSION_MINOR  2
#define SOFTWARE_VERSION_PATCH  0


#define quoting(a) prequoting(a)
#define prequoting(a) #a

#define __UNUSED __attribute__((unused))

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
