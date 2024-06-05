/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32l4xx_hal.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_dma.h"

#include "stm32l4xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GSM_PWR_Pin GPIO_PIN_13
#define GSM_PWR_GPIO_Port GPIOC
#define MB_RE_Pin GPIO_PIN_0
#define MB_RE_GPIO_Port GPIOA
#define MB_DE_Pin GPIO_PIN_1
#define MB_DE_GPIO_Port GPIOA
#define ADC_BATT_Pin GPIO_PIN_4
#define ADC_BATT_GPIO_Port GPIOA
#define ADC_IN_2_uC_Pin GPIO_PIN_5
#define ADC_IN_2_uC_GPIO_Port GPIOA
#define ADC_IN_1_uC_Pin GPIO_PIN_6
#define ADC_IN_1_uC_GPIO_Port GPIOA
#define VIN_ADC_IN_uC_Pin GPIO_PIN_7
#define VIN_ADC_IN_uC_GPIO_Port GPIOA
#define CAN1_MODE_Pin GPIO_PIN_1
#define CAN1_MODE_GPIO_Port GPIOB
#define DO_uC_Pin GPIO_PIN_2
#define DO_uC_GPIO_Port GPIOB
#define WD_FEED_uC_Pin GPIO_PIN_12
#define WD_FEED_uC_GPIO_Port GPIOB
#define RPM_IN1_Pin GPIO_PIN_8
#define RPM_IN1_GPIO_Port GPIOA
#define GPS_LED_uC_Pin GPIO_PIN_15
#define GPS_LED_uC_GPIO_Port GPIOA
#define COMM_LED_uC_Pin GPIO_PIN_10
#define COMM_LED_uC_GPIO_Port GPIOC
#define I_O_uC_1_Pin GPIO_PIN_11
#define I_O_uC_1_GPIO_Port GPIOC
#define I_O_uC_2_Pin GPIO_PIN_12
#define I_O_uC_2_GPIO_Port GPIOC
#define DI_IN_1_uC_Pin GPIO_PIN_4
#define DI_IN_1_uC_GPIO_Port GPIOB
#define DI_IN_2_uC_Pin GPIO_PIN_5
#define DI_IN_2_uC_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
