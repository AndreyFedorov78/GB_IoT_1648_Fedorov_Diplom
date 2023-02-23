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
#include "stm32g0xx_hal.h"

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
#define W5500_INT_Pin GPIO_PIN_1
#define W5500_INT_GPIO_Port GPIOA
#define W5500_CS_Pin GPIO_PIN_2
#define W5500_CS_GPIO_Port GPIOA
#define RS485_Pin GPIO_PIN_12
#define RS485_GPIO_Port GPIOB
#define KEY_Pin GPIO_PIN_13
#define KEY_GPIO_Port GPIOB
#define W5500_RST_Pin GPIO_PIN_7
#define W5500_RST_GPIO_Port GPIOC
#define RADIO_PIN_Pin GPIO_PIN_15
#define RADIO_PIN_GPIO_Port GPIOA
#define RADIO_PIN_EXTI_IRQn EXTI4_15_IRQn
#define DIG_CS_Pin GPIO_PIN_3
#define DIG_CS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
