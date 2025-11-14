/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define LED_BUILTIN_Pin GPIO_PIN_13
#define LED_BUILTIN_GPIO_Port GPIOC
#define NRF24_CSN_Pin GPIO_PIN_4
#define NRF24_CSN_GPIO_Port GPIOA
#define NRF24_SCK_Pin GPIO_PIN_5
#define NRF24_SCK_GPIO_Port GPIOA
#define NRF24_MISO_Pin GPIO_PIN_6
#define NRF24_MISO_GPIO_Port GPIOA
#define NRF24_MOSI_Pin GPIO_PIN_7
#define NRF24_MOSI_GPIO_Port GPIOA
#define NRF24_CE_Pin GPIO_PIN_0
#define NRF24_CE_GPIO_Port GPIOB
#define NRF24_IRQ_Pin GPIO_PIN_1
#define NRF24_IRQ_GPIO_Port GPIOB
#define NRF24_IRQ_EXTI_IRQn EXTI1_IRQn
#define KEY_COL_0_Pin GPIO_PIN_12
#define KEY_COL_0_GPIO_Port GPIOB
#define KEY_COL_1_Pin GPIO_PIN_13
#define KEY_COL_1_GPIO_Port GPIOB
#define KEY_COL_2_Pin GPIO_PIN_14
#define KEY_COL_2_GPIO_Port GPIOB
#define KEY_COL_3_Pin GPIO_PIN_15
#define KEY_COL_3_GPIO_Port GPIOB
#define KEY_ROW_0_Pin GPIO_PIN_8
#define KEY_ROW_0_GPIO_Port GPIOA
#define KEY_ROW_1_Pin GPIO_PIN_9
#define KEY_ROW_1_GPIO_Port GPIOA
#define KEY_ROW_2_Pin GPIO_PIN_10
#define KEY_ROW_2_GPIO_Port GPIOA
#define KEY_ROW_3_Pin GPIO_PIN_15
#define KEY_ROW_3_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
