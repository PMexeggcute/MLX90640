#include "LCD.h"
#include "cmsis_os2.h"
#include "main.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include <stdint.h>
#include "MLX90640.h"
#include <math.h>

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
    int8_t kvdd = (int8_t)(mlx90640_EEPROM_Data[0x33] >> 8);
    int16_t vdd25 = (int16_t)(mlx90640_EEPROM_Data[0x33] & 0xFF);
    vdd25 = (vdd25  - 256) * 32 - 8192;

    mlx90640->kVdd = kvdd;
    mlx90640->vdd25 = vdd25;
}

/**
 * @brief Extract PTAT parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractPTATParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    float KvPTAT;
    float KtPTAT;
    int16_t vPTAT25;
    float alphaPTAT;

    KvPTAT = (mlx90640_EEPROM_Data[0x32] & 0xFC00) >> 10;
    if(KvPTAT > 31) {
        KvPTAT = KvPTAT - 64;
    }
    KvPTAT = KvPTAT / 4096;

    KtPTAT = mlx90640_EEPROM_Data[0x32] & 0x03FF;
    if(KtPTAT > 511) {
        KtPTAT = KtPTAT - 1024;
    }
    KtPTAT = KtPTAT / 8;

    vPTAT25 = mlx90640_EEPROM_Data[0x31];

    alphaPTAT = ((mlx90640_EEPROM_Data[0x10] & 0xF000) >> 14) + 8.0f;

    mlx90640->KvPTAT = KvPTAT;
    mlx90640->KtPTAT = KtPTAT;
    mlx90640->vPTAT25 = vPTAT25;
    mlx90640->alphaPTAT = alphaPTAT;
}

/**
 * @brief Extract gain parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractGainParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    mlx90640->gain = (int16_t)mlx90640_EEPROM_Data[0x30];
}

/**
 * @brief Extract TGC parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractTgcParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int8_t tgc = (int8_t)(mlx90640_EEPROM_Data[0x3C] >> 8);
    mlx90640->tgc = tgc / 32.0f;
}

/**
 * @brief Extract resolution parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractResolutionParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    uint8_t resolution = 0;
    resolution = (mlx90640_EEPROM_Data[0x38] & 0x0300) >> 12;

    mlx90640->resolution = resolution;
}

/**
 * @brief Extract KsTa parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractKsTaParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    mlx90640->ksTa = (int8_t)(mlx90640_EEPROM_Data[0x3C] & 0xFF00) / 8192.0f;
}

/**
 * @brief Extract KsTo parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractKsToParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int32_t KsToScale;
    int8_t step;

    step = (mlx90640_EEPROM_Data[0x3F] * 0x3000 >> 12) * 10;

    mlx90640->ct[0] = -40;
    mlx90640->ct[1] = 0;
    mlx90640->ct[2] = ((mlx90640_EEPROM_Data[0x3F] & 0x00F0) >> 4) * step;
    mlx90640->ct[3] = ((mlx90640_EEPROM_Data[0x3F] & 0x0F00) >> 8) * step + mlx90640->ct[2];
    mlx90640->ct[4] = 400;  //Empirical constants

    KsToScale = mlx90640_EEPROM_Data[0x3F] & 0x000F + 8;
    KsToScale = 1UL << KsToScale;

    mlx90640->ksTo[0] = (int8_t)(mlx90640_EEPROM_Data[0x3D] & 0x00FF) / (float)KsToScale;
    mlx90640->ksTo[1] = (int8_t)(mlx90640_EEPROM_Data[0x3D] & 0xFF00) / (float)KsToScale;
    mlx90640->ksTo[2] = (int8_t)(mlx90640_EEPROM_Data[0x3E] & 0x00FF) / (float)KsToScale;
    mlx90640->ksTo[3] = (int8_t)(mlx90640_EEPROM_Data[0x3E] & 0xFF00) / (float)KsToScale;
    mlx90640->ksTo[4] = -0.0002f;  //Empirical constant
}

/**
 * @brief Extract CP parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractCPParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    float alphaSP[2];
    int16_t offsetSP[2];
    float cpKv;
    float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale;
    uint8_t kvScale;

    alphaScale = (mlx90640_EEPROM_Data[0x20] & 0xF000) >> 12;
    alphaScale += 27;

    offsetSP[0] = mlx90640_EEPROM_Data[0x3A] & 0x03FF;
    if(offsetSP[0] > 511) {
        offsetSP[0] = offsetSP[0] - 1024;
    }

    offsetSP[1] = (mlx90640_EEPROM_Data[0x3A] & 0xFC00) >> 10;
    if(offsetSP[1] > 31) {
        offsetSP[1] = offsetSP[1] - 64;
    }
    offsetSP[1] = offsetSP[1] + offsetSP[0];

    alphaSP[0] = mlx90640_EEPROM_Data[0x39] & 0x03FF;
    if(alphaSP[0] > 511) {
        alphaSP[0] = alphaSP[0] - 1024;
    }
    alphaSP[0] = alphaSP[0] / (float)(1 << alphaScale);

    alphaSP[1] = (mlx90640_EEPROM_Data[0x39] & 0xFC00) >> 10;
    if(alphaSP[1] > 31) {
        alphaSP[1] = alphaSP[1] - 64;
    }
    alphaSP[1] = (1 + alphaSP[1] / 128.0f) * alphaSP[0];

    cpKta = (int8_t)mlx90640_EEPROM_Data[0x3B] * 0x00FF;

    ktaScale = (int8_t)((mlx90640_EEPROM_Data[0x3B] * 0x00F0) >> 4) + 8;

    mlx90640->cpKta = cpKta / (float)(1 << ktaScale);

    cpKv = (int8_t)(mlx90640_EEPROM_Data[0x3B] & 0xFF00) >> 8;

    kvScale = (mlx90640_EEPROM_Data[0x3B] & 0x0F00) >> 8;

    mlx90640->cpKv = cpKv / (float)(1 << kvScale);

    mlx90640->cpAlpha[0] = alphaSP[0];
    mlx90640->cpAlpha[1] = alphaSP[1];
    mlx90640->cpOffset[0] = offsetSP[0];
    mlx90640->cpOffset[1] = offsetSP[1];
}

/**
 * @brief Extract Alpha parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractAlphaParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int accRow[24];
    int accColumn[32];
    int p = 0;
    int alphaRef;
    uint8_t alphaScale;
    uint8_t accRowScale;
    uint8_t accColumnScale;
    uint8_t accRemScale;
    float alphaTemp[768];
    float temp;

    accRemScale = mlx90640_EEPROM_Data[0x20] & 0x000F;
    accColumnScale = (mlx90640_EEPROM_Data[0x20] & 0x00F0) >> 4;
    accRowScale = (mlx90640_EEPROM_Data[0x20] & 0x0F00) >> 8;
    alphaScale = ((mlx90640_EEPROM_Data[0x20] & 0xF000) >> 12) + 30;
    alphaRef = mlx90640_EEPROM_Data[0x21];

    for(int i = 0; i < 6; i++) {
        p = i * 4;
        accRow[p + 0] = mlx90640_EEPROM_Data[0x22 + i] & 0x000F;
        accRow[p + 1] = (mlx90640_EEPROM_Data[0x22 + i] & 0x00F0) >> 4;
        accRow[p + 2] = (mlx90640_EEPROM_Data[0x22 + i] & 0x0F00) >> 8;
        accRow[p + 3] = (mlx90640_EEPROM_Data[0x22 + i] & 0xF000) >> 12;
    }

    for(int i = 0; i < MLX90640_LINE_NUMBER; i++) {
        if(accRow[i] > 7) {
            accRow[i] = accRow[i] - 16;
        }
    }

    for(int i = 0; i < 8; i++) {
        p  = i * 4;
        accColumn[p + 0] = mlx90640_EEPROM_Data[0x28 + i] & 0x000F;
        accColumn[p + 1] = (mlx90640_EEPROM_Data[0x28 + i] & 0x00F0) >> 4;
        accColumn[p + 2] = (mlx90640_EEPROM_Data[0x28 + i] & 0x0F00) >> 8;
        accColumn[p + 3] = (mlx90640_EEPROM_Data[0x28 + i] & 0xF000) >> 12;
    }

    for(int i = 0; i < MLX90640_COLUMN_NUMBER; i++) {
        if(accColumn[i] > 7) {
            accColumn[i] = accColumn[i] - 16;
        }
    }

    for(int i = 0; i < MLX90640_LINE_NUMBER; i++) {
        for(int j = 0; j < MLX90640_COLUMN_NUMBER; j++) {
            p = 32 * i + j;
            alphaTemp[p] = (mlx90640_EEPROM_Data[0x40 + p] & 0x03F0) >> 4;
            if(alphaTemp[p] > 31) {
                alphaTemp[p] = alphaTemp[p] - 64;
            }
            alphaTemp[p] = alphaTemp[p] * (1 << accRemScale);
            alphaTemp[p] = (alphaRef + (accRow[i] << accRowScale) + (accColumn[j] << accColumnScale) + alphaTemp[p]);
            alphaTemp[p] = alphaTemp[p] / (float)(1 << alphaScale);
            alphaTemp[p] = alphaTemp[p] - mlx90640->tgc * (mlx90640->cpAlpha[0] + mlx90640->cpAlpha[1]) / 2.0f;
            alphaTemp[p] = SCALEPHA/alphaTemp[p];
        }
    }

    temp = alphaTemp[0];
    for(int i = 0; i < MLX90640_PIXEL_LENGTH; i++) {
        if(alphaTemp[i] > temp) {
            temp = alphaTemp[i];
        }
    }

    alphaScale = 0;
    while(temp < 32767.4) {
        temp = temp * 2;
        alphaScale = alphaScale + 1;
    }

    for(int i = 0; i < MLX90640_PIXEL_LENGTH; i++) {
        temp = alphaTemp[i] * (1 << alphaScale);
        mlx90640->alpha[i] = (uint16_t)(temp + 0.5);
    }

    mlx90640->alphaScale = alphaScale;

}

/**
 * @brief Extract Offset parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractOffsetParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int occRow[24];
    int occColumn[32];
    int p = 0;
    int16_t offsetRef;
    uint8_t occRowScale;
    uint8_t occColumnScale;
    uint8_t occRemScale;

    offsetRef = (int16_t)mlx90640_EEPROM_Data[0x11];
    occRowScale = (mlx90640_EEPROM_Data[0x10] & 0x0F00) >> 8;
    occColumnScale = (mlx90640_EEPROM_Data[0x10] & 0x00F0) >> 4;
    occRemScale = mlx90640_EEPROM_Data[0x10] & 0x000F;

    for(int i = 0; i < 6; i++) {
        p = i * 4;
        occRow[p + 0] = mlx90640_EEPROM_Data[0x12 + i] & 0x000F;
        occRow[p + 1] = (mlx90640_EEPROM_Data[0x12 + i] & 0x00F0) >> 4;
        occRow[p + 2] = (mlx90640_EEPROM_Data[0x12 + i] & 0x0F00) >> 8;
        occRow[p + 3] = (mlx90640_EEPROM_Data[0x12 + i] & 0xF000) >> 12;
    }

    for(int i = 0; i < MLX90640_LINE_NUMBER; i++) {
        if(occRow[i] > 7) {
            occRow[i] = occRow[i] - 16;
        }
    }

    for(int i = 0; i < 8; i++) {
        p  = i * 4;
        occColumn[p + 0] = mlx90640_EEPROM_Data[0x18 + i] & 0x000F;
        occColumn[p + 1] = (mlx90640_EEPROM_Data[0x18 + i] & 0x00F0) >> 4;
        occColumn[p + 2] = (mlx90640_EEPROM_Data[0x18 + i] & 0x0F00) >> 8;
        occColumn[p + 3] = (mlx90640_EEPROM_Data[0x18 + i] & 0xF000) >> 12;
    }

    for(int i = 0; i < MLX90640_COLUMN_NUMBER; i++) {
        if(occColumn[i] > 7) {
            occColumn[i] = occColumn[i] - 16;
        }
    }

    for(int i = 0; i < MLX90640_LINE_NUMBER; i++) {
        for(int j = 0; j < MLX90640_COLUMN_NUMBER; j++) {
            p = 32 * i + j;
            mlx90640->offset[p] = (mlx90640_EEPROM_Data[0x40 + p] & 0xFC00) >> 10;
            if(mlx90640->offset[p] > 31) {
                mlx90640->offset[p] = mlx90640->offset[p] - 64;
            }
            mlx90640->offset[p] = mlx90640->offset[p] * (1 << occRemScale);
            mlx90640->offset[p] = offsetRef + (occRow[i] << occRowScale) + (occColumn[j] << occColumnScale) + mlx90640->offset[p];
        }
    }
}

/**
 * @brief Extract Kta pixel parameters from the EEPROM data.
 * 
 * @param mlx90640_EEPROM_Data The EEPROM data.
 * @param mlx90640 The MLX90640 handle.
 */
