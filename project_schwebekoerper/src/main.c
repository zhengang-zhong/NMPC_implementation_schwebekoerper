#include "main.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Prinft redirect to fputc -> i2c transmit */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#define I2C_ADU_ADDR 0x90
/* I2C ultrasonic sensor */
#define I2C_ULTRASONIC_ADDR 0xE0
#define I2C_COMMAND_INCH 0x50
#define I2C_COMMAND_CM 0x51
#define I2C_COMMAND_MS 0x52
#define I2C_COMMAND_ADDR 0x00
#define I2C_MAG_ADDR 0x01 /* maximal analogue gain address */
#define I2C_READ_ADDR 0x02
#define I2C_RANGE_ADDR 0x02
#define I2C_MAG_VALUE 0x02 /* maximal analogue gain value */
#define I2C_RANGE_VALUE 0x28 /* 0x28 = 40 -> 42*43+43 = 1849mm  */
#define I2C_REGISTER_LENGTH 8U

/* Set I2C SCL frequency 100kHz */
// #define  I2C_TIMING 0xC0421014 //Prescaler | 0 | SCLDEL | SDADEL | SCLH | SCLL |
#define  I2C_TIMING 0xC0421314 //Prescaler | 0 | SCLDEL | SDADEL | SCLH | SCLL |
/* Set Timer4 to capture input PWM between 30 -150 Hz*/
#define  PERIOD_VALUE       (uint32_t)(900 - 1)  /* Period Value  */
#define TXBUFFERSIZE (COUNTOF(aTxBuffer) - 1)
/* NMPC */
#define CM_TO_M  0.01
#define MS_TO_S 0.001 
/* Private variables ---------------------------------------------------------*/
static GPIO_InitTypeDef  GPIO_InitStruct;
UART_HandleTypeDef Uart3Handle;
TIM_HandleTypeDef    Tim1;
TIM_OC_InitTypeDef Tim1_PWMConfig;
TIM_HandleTypeDef    Tim3;
TIM_HandleTypeDef    Tim4;
I2C_HandleTypeDef I2c1Handle;
__IO uint32_t Prescaler_TIM1 = 0;
__IO uint32_t Prescaler_TIM4 = 0;
__IO uint32_t Period_TIM1 = 0;
__IO uint32_t Period_TIM4 = 0;
__IO uint32_t Pulse_TIM1 = 0;
__IO uint32_t Pulse_TIM4 = 0;
__IO uint32_t TIM3_counter = 0;/* maximal counter is 2^32 -> 4,296e9 -> 70 minutes */
uint8_t RxBuffer_uart[1] ;
/* Timer 1 */
float TIM1_PWM_dutycycle = 0;
/* Buffer used for transmission */
uint8_t TxBuffer_i2c_command[2] = {I2C_COMMAND_ADDR,I2C_COMMAND_CM};
uint8_t TxBuffer_i2c_mag[2] = {I2C_MAG_ADDR,I2C_MAG_VALUE}; /* set maximal analogue gain */
uint8_t TxBuffer_i2c_range[2] = {I2C_RANGE_ADDR,I2C_RANGE_VALUE}; /* set maximal analogue gain */
uint8_t TxBuffer_i2c_readaddr[1] = {I2C_READ_ADDR}; /* set read address */
/* Buffer used for reception */
uint8_t RxBuffer_i2c[2];
uint16_t Rx_cm = 0x9a;

uint32_t Capture_old = 0;
uint32_t Capture_new = 0;
float Capture_frequency = 0.0;
uint32_t Capture_value = 0;
/* Tik tok */
extern __IO uint32_t uwTick; 
__IO uint32_t tic=0;
__IO uint32_t toc=0;
__IO uint32_t toc_new=0;
__IO uint32_t toc_old=0;
__IO uint32_t duration=0;
/* NMPC */
float h_pre=0.0;
float h_now=0.0;
float v_now = 0.0;
float eta_now = 0.0;
float x0_input[3]={0.0};
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void TIM1_Configuration(void);
static void LED_Configuration(void);
static void UART3_Configuration(void);
static void Error_Handler(void);
static void TIM3_Configuration(void);
static void TIM4_Configuration(void);
static void I2C1_Configuration(void);
static void I2C1_temp(void);
void I2C1Read(void);
void TIM1_PWM_Configuration(float duty_cycle);
 



