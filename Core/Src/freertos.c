/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "LCD.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"
#include "MLX90640_API.h"
#include <stdint.h>
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MLX90640Show */
osThreadId_t MLX90640ShowHandle;
const osThreadAttr_t MLX90640Show_attributes = {
  .name = "MLX90640Show",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void MLX90640GetDataTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of MLX90640Show */
  MLX90640ShowHandle = osThreadNew(MLX90640GetDataTask, NULL, &MLX90640Show_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {



    osDelay(1);


  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_MLX90640GetDataTask */
/**
* @brief Function implementing the MLX90640GetData thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MLX90640GetDataTask */
__weak void MLX90640GetDataTask(void *argument)
{
  /* USER CODE BEGIN MLX90640GetDataTask */
  /* Infinite loop */
  for(;;)
  {

    
    float ta, tr;
    uint16_t statusRegister;
    do {
        MLX90640_I2CRead(MLX90640_ADDR, MLX90640_STATUS_REG, 1, &statusRegister);
        osDelay(1);
        my_printf("waiting for subpage 0\n");
    } while((statusRegister & 0x000F) != 0b1000);

    // uint16_t test;

    // MLX90640_I2CRead(MLX90640_ADDR, 0x700, 1, &test);
    // my_printf("data read 420: %x\n", test);




    uint8_t subpage = MLX90640_GetFrameData(MLX90640_ADDR, frame);
    if(subpage != 0) {
      my_printf("getting subpage0 error\n");
    }
    my_printf("got subpage0\n");

    // for(int i = 0; i < 768; i++) {
    //   my_printf("sp1[%d]: %x\n", i, frame[i]);
    // }



    ta = MLX90640_GetTa(frame, &mlx90640);
    tr = ta - TA_SHIFT;
    float emissivity = 0.95;
    MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, mlx90640To);

    subpage = MLX90640_GetFrameData(MLX90640_ADDR, frame);
    if(subpage != 1) {
      my_printf("getting subpage0 error\n");
    }
    my_printf("got subpage1\n");


    // for(int i = 0; i < 768; i++) {
    //   my_printf("sp2[%d]: %x\n", i, frame[i]);
    // }




    ta = MLX90640_GetTa(frame, &mlx90640);
    tr = ta - TA_SHIFT;
    MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, mlx90640To);

    // my_printf("data[0]: %x && data[420]: %x\n", frame[0],frame[420]);

    my_printf("data ready\n");




    // for(int i = 0; i < 768; i++) {
    //   my_printf("temp[%d]: %f\n", i, mlx90640To[i]);
    // }

    
    // MLX90640_I2CRead(MLX90640_ADDR, 0x710, 1, &test);
    // my_printf("data read 0x710: %x\n", test);


    float maxTemp, minTemp = 0;
    uint16_t maxAddr[2], minAddr[2];
    temp_limit(mlx90640To, &maxTemp, maxAddr, &minTemp, minAddr);
    uint16_t color_list[256];
    color_listcode(color_list, 1);
    display_code(mlx90640To, color_list, 1, maxTemp, minTemp);
    LCD_ShowStringTrpbg(0, 0, "max:", Color_Black);
    LCD_ShowSignedNumTrpbg(0, 4, (uint32_t)maxTemp, 3, Color_Black);
    LCD_ShowStringTrpbg(0, 8, "min:", Color_Black);
    LCD_ShowSignedNumTrpbg(0, 12, (uint32_t)minTemp, 3, Color_Black);


    osDelay(1);
  }
  /* USER CODE END MLX90640GetDataTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