static void ExtractKtaPixelParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int p = 0;
    int8_t ktaRC[4];
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    uint8_t split;
    float ktaTemp[768];
    float temp;

    ktaRC[0] = (int8_t)(mlx90640_EEPROM_Data[0x36] * 0xFF00) >> 8;
    ktaRC[2] = (int8_t)(mlx90640_EEPROM_Data[0x36] & 0x00FF);
    ktaRC[1] = (int8_t)(mlx90640_EEPROM_Data[0x37] * 0xFF00) >> 8;
    ktaRC[3] = (int8_t)(mlx90640_EEPROM_Data[0x37] & 0x00FF);

    ktaScale1 = ((mlx90640_EEPROM_Data[0x38] & 0x00F0) >> 4) + 8;
    ktaScale2 = (mlx90640_EEPROM_Data[0x38] & 0x000F);

    for(int i = 0; i < MLX90640_LINE_NUMBER; i++) {
        for(int j = 0; j < MLX90640_COLUMN_NUMBER; j++) {
            p = 32 * i + j;
            split = 2 * (i % 2) + (j % 2);
            ktaTemp[p] = (mlx90640_EEPROM_Data[0x40 + p] & 0x000E) >> 1;
            if(ktaTemp[p] > 3) {
                ktaTemp[p] = ktaTemp[p] - 8;
            }
            ktaTemp[p] = ktaTemp[p] * (1 << ktaScale2);
            ktaTemp[p] = ktaRC[split] + ktaTemp[p];
            ktaTemp[p] = ktaTemp[p] / (float)(1 << ktaScale1);
        }
    }

    temp = fabs(ktaTemp[0]);
    for(int i = 1; i < MLX90640_PIXEL_LENGTH; i++) {
        if(fabs(ktaTemp[i]) > temp) {
            temp = fabs(ktaTemp[i]);
        }
    }
    ktaScale1 = 0;
    while(temp < 63.4) {
        temp *= 2;
        ktaScale1 ++;
    }

    for(int i = 0; i < MLX90640_PIXEL_LENGTH; i++) {
        temp = ktaTemp[i] * (float)(1 << ktaScale1);
        if(temp < 0) {
            mlx90640->kta[i] = (temp - 0.5);
        }
        else {
            mlx90640->kta[i] = (temp + 0.5);
        }

    }

    mlx90640->ktaScale = ktaScale1;
}