int main(void){
	// test for mpc di
	float u_temp[1]={0.0};

	//initial states, destination states and input
	x0_input[0] = 0;
	x0_input[1] = 0;
	x0_input[2] = 50;


	/* 
		CPU_CACHE_Enable() guarentee faster memory access
		HAL_Init() configure the board with following configurations:

		STM32F7xx HAL library initialization:
    	- Configure the Flash prefetch
    	- Systick timer is configured by default as source of time base, but user
    	can eventually implement his proper time base source (a general purpose
    	timer for example or other time source), keeping in mind that Time base
        duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
        handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
	CPU_CACHE_Enable();
  	HAL_Init();
	/* Configure the system clock to 216 MHz */
	/* The first three functions are used in STM32f7 examples */
	SystemClock_Config();

	/* LED GPIO configuration */
	LED_Configuration();
	UART3_Configuration();
	TIM1_Configuration();
	TIM4_Configuration();
	I2C1_Configuration();
	mpc_initialization();

	h_pre = h_now = 0;
	while (1){
		HAL_Delay(20);
		tic = uwTick;
		I2C1_temp();

		h_now = (float)(0x9a - Rx_cm)* CM_TO_M;
		v_now = (float) ((h_now - h_pre) * CM_TO_M )/ (100 * MS_TO_S);
	
		eta_now = Capture_frequency / 60 ;
		x0_input[0] = h_now;
		x0_input[1] = v_now;
		x0_input[2] = eta_now;
		//Debug:
		// printf("Rx_cm is %d\n\r",Rx_cm);
		// if(Rx_cm == 0){
		// 	continue;
		// }
		// printf("current state: x %0.3f, v %0.3f, eta %0.3f\n\r",x0_input[0],x0_input[1],x0_input[2]);
		printf("state: %0.3f, %0.3f, %0.3f\n",x0_input[0],x0_input[1],x0_input[2]);
		u_temp[0] = (float)acado_solver_step(x0_input);
		if(u_temp[0] > 255){
			u_temp[0] = 255;
		}
		// di_sim(x0_input,u_temp,(uint32_t)100000);
		printf("data1: 0x%x, data2 0x%x\n",RxBuffer_i2c[0],RxBuffer_i2c[1]);
		printf("input: %0.3f\n",u_temp[0]/255);
		TIM1_PWM_Configuration(u_temp[0]/255);
		h_pre = h_now;
		toc_new = uwTick;
		duration = toc_new - toc_old;
		// printf("duration: %d\n",duration);
		toc_old = toc_new;
		toc = uwTick;
		// printf("return u is: %lf\n\r",(double)acado_solver_step(x0_input));
		// acado_di(x0_input,x_u_des_input,x_u_output);
		// di_sim(x_u_output,0.5e6);
		// x0_input[0] = x_u_output[0];
		// x0_input[1] = x_u_output[1];
		// duration = toc - tic;
	//printf("mpc simulation execution time is: %ld microsecond\n\r",toc-tic);
		// printf("x0 input is %f\n\r",x0_input[0]);



	//HAL_UART_Transmit(&UartHandle, "a", 1, 0xFFFF);
    // HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    // /* Insert delay 100 ms */
    // HAL_Delay(100);
	}
}

