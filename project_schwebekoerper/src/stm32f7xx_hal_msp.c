/**
  ******************************************************************************
  * @file    TIM/TIM_TimeBase/Src/stm32f7xx_hal_msp.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */


/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param htim: TIM handle pointer
  * @retval None
  */


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	if (htim->Instance == TIM1 ){
		printf("configure timer1 ...\n\r");

		/*##-1- Enable peripherals and GPIO Clocks #################################*/
		/* TIMx Peripheral clock enable */
		// printf("debug timer pwm\n\r");
		__HAL_RCC_TIM1_CLK_ENABLE();

		/* Enable GPIO Channels Clock */
		__HAL_RCC_GPIOE_CLK_ENABLE();

		/* Configure (PE13, TIM1_CH3) in push-pull, alternate function mode  */

		/* Common configuration for all channels */
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

		/* GPIO TIM1_Channel3 configuration */
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);


		// /* Set the TIMx priority */
		// HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);

		// /* Enable the TIMx global Interrupt */
		// HAL_NVIC_EnableIRQ(TIM3_IRQHandler);
	}
	else if(htim->Instance == TIM3){
		printf("configure timer3 ...\n\r");
		/* PC6  */
		/*##-1- Enable peripherals and GPIO Clocks #################################*/
		/* TIMx Peripheral clock enable */
		// printf("debug timer pwm\n\r");
		__HAL_RCC_TIM3_CLK_ENABLE();

		/* Enable GPIO Channels Clock */
		__HAL_RCC_GPIOC_CLK_ENABLE();

		/* Configure (PC6, TIM3_CH1) in push-pull, alternate function mode  */

		/* Common configuration for all channels */
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLDOWN;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

		/* GPIO TIM3_Channel1 configuration */
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


		// /* Set the TIMx priority */
		// /*##-2- Configure the NVIC for TIMx ########################################*/
		// HAL_NVIC_SetPriority(TIM1_CC_IRQn, 3, 0);

		// /* Enable the TIMx global Interrupt */
		// HAL_NVIC_EnableIRQ(TIM1_CC_IRQHandler);
	
	}
	else{
		printf("pwm configuration goes wrong..\n\r");
	}
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/* Select SysClk as source of USART3 clocks */
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
	RCC_PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
	// RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	// RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

	/* Enable USARTx clock */
	__HAL_RCC_USART3_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* UART TX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = GPIO_PIN_8;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* UART RX GPIO pin configuration  */
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;

	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*##-3- Configure the NVIC for UART ########################################*/   
	/* NVIC for USARTx */
	HAL_NVIC_SetPriority(USART3_IRQn, 2, 1);
	HAL_NVIC_EnableIRQ(USART3_IRQn);

	// __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE); 
}


void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	//printf("debug capture\n\r");
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* TIMx Peripheral clock enable */
	__HAL_RCC_TIM4_CLK_ENABLE();

	/* Enable GPIO channels Clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* Configure  (TIMx_Channel) in Alternate function, push-pull and high speed */
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*##-2- Configure the NVIC for TIMx #########################################*/

	HAL_NVIC_SetPriority(TIM4_IRQn, 1, 1);
	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}



void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;
	/*##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #*/
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

	/*##-2- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	/* Enable I2Cx clock */
	__HAL_RCC_I2C1_CLK_ENABLE(); 

	/*##-3- Configure peripheral GPIO ##########################################*/  
	/* I2C TX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = GPIO_PIN_8;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* I2C RX GPIO pin configuration  */
	GPIO_InitStruct.Pin       = GPIO_PIN_9;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*##-4- Configure the NVIC for I2C ########################################*/   
	/* NVIC for I2Cx */
	HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
	HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
