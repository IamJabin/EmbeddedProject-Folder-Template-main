/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ITM_SWO_BAUDRATE_HZ                 1000000UL
#define LOGIC_ANALYZER_TEST_GPIO_PORT        GPIOB
#define LOGIC_ANALYZER_TEST_GPIO_PIN         GPIO_PIN_12
#define LOGIC_ANALYZER_TOGGLE_INTERVAL_MS    100UL
#define DEBUG_NUMBER_MAX_EXCLUSIVE           1000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int a;
int numbers[] = {
  532, 172, 428, 635, 887, 763, 5356, 123, 575,
  654, 986, 456, 678, 987, 101, 145, 90
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void ITM_TraceInit(void);
static void LogicAnalyzerTestPinInit(void);
static void array_fill_numbers(int *p_numbers, size_t length);
static void swap_numbers(int *p_left, int *p_right);
static void bubble_sort(int *p_numbers, size_t length);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f)
{
  (void)f;

  /* Do not let an unavailable trace sink stall the application. */
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&
      ((ITM->TER & 1UL) != 0UL) &&
      (ITM->PORT[0U].u32 != 0UL))
  {
    ITM->PORT[0U].u8 = (uint8_t)ch;
  }

  return ch;
}

static void ITM_TraceInit(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  MODIFY_REG(DBGMCU->CR,
             DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE,
             DBGMCU_CR_TRACE_IOEN);

  ITM->LAR = 0xC5ACCE55UL;
  TPI->ACPR = (SystemCoreClock / ITM_SWO_BAUDRATE_HZ) - 1UL;
  TPI->SPPR = 2UL;
  TPI->FFCR = 0UL;

  ITM->TPR = 0UL;
  ITM->TER = 1UL;
  ITM->TCR = ITM_TCR_ITMENA_Msk |
             ITM_TCR_SYNCENA_Msk |
             ITM_TCR_DWTENA_Msk |
             (1UL << ITM_TCR_TraceBusID_Pos);
}

static void LogicAnalyzerTestPinInit(void)
{
  GPIO_InitTypeDef gpio_init = {0};

  HAL_GPIO_WritePin(LOGIC_ANALYZER_TEST_GPIO_PORT,
                    LOGIC_ANALYZER_TEST_GPIO_PIN,
                    GPIO_PIN_RESET);

  gpio_init.Pin = LOGIC_ANALYZER_TEST_GPIO_PIN;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LOGIC_ANALYZER_TEST_GPIO_PORT, &gpio_init);
}

static void array_fill_numbers(int *p_numbers, size_t length)
{
  size_t index = 0U;

  if (p_numbers == NULL)
  {
    return;
  }

  for (index = 0U; index < length; ++index)
  {
    p_numbers[index] = rand() % DEBUG_NUMBER_MAX_EXCLUSIVE;
  }
}

static void swap_numbers(int *p_left, int *p_right)
{
  int temporary = 0;

  if ((p_left == NULL) || (p_right == NULL))
  {
    return;
  }

  temporary = *p_left;
  *p_left = *p_right;
  *p_right = temporary;
}

static void bubble_sort(int *p_numbers, size_t length)
{
  size_t pass = 0U;
  size_t index = 0U;
  uint8_t was_swapped = 0U;

  if ((p_numbers == NULL) || (length < 2U))
  {
    return;
  }

  for (pass = 0U; pass < (length - 1U); ++pass)
  {
    was_swapped = 0U;

    for (index = 0U; index < (length - 1U - pass); ++index)
    {
      if (p_numbers[index] > p_numbers[index + 1U])
      {
        swap_numbers(&p_numbers[index], &p_numbers[index + 1U]);
        was_swapped = 1U;
      }
    }

    if (was_swapped == 0U)
    {
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
  ITM_TraceInit();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  const size_t numbers_count = sizeof(numbers) / sizeof(numbers[0]);

  LogicAnalyzerTestPinInit();
  array_fill_numbers(numbers, numbers_count);
  bubble_sort(numbers, numbers_count);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_GPIO_TogglePin(LOGIC_ANALYZER_TEST_GPIO_PORT,
                       LOGIC_ANALYZER_TEST_GPIO_PIN);
    ++a;
    printf("A\r\n");
    HAL_Delay(LOGIC_ANALYZER_TOGGLE_INTERVAL_MS);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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

#ifdef  USE_FULL_ASSERT
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