static void TIM3_Configuration(void){


	
	/*configure timer 3 as a general clock source for software
	Timer 3 clock source is 54 * 2Mhz 
	Timer 3 incremets every 1 us
	$\frac{54*2 \times 10^6}{(60-1+1)\times(60000-1+1)} = 30$
	*/

	TIM_OC_InitTypeDef Tim3_PWMConfig;
	__IO uint32_t Period_TIM3=(uint32_t)(108-1);
	__IO uint32_t Pulse_TIM3 = 0;
	Pulse_TIM3 = (uint32_t)(0.5 * (Period_TIM3+1 ));
	// Prescaler_TIM4 = (uint32_t)(30-1);
	// uint32_t_ Prescaler_TIM4 = (uint32_t)(1-1);
	Tim3.Instance = TIM3;
	Tim3.Init.Period            = Period_TIM3;
	Tim3.Init.Prescaler         = 0;
	Tim3.Init.ClockDivision     = 0;
	Tim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
	Tim3.Init.RepetitionCounter = 0;
	Tim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_PWM_Init(&Tim3) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
	Tim3_PWMConfig.OCMode      = TIM_OCMODE_PWM1;
	Tim3_PWMConfig.OCFastMode  = TIM_OCFAST_DISABLE;
	Tim3_PWMConfig.OCPolarity  = TIM_OCPOLARITY_LOW;
	Tim3_PWMConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	Tim3_PWMConfig.OCIdleState = TIM_OCIDLESTATE_SET;
	Tim3_PWMConfig.OCNIdleState= TIM_OCNIDLESTATE_RESET;
	Tim3_PWMConfig.Pulse = Pulse_TIM3;
	printf("pulse 3 %d\n\r", Pulse_TIM3);
	/* Set the pulse value for channel 1 */   
	/* PC6 */
	if(HAL_TIM_PWM_ConfigChannel(&Tim3, &Tim3_PWMConfig, TIM_CHANNEL_1) != HAL_OK)
	{
	/* Configuration Error */
		Error_Handler();
	}

	/*Step 3: Start PWM signals generation*/ 
	/* Start channel 1*/
	if(HAL_TIM_PWM_Start(&Tim3, TIM_CHANNEL_1) != HAL_OK)
	{
	/* Starting Error */
		Error_Handler();
	}   
}

