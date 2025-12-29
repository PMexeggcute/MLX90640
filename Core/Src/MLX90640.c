#include "LCD.h"
#include "cmsis_os2.h"
#include "main.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include <stdint.h>
#include "MLX90640.h"

MLX90640_HandleTypeDef mlx90640 = {0};

static void mlx90640_mergeData(uint16_t *dataBuff);
static uint8_t mlx90640_checkStatus(void);
static uint8_t mlx90640_getFrameData(void);
static void ExtractVDDParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractPTATParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractGainParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractTgcParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractResolutionParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractKsTaParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractKsToParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractCPParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractAlphaParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractOffsetParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractKtaPixelParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractKvPixelParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static void ExtractCILCParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);
static uint8_t ExtractDeviatingPixels(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640);


/**
 * @brief I2C memory read complete callback.
 * 
 * @param hi2c I2C handle
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  // Handle the received data here
  if(hi2c->Instance == hi2c1.Instance) {
    if(mlx90640.state == MLX90640_STATE_READING_SUBPAGE0) {
      mlx90640.state = MLX90640_STATE_COMPLETE_SUBPAGE0;
    } 
    else if(mlx90640.state == MLX90640_STATE_READING_SUBPAGE1) {
      mlx90640.state = MLX90640_STATE_COMPLETE_SUBPAGE1;
    }
  }
  LCD_ShowHexNum(2, 0, mlx90640.frameData[0], 4, Color_White, Color_Black);
  LCD_ShowHexNum(2, 5, mlx90640.frameData[1], 4, Color_White, Color_Black);
  LCD_ShowHexNum(2, 10, mlx90640.frameData[2], 4, Color_White, Color_Black);
  LCD_ShowHexNum(3, 0, mlx90640.frameData[768 - 4], 4, Color_White, Color_Black);
  LCD_ShowHexNum(3, 5, mlx90640.frameData[768 - 5], 4, Color_White, Color_Black);
  LCD_ShowHexNum(3, 10, mlx90640.frameData[768 - 6], 4, Color_White, Color_Black);
  // if(++i == 2) {
  //   mlx90640_mergeData();
  //   // i = 0;
  // }
  // for(int j = 0; j < 5; j++) {
  //   LCD_ShowHexNum(j + 1, 0, mlx90640_data[j], 2, Color_White, Color_Black);
  // }

  // LCD_ShowHexNum(2, 0, mlx90640_data[768 * 2 - 1], 2, Color_White, Color_Black);
  // LCD_ShowHexNum(2, 3, mlx90640_data[768 * 2 - 2], 2, Color_White, Color_Black);
  // LCD_ShowHexNum(2, 6, mlx90640_data[768 * 2 - 3], 2, Color_White, Color_Black);
  // LCD_ShowHexNum(2, 9, mlx90640_data[768 * 2 - 4], 2, Color_White, Color_Black);
}

/***
 * @brief Get the ID of the MLX90640 sensor.
 * 
 * @return The error code of the I2C operation.
 */
uint8_t mlx90640_getID(void) {
    uint8_t error = MLX90640_I2C_NACK_ERROR;
    error = HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_ID_REG, I2C_MEMADD_SIZE_16BIT, mlx90640.ID, 3, MLX90640_DELAY_MS);
    return error;
}

/***
 * @brief Check the status register of the MLX90640 sensor.
 * 
 * @return The status byte or an error code. 0x08 indicates subpage 0 ready, 0x09 indicates subpage 1 ready.
 */
static uint8_t mlx90640_checkStatus(void) {
    uint8_t error = MLX90640_I2C_NACK_ERROR;
    uint8_t status[2] = {0};
    error = HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, 
        I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);
        if(error != HAL_OK) {
            return error;
        }
    return status[1];
}

/**
 * @brief Get the frame data from the MLX90640 sensor.
 * 
 * @return The error code of the I2C operation or timeout.
 */
