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
#include "LCD.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// #include "LCD.h"
// #include "MLX90640_I2C_Driver.h"
// #include <stdint.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdarg.h>
// #include "IMG.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */




uint16_t frame[834] = {0};
uint16_t eeMLX90640[832] = {0};
float mlx90640To[768] = {0};
paramsMLX90640 mlx90640 = {0};
float emissivity=0.95;
uint16_t mlx90640_step=0;

// uint16_t show_list[128*160];


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *I2c_Handle){
// 	mlx90640_step=1;
// 	MLX90640_GetFrameData_IT(MLX90640_ADDR, frame,&mlx90640_step);
// }

// int __io_putchar(int ch)
// {
//     HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//     return ch;
// }

void my_printf(const char *format, ...)
{
    char buffer[256];  // 缓冲区大小可以根据需要调整
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
}

// void uart_printf(const char *format, ...)
// {
//     char buffer[256];
//     va_list args;
//     va_start(args, format);
//     vsnprintf(buffer, sizeof(buffer), format, args);
//     va_end(args);
//     HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
// }

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  // float ta, tr;

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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  // my_printf("test\n");


  LCD_Init();
  LCD_FillScreen(0x0000);

  // LCD_ShowSignedNumTrpbg(1, 1, 12, 3, Color_White);
  // LCD_ShowSignedNum(2, 1, 12, 3, Color_White, Color_Black);

  // uint16_t test = 0;  
  // MLX90640_I2CRead(MLX90640_ADDR, 0x800D, 1, &test);
  // LCD_ShowHexNum(1, 0, test, 4, Color_White, Color_Black);

  //init and config MLX90640
  MLX90640_SetRefreshRate(MLX90640_ADDR, 5);
  uint8_t error = MLX90640_SetChessMode(MLX90640_ADDR);
  if(error!= 0) {
    LCD_ShowString(0, 0, "set chess mode error", Color_White, Color_Black);
  }

  MLX90640_DumpEE(MLX90640_ADDR, eeMLX90640);
  MLX90640_ExtractParameters(eeMLX90640, &mlx90640);



  // uint16_t statusRegister;
  // do {
  //     MLX90640_I2CRead(MLX90640_ADDR, MLX90640_STATUS_REG, 1, &statusRegister);
  //     HAL_Delay(1);
  //     my_printf("waiting for subpage 0\n");
  // } while((statusRegister & 0x000F) != 0b1000);

  // // uint16_t test;

  // // MLX90640_I2CRead(MLX90640_ADDR, 0x700, 1, &test);
  // // my_printf("data read 420: %x\n", test);




  // uint8_t subpage = MLX90640_GetFrameData(MLX90640_ADDR, frame);
  // if(subpage != 0) {
  //   my_printf("getting subpage0 error\n");
  // }
  // my_printf("got subpage0\n");

  // // for(int i = 0; i < 768; i++) {
  // //   my_printf("sp1[%d]: %x\n", i, frame[i]);
  // // }



  // ta = MLX90640_GetTa(frame, &mlx90640);
  // tr = ta - TA_SHIFT;
  // MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, mlx90640To);

  // subpage = MLX90640_GetFrameData(MLX90640_ADDR, frame);
  // if(subpage != 1) {
  //   my_printf("getting subpage0 error\n");
  // }
  // my_printf("got subpage1\n");


  // // for(int i = 0; i < 768; i++) {
  // //   my_printf("sp2[%d]: %x\n", i, frame[i]);
  // // }




  // ta = MLX90640_GetTa(frame, &mlx90640);
  // tr = ta - TA_SHIFT;
  // MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, mlx90640To);

  // // my_printf("data[0]: %x && data[420]: %x\n", frame[0],frame[420]);

  // my_printf("data ready\n");




  // // for(int i = 0; i < 768; i++) {
  // //   my_printf("temp[%d]: %f\n", i, mlx90640To[i]);
  // // }

  
  // // MLX90640_I2CRead(MLX90640_ADDR, 0x710, 1, &test);
  // // my_printf("data read 0x710: %x\n", test);


  // float maxTemp, minTemp = 0;
  // uint16_t maxAddr[2], minAddr[2];
  // temp_limit(mlx90640To, &maxTemp, maxAddr, &minTemp, minAddr);
  // uint16_t color_list[256];
  // color_listcode(color_list, 1);
  // display_code(mlx90640To, color_list, 1, maxTemp, minTemp);

  // for(uint8_t i = 0; i < 128; i++) {
  //   for(uint8_t j = 0; j < 160; j++) {
  //     LCD_DrawPoint(i, j, show_list[i*160+j]);
  //   }
  // }


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