static void TIM4_Configuration(void){
	/* Timer handler declaration */
	/* Timer Input Capture Configuration Structure declaration */
	TIM_IC_InitTypeDef     sICConfig;

	/* Step 1: configure PWM capture belonging timer (Tachosignal)  */
	/* TIM4 configuration: Input Capture mode ---------------------
		The external signal is connected to TIM4 CH1 pin (PB6)  
		The Rising edge is used as active edge,
		The TIM4 CCR1 is used to compute the frequency value 
	------------------------------------------------------------ */

	/* Set TIM4 instance */
	Tim4.Instance = TIM4;


	/*
	Timer 4 clock source is 54 * 2Mhz 
	Frequncy to be captured: 30-150Hz
	$\frac{54*2 \times 10^6}{(60-1+1)\times(60000-1+1)} = 30$
	*/
	// Period_TIM4 = (uint32_t)((SystemCoreClock/4) / 60000) - 1;
	Period_TIM4 =(uint32_t)(60000-1);
	// Prescaler_TIM4 = (uint32_t)(30-1);
	Prescaler_TIM4 = (uint32_t)(60-1);

	Tim4.Init.Period            = Period_TIM4;
	Tim4.Init.Prescaler         = Prescaler_TIM4;
	Tim4.Init.ClockDivision     = 0;
	Tim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
	Tim4.Init.RepetitionCounter = 0;
	Tim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_IC_Init(&Tim4) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*Step 2: Configure the Input Capture channel*/ 
	/* Configure the Input Capture of channel 2 */
	sICConfig.ICPolarity  = TIM_ICPOLARITY_RISING;
	sICConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sICConfig.ICPrescaler = TIM_ICPSC_DIV1;
	sICConfig.ICFilter    = 0;   
	if(HAL_TIM_IC_ConfigChannel(&Tim4, &sICConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Configuration Error */
		Error_Handler();
	}

	/*Step3: Start the Input Capture in interrupt mode*/
	/*when input signal at PB6 is captured, hardware interrupt signal calls interrupt handler  */
	if(HAL_TIM_IC_Start_IT(&Tim4, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}
	printf("Timer 4 initialized\n\r");
}

static void TIM1_Configuration(void){
	TIM_OC_InitTypeDef Tim1_PWMConfig;
	/**TIM1 GPIO Configuration
	PE13     ------> TIM1_CH3
	*/
	/*Step 1: Configure and initialize Timer  */
	/* Timer 1 clock source is 216Mhz */
	/*set PWM output frequency 30kHz*/
	Period_TIM1 =(uint32_t)(20000-1);
	Prescaler_TIM1 = (uint32_t)(360-1);
	// Period_TIM1 =(uint32_t)(240-1);
	// Prescaler_TIM1 = (uint32_t)(6000-1);
	TIM1_PWM_dutycycle = 0.5;
	Pulse_TIM1 = (uint32_t)(TIM1_PWM_dutycycle * (Period_TIM1+1 ));
	Tim1.Instance = TIM1;
	Tim1.Init.Prescaler = Prescaler_TIM1;
	Tim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	Tim1.Init.Period =  Period_TIM1;
	Tim1.Init.ClockDivision = 0;
	Tim1.Init.RepetitionCounter = 0;
	Tim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	// printf("state is reset 0x%x",Tim1.State );

	if(HAL_TIM_PWM_Init(&Tim1) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
	/*Step2:  Configure the PWM channels */
	/* Common configuration for all channels */
	Tim1_PWMConfig.OCMode      = TIM_OCMODE_PWM1;
	Tim1_PWMConfig.OCFastMode  = TIM_OCFAST_DISABLE;
	Tim1_PWMConfig.OCPolarity  = TIM_OCPOLARITY_LOW;
	Tim1_PWMConfig.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	Tim1_PWMConfig.OCIdleState = TIM_OCIDLESTATE_SET;
	Tim1_PWMConfig.OCNIdleState= TIM_OCNIDLESTATE_RESET;

	Tim1_PWMConfig.Pulse = Pulse_TIM1;
	/* Set the pulse value for channel 3 */   
	if(HAL_TIM_PWM_ConfigChannel(&Tim1, &Tim1_PWMConfig, TIM_CHANNEL_3) != HAL_OK)
	{
	/* Configuration Error */
		Error_Handler();
	}


	/*Step 3: Start PWM signals generation*/ 
	/* Start channel 3 */
	if(HAL_TIM_PWM_Start(&Tim1, TIM_CHANNEL_3) != HAL_OK)
	{
	/* Starting Error */
		Error_Handler();
	}   

	printf("PWM1 initialized \n\r");


}

void TIM1_PWM_Configuration(float duty_cycle){
	TIM1_PWM_dutycycle = duty_cycle;
	uint32_t Pulse_temp =(uint32_t) ( (1- duty_cycle )* (Period_TIM1+1 )) ;

	if ( duty_cycle <=1.0 && duty_cycle >= 0.0 ){
		__HAL_TIM_SET_COMPARE(&Tim1,TIM_CHANNEL_3,Pulse_temp);
	}
	else{
		printf("Tim1 PWM setting wrong input.../n/r");
		Error_Handler();
	}

}

static void UART3_Configuration(void){
	/*Step 1- Configure the UART peripheral 3*/
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART configured as follows:
		- Word Length = 8 Bits (7 data bit + 1 parity bit) : 
						BE CAREFUL : Program 7 data bits + 1 parity bit in PC HyperTerminal

		- Stop Bit    = One Stop bit
		- Parity      = None
		- BaudRate    = 115200 baud
		- Hardware flow control disabled (RTS and CTS signals) */

	Uart3Handle.Instance        = USART3;
	Uart3Handle.Init.BaudRate   = 115200;
	Uart3Handle.Init.WordLength = UART_WORDLENGTH_8B;
	Uart3Handle.Init.StopBits   = UART_STOPBITS_1;
	// UartHandle.Init.Parity     = UART_PARITY_ODD;
	Uart3Handle.Init.Parity     = UART_PARITY_NONE;
	Uart3Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	Uart3Handle.Init.Mode       = UART_MODE_TX_RX;
	Uart3Handle.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&Uart3Handle) != HAL_OK)
	{
	/* Initialization Error */
		Error_Handler();
	}

	/* Output a message on Hyperterminal using printf function */
	printf("\n\r UART Printf \n\r");
	printf("** Test finished successfully. ** \n\r");

	/*Step3 Put UART peripheral in reception process */  
	/* Any data received will be stored "RxBuffer_uart" buffer : One char will be detected for instruction execution */
	if(HAL_UART_Receive_IT(&Uart3Handle, (uint8_t *) RxBuffer_uart, 1) != HAL_OK)
	{
	/* Transfer error in reception process */
		Error_Handler();
	}
}

static void I2C1_temp(void){
	
	if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_mag, 2, I2C_FIRST_AND_LAST_FRAME)!= HAL_OK)
	// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_mag, 2, I2C_FIRST_FRAME)!= HAL_OK)
	{
		/* Error_Handler() function is called when error occurs. */
		printf("2\n\r");
		Error_Handler();
	}
	HAL_Delay(10);
	if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_range, 2, I2C_FIRST_AND_LAST_FRAME)!= HAL_OK)
	// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_range, 2, I2C_NEXT_FRAME)!= HAL_OK)
	{
		printf("3\n\r");		
		/* Error_Handler() function is called when error occurs. */
		Error_Handler();
	}
	HAL_Delay(10);
	if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_command, 2, I2C_FIRST_AND_LAST_FRAME)!= HAL_OK)
	// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_command, 2, I2C_NEXT_FRAME)!= HAL_OK)
	{
		printf("4\n\r");		
		/* Error_Handler() function is called when error occurs. */
		Error_Handler();
	}	
	HAL_Delay(40); //shorter than 40 ms, transferd the ultra sonic sensor 0x00 

	if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_readaddr, 1, I2C_FIRST_FRAME)!= HAL_OK){
		// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_readaddr, 1, I2C_FIRST_AND_LAST_FRAME)!= HAL_OK){
			/* Error_Handler() function is called when error occurs. */
		if(HAL_I2C_GetError(&I2c1Handle) == HAL_I2C_ERROR_AF){
			Error_Handler();
		}
	}
		/* Step 1: Send the target data register address to slave device */	
		// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_readaddr, 1, I2C_FIRST_FRAME)!= HAL_OK){
		// 	/* Error_Handler() function is called when error occurs. */
		// 	Error_Handler();
		// }
		/* Step 2: Wait for the end of the tranfer*/
		/*  Before starting a new communication transfer, you need to check the current
			state of the peripheral; if it’s busy you need to wait for the end of current
			transfer before starting a new one.
			For simplicity reasons, this example is just waiting till the end of the
			transfer, but application may perform other tasks while transfer operation
			is ongoing. */
		/*HAL_I2C_STATE_READY = 0x20U, HAL_I2C_STATE_BUSY  = 0x24U*/
	while (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_READY){
		// printf("0x%x",HAL_I2C_GetState(&I2c1Handle));
	}	
		// HAL_Delay(10);
		/*Step 3: Put I2C peripheral in reception*/
		/*function in i2c c-file has different name from user manual UM1905*/
		// if(HAL_I2C_Master_Seq_Receive_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t *)RxBuffer_i2c,2 , I2C_FIRST_AND_LAST_FRAME) != HAL_OK){
	if(HAL_I2C_Master_Seq_Receive_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t *)RxBuffer_i2c,2 , I2C_LAST_FRAME) != HAL_OK){
		/* Error_Handler() function is called when error occurs. */
		Error_Handler();
	}

		/*Step 4: Wait for the end of the transfer */
	while (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_READY){
		if(HAL_I2C_GetError(&I2c1Handle) == HAL_I2C_ERROR_AF){
			Error_Handler();
		}
	}

	while (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_READY){
		printf("busy\n\r");
	}	
    /* When Acknowledge failure occurs, master restart the transfer again*/

}