static uint8_t mlx90640_getFrameData(void) {
    uint16_t dataBuff[MLX90640_PIXEL_LENGTH] = {0};
    uint8_t error = MLX90640_I2C_NACK_ERROR;

    if(mlx90640.state != MLX90640_STATE_IDLE) {
        return MLX90640_BUSY_ERROR;
    }

    uint16_t timeout = 0;
    while(mlx90640_checkStatus() != 0x08) {
        timeout++;
        HAL_Delay(1);
        if(timeout > 1000){
            return MLX90640_TIMEOUT_ERROR;
        }
    }

    mlx90640.state = MLX90640_STATE_READING_SUBPAGE0;
    error = HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_RAM_START, 
        I2C_MEMADD_SIZE_16BIT, (uint8_t *)mlx90640.frameData - 1, MLX90640_PIXEL_LENGTH * 2);
    if (error != HAL_OK) {
        return error;
    }

    timeout = 0;
    while(mlx90640_checkStatus() != 0x09 || mlx90640.state != MLX90640_STATE_COMPLETE_SUBPAGE0) {
        timeout++;
        HAL_Delay(1);
        if(timeout > 1000){
            return MLX90640_TIMEOUT_ERROR;
        }
    }
    
    mlx90640.state = MLX90640_STATE_READING_SUBPAGE1;
    error = HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_RAM_START, 
        MLX90640_AUXDATA_START, (uint8_t *)dataBuff - 1, MLX90640_PIXEL_LENGTH * 2);
    if (error != HAL_OK) {
        return error;
    }

    while (mlx90640.state != MLX90640_STATE_COMPLETE_SUBPAGE1) {
        osDelay(1);
        timeout++;
        if(timeout > 1000){
            return MLX90640_TIMEOUT_ERROR;
        }
    }
    mlx90640_mergeData(dataBuff);

    error = HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_AUXDATA_START, 
        MLX90640_AUXDATA_START, (uint8_t *)mlx90640.frameData + MLX90640_PIXEL_LENGTH * 2, MLX90640_AUXDATA_LENGTH);
    if (error != HAL_OK) {
        return error;   
    }

    return MLX90640_NO_ERROR;
}

/**
 * @brief Merge the data from both subpages.
 * 
 * @param dataBuff The buffer containing the second subpage data.
 */
static void mlx90640_mergeData(uint16_t *dataBuff) {
    for(int i=1; i<MLX90640_FRAMEDATA_LENGTH; i += 2) {
        mlx90640.frameData[i] = dataBuff[i];
    }
    mlx90640.state = MLX90640_STATE_IDLE;
}

/**
 * @brief Dump the EEPROM data from the MLX90640 sensor.
 * 
 * @return uint8_t The error code of the I2C operation.
 */
static uint8_t mlx90640_dumpEEPROM(void) {
    uint8_t error = MLX90640_I2C_NACK_ERROR;
    error = HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_EEPROM_START, 
        I2C_MEMADD_SIZE_16BIT, (uint8_t *)mlx90640.mlx90640_EEPROM_Data - 1, MLX90640_EEPROM_DUMP_LENGTH * 2);
    if(error != HAL_OK) {
        return error;
    }
    return MLX90640_NO_ERROR;
}

/**
 * @brief Extract the parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 * @return int The error code.
 */
int MLX90640_ExtractParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int error = MLX90640_NO_ERROR;
    
    ExtractVDDParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractPTATParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractGainParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractTgcParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractResolutionParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractKsTaParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractKsToParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractCPParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractAlphaParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractOffsetParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractKtaPixelParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractKvPixelParameters(mlx90640_EEPROM_Data, mlx90640);
    ExtractCILCParameters(mlx90640_EEPROM_Data, mlx90640);
    error = ExtractDeviatingPixels(mlx90640_EEPROM_Data, mlx90640);  
    
    return error;

}

/**
 * @brief Extract VDD parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractVDDParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {

}

/**
 * @brief Calculate the temperature from the frame data.
 * 
 * @param frameData The frame data.
 * @param mlx90640 The MLX90640 handle.
 * @param emissivity The emissivity of the object.
 * @param tr The temperature of the object.
 * @param result The result array.
 */
void MLX90640_calculateTo(uint16_t *frameData, const MLX90640_HandleTypeDef *mlx90640, float emissivity, float tr, float *result) {

}

void mlx90640_test(void) {
    HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_EEPROM_START, 
        I2C_MEMADD_SIZE_16BIT, mlx90640.mlx90640_EEPROM_Data, MLX90640_EEPROM_DUMP_LENGTH);
}
