/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lpsSensorLib.h"
#include "lcd.h"

#include <stdio.h>

#include "../../hagl/include/font6x9.h"
#include "../../hagl/include/hagl.h"
#include "../../hagl/include/rgb565.h"

#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MEASUREMENT_DELAY 800

#define CAPTURED_VALUES_FROM_IR   64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int character)
{
	if(character == '\n')
	{
		int newLineBegin = '\r';
		HAL_UART_Transmit(&huart2, (uint8_t*)&newLineBegin, 1, HAL_MAX_DELAY);
	}

	HAL_UART_Transmit(&huart2, (uint8_t*)&character, 1, HAL_MAX_DELAY);

	return 1;
}

typedef enum {
	PULSE_9MS,
	PULSE_4MS,
	PULSE_2MS,
	PULSE_LONG,
	PULSE_SHORT,
	PULSE_ERROR,
}timeOfPulse;

static timeOfPulse definePulseBaseOnTime(uint32_t time)
{
	if(time < 250){
		return PULSE_ERROR;
	}
	else if (time < 1200){
		return PULSE_SHORT;
	}
	else if (time < 2000) {
		return PULSE_LONG;
	}
	else if (time < 3000) {
		return PULSE_2MS;
	}
	else if (time < 6000) {
		return PULSE_4MS;
	}
	else if (time < 12000) {
		return PULSE_9MS;
	}
	else{
		return PULSE_ERROR;
	}
}

static volatile uint32_t receivedValues;
static int receivedBits;

static void pulseHandler(timeOfPulse pulse)
{
	if(receivedBits >= 32){
		return;
	}

	switch(pulse){
	case PULSE_SHORT:
		receivedValues = receivedValues >> 1;
		receivedBits++;
		break;
	case PULSE_LONG:
		receivedValues = (receivedValues >> 1) | 0x80000000;
		receivedBits++;
		break;
	case PULSE_4MS:
	    receivedValues = 0;
	    receivedBits = 0;
	    break;
	case PULSE_2MS:
	    if (receivedBits == 0){
	      receivedBits = 32;
	    }
	    break;
	default:
		receivedBits = 0;
		break;
	}
}

volatile uint32_t values[CAPTURED_VALUES_FROM_IR];

int ir_counter;

int irPilotCaptureKey(void)
{
	if(receivedBits != 32){
		return -1;
	}

	uint8_t values = receivedValues >> 16;

	receivedBits = 0;

	return values;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  uint32_t new_val;

  if (htim == &htim2) {

	  switch (HAL_TIM_GetActiveChannel(&htim2)) {

	  	  case HAL_TIM_ACTIVE_CHANNEL_1:

	  		  new_val = definePulseBaseOnTime(HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1));

	  		  pulseHandler(new_val);
	  		  break;
	  	  default:
	  		  break;
	  }
  }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  printf("Looking for devices: \n");

  if(lpsSensorInit() == HAL_OK){
	  printf("LPS25HB ------------------------------------ OK");
  }else{
	  printf("LPS25HB ------------------------------NOT FOUND");
	  Error_Handler();
  }


  screenInit();
  for (int i = 0; i < 8; i++) {
    hagl_draw_rounded_rectangle(2+i, 2+i, 158-i, 126-i, 8-i, rgb565(0, 0, i*16));
  }
  hagl_put_text(L"Hello World!", 40, 55, YELLOW, font6x9);
  screenCopy();

  HAL_Delay(5000);

  typedef enum{
	    SCREEN_NONE = 0,
	    SCREEN_CHANNEL_1,
	    SCREEN_CHANNEL_2,
	    SCREEN_TEMPERATURE,
	    SCREEN_PRESSURE,
  } ScreenState;

  ScreenState currentScreen = SCREEN_NONE;

  while (1)
  {
	  int key = irPilotCaptureKey();

	      if (key != -1)
	      {
	          printf("Key: %02x\n", key);

	          switch (key)
	          {
	              case 0x0C:
	                  currentScreen = SCREEN_CHANNEL_1;
	                  break;

	              case 0x08:
	                  currentScreen = SCREEN_CHANNEL_2;
	                  break;

	              case 0x18:
	                  currentScreen = SCREEN_TEMPERATURE;
	                  break;

	              case 0x5E:
	                  currentScreen = SCREEN_PRESSURE;
	                  break;

	              case 0x5A:
	                  currentScreen = SCREEN_NONE;
	                  hagl_clear_screen();
	                  screenCopy();
	                  break;

	              case 0x1C:
	                  hagl_clear_screen();
	                  screenCopy();
	                  break;

	              default:
	                  break;
	          }
	      }

	      if (currentScreen != SCREEN_NONE)
	         {
	             hagl_clear_screen();

            	 char buf_char[32];
            	 wchar_t buf_wchar[32];

	             switch (currentScreen)
	             {
	                 case SCREEN_CHANNEL_1:
	                	 hagl_fill_rectangle(10, 50, 150, 80, BLACK);
	                	 sprintf(buf_char, "Kanał 1");
	                	 mbstowcs(buf_wchar, buf_char, strlen(buf_char)+1);
	                	 hagl_put_text(buf_wchar, 20, 55, WHITE, font6x9);
	                     break;

	                 case SCREEN_CHANNEL_2:
	                	 hagl_fill_rectangle(10, 50, 150, 80, BLACK);
	                	 sprintf(buf_char, "Kanał 2");
	                	 mbstowcs(buf_wchar, buf_char, strlen(buf_char)+1);
	                	 hagl_put_text(buf_wchar, 20, 55, WHITE, font6x9);
	                     break;

	                 case SCREEN_TEMPERATURE:
	                 {
	                     float temp = lpsSensorGetTemp();
	                     hagl_fill_rectangle(10, 50, 150, 80, BLACK);
	                     sprintf(buf_char, "P = %.1f C", temp);
	                     mbstowcs(buf_wchar, buf_char, strlen(buf_char)+1);
	                     hagl_put_text(buf_wchar, 20, 55, WHITE, font6x9);
	                     printf("Temp = %.1f (Celsius)\n", temp);

	                     break;
	                 }

	                 case SCREEN_PRESSURE:
	                 {
	                     float press = lpsSensorGetPress();
	                     hagl_fill_rectangle(10, 50, 150, 80, BLACK);
	                     sprintf(buf_char, "P = %.1f hPa", press);
	                     mbstowcs(buf_wchar, buf_char, strlen(buf_char)+1);
	                     hagl_put_text(buf_wchar, 20, 55, WHITE, font6x9);
	                     printf("Press = %.1f (hPascals)\n", press);

	                     break;
	                 }

	                 default:
	                     break;
	             }

	             screenCopy();
	         }

	  HAL_Delay(100);
//	  float temp = lpsSensorGetTemp();
//	  float press = lpsSensorGetPress();
//
//	  printf("Temp = %.1f (Celsius)\n", temp);
//	  printf("Press = %.1f (hPascals)\n", press);
//
//	  hagl_fill_rectangle(10, 50, 150, 80, BLACK);
//
//	  char buf_char[32];
//	  wchar_t buf_wchar[32];
//
//
//	  sprintf(buf_char, "T = %.1f C", temp);
//
//	  mbstowcs(buf_wchar, buf_char, strlen(buf_char)+1);
//
//	  hagl_put_text(buf_wchar, 20, 55, WHITE, font6x9);
//
//	  screenCopy();
//
//	  HAL_Delay(5000);
//
//	  int capturedKey = irPilotCaptureKey();
//
//	  if(capturedKey != -1){
//		  printf("Code: %02x\n", capturedKey);
//	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