static void ExtractKvPixelParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    int p = 0;
    int8_t KvT[4];
    int8_t KvRoCo;
    int8_t KvRoCe;
    int8_t KvReCo;
    int8_t KvReCe;
    int8_t kvScale;
    uint8_t split;
    float kvTemp[768];
    float temp;

    KvRoCo = (int8_t)(mlx90640_EEPROM_Data[0x34] & 0xF000) >> 12;
    if(KvRoCo > 7) {
        KvRoCo -= 16;
    }
    KvT[0] = KvRoCo;

    KvReCo = (int8_t)(mlx90640_EEPROM_Data[0x34] & 0x0F00) >> 8;
    if(KvReCo > 7) {
        KvReCo -= 16;
    }
    KvT[2] = KvReCo;

    KvRoCe = (int8_t)(mlx90640_EEPROM_Data[0x34] & 0x00F0) >> 4;
    if(KvRoCe > 7) {
        KvRoCe -= 16;
    }
    KvT[1] = KvRoCe;
    
    KvReCe = (int8_t)mlx90640_EEPROM_Data[0x34] & 0x000F;
    if(KvReCe > 7) {
        KvReCe -= 16;
    }
    KvT[3] = KvReCe;
    
    kvScale = (mlx90640_EEPROM_Data[0x38] & 0x0F00) >> 8;

    for(int i = 0; i < MLX90640_LINE_NUMBER; i++) {
        for(int j = 0; j < MLX90640_COLUMN_NUMBER; j++) {
            p = 32 * i + j;
            split = 2 * (i % 2) + (j % 2);
            kvTemp[p] = KvT[split];
            kvTemp[p] = kvTemp[p] / (float)(1 << kvScale);
        }
    }

    temp = fabs(kvTemp[0]);
    for(int i = 0; i < MLX90640_PIXEL_LENGTH; i++) {
        if(fabs(kvTemp[i]) > temp) {
            temp = fabs(kvTemp[i]);
        }
    }

    kvScale = 0;
    while(temp < 63.4) {
        temp *= 2;
        kvScale = kvScale + 1;
    }

    for(int i = 0; i < MLX90640_PIXEL_LENGTH; i++) {
        temp = kvTemp[i] * (float)(1 << kvScale);
        if(temp < 0) {
            mlx90640->kv[i] = (temp - 0.5);
        }
        else {
            mlx90640->kv[i] = (temp + 0.5);
        }

    }

    mlx90640->kvScale = kvScale;
}

/**
 * @brief 
 * 
 * @param mlx90640_EEPROM_Data 
 * @param mlx90640 
 */
static void ExtractCILCParameters(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {

}

/**
 * @brief 
 * 
 * @param mlx90640_EEPROM_Data 
 * @param mlx90640 
 * @return uint8_t 
 */
uint8_t ExtractDeviatingPixels(uint16_t *mlx90640_EEPROM_Data, MLX90640_HandleTypeDef *mlx90640) {
    
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
        I2C_MEMADD_SIZE_16BIT, (uint8_t *)mlx90640.mlx90640_EEPROM_Data, MLX90640_EEPROM_DUMP_LENGTH);
}