static void I2C1_Configuration(void){
	/*Step 1: Configure the I2C peripheral */
	I2c1Handle.Instance             = I2C1;
	I2c1Handle.Init.Timing          = I2C_TIMING;
	I2c1Handle.Init.OwnAddress1     = 0x01;
	I2c1Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	I2c1Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2c1Handle.Init.OwnAddress2     = 0xFE; //second address for dual addressing mode
	I2c1Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2c1Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	if(HAL_I2C_Init(&I2c1Handle) != HAL_OK)
	{
		/* Initialization Error */
		printf("1\n\r");
		Error_Handler();
	}



	printf("I2C1 configured\n\r");
}


void I2C1Read(void){

	/* Start the transmission process */

	if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_readaddr, 1, I2C_FIRST_FRAME)!= HAL_OK){
		// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_readaddr, 1, I2C_FIRST_AND_LAST_FRAME)!= HAL_OK){
			/* Error_Handler() function is called when error occurs. */
		if(HAL_I2C_GetError(&I2c1Handle) == HAL_I2C_ERROR_AF){
			Error_Handler();
		}
	}
		/* Step 1: Send the target data register address to slave device */	
		// if(HAL_I2C_Master_Sequential_Transmit_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t*) TxBuffer_i2c_readaddr, 1, I2C_FIRST_FRAME)!= HAL_OK){
		// 	/* Error_Handler() function is called when error occurs. */
		// 	Error_Handler();
		// }
		/* Step 2: Wait for the end of the tranfer*/
		/*  Before starting a new communication transfer, you need to check the current
			state of the peripheral; if it’s busy you need to wait for the end of current
			transfer before starting a new one.
			For simplicity reasons, this example is just waiting till the end of the
			transfer, but application may perform other tasks while transfer operation
			is ongoing. */
		/*HAL_I2C_STATE_READY = 0x20U, HAL_I2C_STATE_BUSY  = 0x24U*/
	while (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_READY){
		// printf("0x%x",HAL_I2C_GetState(&I2c1Handle));
	}	
		// HAL_Delay(10);
		/*Step 3: Put I2C peripheral in reception*/
		/*function in i2c c-file has different name from user manual UM1905*/
		// if(HAL_I2C_Master_Seq_Receive_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t *)RxBuffer_i2c,2 , I2C_FIRST_AND_LAST_FRAME) != HAL_OK){
	if(HAL_I2C_Master_Seq_Receive_IT(&I2c1Handle, (uint16_t)I2C_ULTRASONIC_ADDR, (uint8_t *)RxBuffer_i2c,2 , I2C_LAST_FRAME) != HAL_OK){
		/* Error_Handler() function is called when error occurs. */
		Error_Handler();
	}

		/*Step 4: Wait for the end of the transfer */
	while (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_READY){
		if(HAL_I2C_GetError(&I2c1Handle) == HAL_I2C_ERROR_AF){
			Error_Handler();
		}
	}
	printf("data1: 0x%x, data2 0x%x\n\r",RxBuffer_i2c[0],RxBuffer_i2c[1]);
	while (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_READY){
		printf("busy\n\r");
	}	
    /* When Acknowledge failure occurs, master restart the transfer again*/
	Rx_cm =(RxBuffer_i2c[0] << I2C_REGISTER_LENGTH) | RxBuffer_i2c[1]; 
}

