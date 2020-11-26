
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_it.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef Uart3Handle;
extern	TIM_HandleTypeDef    Tim1;
extern	TIM_HandleTypeDef    Tim4;
extern I2C_HandleTypeDef I2c1Handle;
extern uint32_t Capture_old ;
extern uint32_t Capture_new ;
extern float Capture_frequency ;
extern uint32_t Capture_value ;
extern uint32_t Period_TIM4 ;
extern uint32_t Prescaler_TIM4 ;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}



/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}


void TIM4_IRQHandler(void)
{
	// HAL_TIM_IRQHandler(&Tim4);
	//printf("something captured\n\r");
	if (__HAL_TIM_GET_FLAG(&Tim4, TIM_FLAG_CC1) == SET){
		if (__HAL_TIM_GET_IT_SOURCE(&Tim4, TIM_IT_CC1) == SET){
			/* captured signal from channel 1 */
			__HAL_TIM_CLEAR_IT(&Tim4, TIM_IT_CC1);
			/* Channal1 captured signal*/
			// Tim4.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
			
			/*if capture mode*/
			if ((Tim4.Instance->CCMR1 & TIM_CCMR1_CC1S) != 0x00U){
				/*process after signal captured*/
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);

				/* Get the 1st Input Capture value */
				Capture_new = HAL_TIM_ReadCapturedValue(&Tim4, TIM_CHANNEL_1);

				// printf("%d",Capture_new);
				/* Capture computation */
				if (Capture_new >= Capture_old){
					Capture_value = (Capture_new - Capture_old); 
				}
				else if (Capture_new < Capture_old){
				/* value from new period*/
					Capture_value = ((Period_TIM4 - Capture_old) + Capture_new) + 1;
				}
				else{
				/* If capture values are equal, we have reached the limit of frequency
				measures */
					//Error_Handler();
				}
				/* Frequency computation: for this example TIMx (TIM1) is clocked by
				2xAPB2Clk */      
				Capture_frequency = (float)(2*HAL_RCC_GetPCLK1Freq()) / Capture_value/ (Prescaler_TIM4+1) * 60;
				Capture_old = Capture_new;
			}
		}

	}
}




void USART3_IRQHandler(void)
{
  	HAL_UART_IRQHandler(&Uart3Handle);
	// customUart_UART_Receive_IT()
}

// HAL_StatusTypeDef customUart_UART_Receive_IT(UART_HandleTypeDef *huart){

	// if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == SET){
         // *huart->pRxBuffPtr = (uint8_t)(huart->Instance->RDR & 0x001fu);
		 // printf("mask: 0x%x",huart->Mask)
	// }
	
	// return HAL_OK;
	
	
// }


void I2C1_EV_IRQHandler(void)
{
  	HAL_I2C_EV_IRQHandler(&I2c1Handle);
}

/**
  * @brief  This function handles I2C error interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C error
  */
void I2C1_ER_IRQHandler(void)
{
  	HAL_I2C_ER_IRQHandler(&I2c1Handle);
}
/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
