/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "HD44780.h"

#include "radio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint8_t isLaserDetected = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void startUpLCDSplashScreen(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// FUNCTION      : TIM15_IRQHandler()
// DESCRIPTION   : It is an ISR which is executed whenever Interrupt is generated from Timer 15
// PARAMETERS    : NULL
// RETURNS       : NULL
void TIM15_IRQHandler(void) {
	HAL_TIM_IRQHandler(&htim15);
}

// FUNCTION      : HAL_TIM_PeriodElapsedCallback()
// DESCRIPTION   : Period elapsed callback in non blocking mode.
// 		 : ISR For all Timers but here we are using it for Timer 15.
//
// PARAMETERS    : htim TIM handle
// RETURNS       : NULL
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM15) //check if the Interrupt ha occured for TIMER 15
	{
		HAL_GPIO_TogglePin(Laser_Tx_GPIO_Port, Laser_Tx_Pin);
	}
}

// FUNCTION      : EXTI15_10_IRQHandler()
// DESCRIPTION   : External Interrupt
// 		 		 : ISR for detecting Laser Signal.
//
// PARAMETERS    : NULL
// RETURNS       : NULL
void EXTI15_10_IRQHandler(void) {
	/* USER CODE BEGIN EXTI15_10_IRQn 0 */

	isLaserDetected = 1;

	/* USER CODE END EXTI15_10_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	/* USER CODE BEGIN EXTI15_10_IRQn 1 */

	/* USER CODE END EXTI15_10_IRQn 1 */
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t sequence = 0;
	char stringBuffer[16] = { 0 }; // Create a temporary buffer array to hold the data.
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM15_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim15);

#if 0
	HD44780_Init();									// Initialize LCD

	startUpLCDSplashScreen();

	HD44780_GotoXY(0, 0);			// Move cursor to First Line First Position.
	snprintf(stringBuffer, 16, "Pulse:");// write the data to a temporary buffer array.
	HD44780_PutStr(stringBuffer);				// Now write it actually to LCD.

	HD44780_GotoXY(0, 1);			// Move cursor to First Line First Position.
	snprintf(stringBuffer, 16, "Laser:");// write the data to a temporary buffer array.
	HD44780_PutStr(stringBuffer);				// Now write it actually to LCD.
#endif

	printf("\r\nStart Program\r\n");
	radio_init();        // Initialize radio channel

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		if (isLaserDetected) {
			HD44780_GotoXY(7, 1);	// Move cursor to First Line First Position.
			snprintf(stringBuffer, 16, "Detected");	// write the data to a temporary buffer array.
			HD44780_PutStr(stringBuffer);		// Now write it actually to LCD.

			/*
			 insert your code for transmitting data using nRF module
			 here.
			 */
			snprintf(stringBuffer, sizeof(stringBuffer), "%s-%d", "Detected", sequence);
	    radio_send_data(stringBuffer, strlen(stringBuffer));

			isLaserDetected = 0;
		} else {
			HD44780_GotoXY(7, 1);	// Move cursor to First Line First Position.
			snprintf(stringBuffer, 16, "        ");	// write the data to a temporary buffer array.
			HD44780_PutStr(stringBuffer);
		}

		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

		/* send data into nRF for just debugging */
	  snprintf(stringBuffer, sizeof(stringBuffer), "%s-%d", "Detected", sequence++);
	  printf("tx=%s\r\n", stringBuffer);
	  radio_send_data(stringBuffer, strlen(stringBuffer));

		/* receive data from nRF */
	  int len = radio_recv_data(stringBuffer, sizeof(stringBuffer));
		if (len) {
			printf("rx=%s\r\n", stringBuffer);
		}

		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration 
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */

// FUNCTION      : startUpLCDSplashScreen()
// DESCRIPTION   : displays "Assignment 5 GPS Parser" for 2s on line 1 of the display and disappears
// PARAMETERS    : None
// RETURNS       : Nothing
void startUpLCDSplashScreen(void) {
	char stringBuffer[16] = { 0 };// Create a temporary buffer array to hold the data.

	HD44780_GotoXY(0, 0);			// Move cursor to First Line First Position.
	snprintf(stringBuffer, 16, "CapstoneProject");	// write the data to a temporary buffer array.
	HD44780_PutStr(stringBuffer);				// Now write it actually to LCD.

	HD44780_GotoXY(0, 1);		// Move cursor to Second Line First Position.
	snprintf(stringBuffer, 16, "Test 1");// write the data to a temporary buffer array.
	HD44780_PutStr(stringBuffer);				// Now write it actually to LCD.

	HAL_Delay(2000);	// Wait for 2 Seconds let user see what this machine is.

	HD44780_ClrScr();								// Clear the LCD Screen.
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