static void LED_Configuration(void){
	/*Enable GPIO Clock (to be able to program the configuration registers) */
	/*LED1_PIN PB0*/ 
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/* -2- Configure IO in output push-pull mode to drive external LEDs */
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


}



/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 216000000
 *            HCLK(Hz)                       = 216000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 432
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            PLL_R                          = 7
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
	{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 7;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		while(1) {};
	}

	/* Activate the OverDrive to reach the 216 Mhz Frequency */
	if(HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		while(1) {};
	}
	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
		clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
	{
		while(1) {};
	}
}

static void Error_Handler(void)
	{
	/* Turn LED3 on */
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	printf("something goes wrong.../n/r");
	while (1)
	{

	}
}

PUTCHAR_PROTOTYPE{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART3 and Loop until the end of transmission */
	HAL_UART_Transmit(&Uart3Handle, (uint8_t *)& ch, 1, 0xFFFF);
	return ch;
}


/* This callback is called by the HAL_UART_IRQHandler when the given number of bytes are received */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	float u_temp[1]={0.0};
	if (huart->Instance == USART3)
	{
	/* Transmit one byte with 100 ms timeout */

	printf("char %c received\n\r", RxBuffer_uart[0]);
	switch (RxBuffer_uart[0])
	{
		case 'r':
			printf("the captured  frequency: %0.2f 1/min\n\r",Capture_frequency);
			printf("clock HAL_RCC_GetPCLK1Freq():%d \n\r",HAL_RCC_GetPCLK1Freq());
			printf("the captured value: %d \n\r",Capture_value);
			break;
		case 'a':
			printf("flaot print test %f\n\r",1.1);
			break;
		case 'd':
			printf("duration %d\n\r",duration);
			break;
		case '-':
			TIM1_PWM_dutycycle -= 0.01;
			if (TIM1_PWM_dutycycle <=0 ){
				TIM1_PWM_Configuration(0.0);
			}	
			else{
				TIM1_PWM_Configuration(TIM1_PWM_dutycycle);
			}
			printf("duty cycle is %0.2f\n\r",TIM1_PWM_dutycycle);
			printf("compare value is: %d\n\r",__HAL_TIM_GET_COMPARE(&Tim1,TIM_CHANNEL_3));
			break;
		case '+':
			TIM1_PWM_dutycycle += 0.01;
			if (TIM1_PWM_dutycycle >=1.0 ){
				TIM1_PWM_Configuration(1.0);
			}	
			else{
				TIM1_PWM_Configuration(TIM1_PWM_dutycycle);
			}
			printf("duty cycle is %0.2f\n\r",TIM1_PWM_dutycycle);
			printf("compare value is: %d\n\r",__HAL_TIM_GET_COMPARE(&Tim1,TIM_CHANNEL_3));
			break;
		case 's':
 			u_temp[0] = acado_solver_step(x0_input);
			di_sim(x0_input,u_temp,(uint32_t)100000);
			printf("optimal input is: %0.3f\n\r",u_temp[0]);
			printf("state 0 is: %0.3f\n\r",x0_input[0]);
			printf("state 1 is: %0.3f\n\r",x0_input[1]);
			printf("state 2 is: %0.3f\n\r",x0_input[2]);
			break;
		case 'i':
			I2C1Read();
		default:
			break;
	}



	// /* Receive one byte in interrupt mode */ 
	HAL_UART_Receive_IT(&Uart3Handle, (uint8_t *) RxBuffer_uart, 1);
	}
}



/*this function will be called at the end of transfer, if I2C_AUTOEND_MODE is enable*/
/*namely the LAST_FRAME flag is set.*/
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	/* Turn LED2 on: Transfer in reception process is correct */
	//BSP_LED_On(LED2);
	if(RxBuffer_i2c[1] >= 0x9a){
		RxBuffer_i2c[1] = 0x9a;
	}
	Rx_cm =(RxBuffer_i2c[0] << I2C_REGISTER_LENGTH) | RxBuffer_i2c[1]; 
	// printf("received\n\r");
}

/*call HAL_I2C_MasterTxCpltCallback as well, if needed*/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	/* Turn LED1 on: Transfer in transmission process is correct */
	// printf("transmitted\n\r");
}

static void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}