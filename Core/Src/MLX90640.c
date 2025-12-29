#include "LCD.h"
#include "cmsis_os2.h"
#include "main.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include "MLX90640.h"

MLX90640_HandleTypeDef mlx90640 = {0};

static void mlx90640_mergeData(uint16_t *dataBuff);

uint8_t mlx90640_caliData[MLX90640_FRAMEDATA_LENGTH] = {0};

uint8_t mlx90640_getID(void) {
    uint8_t error = MLX90640_I2C_NACK_ERROR;
    error = HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_ID_REG, I2C_MEMADD_SIZE_16BIT, mlx90640.ID, 3, MLX90640_DELAY_MS);
    return error;
}

/***
 * Check the status of the MLX90640 sensor.
 * @return The status byte. 0x08 means subpage 0, 0x09 means subpage 1 is ready.
 */
uint8_t mlx90640_checkStatus(void) {
    uint8_t error = MLX90640_I2C_NACK_ERROR;
    uint8_t status[2] = {0};
    error = HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, 
        I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);
        if(error != HAL_OK) {
            return error;
        }
    return status[1];
}

// void mlx90640_resetReadyStatus(void) {
//     uint8_t status[2] = {0};
//     HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);
//     status[1] &= 0b11110111; // Clear ready status bit
//     HAL_I2C_Mem_Write(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);
// }


uint8_t mlx90640_getFrameData(void) {
    uint16_t dataBuff[MLX90640_FRAMEDATA_LENGTH] = {0};
    uint8_t error = MLX90640_I2C_NACK_ERROR;
    // uint8_t status[2] = {0};
    // if(mlx90640_checkStatus() == 0x09) {
    //     // mlx90640_resetReadyStatus();
    //     status[1] = 0b00000001;
    //     HAL_I2C_Mem_Write(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);
    //     // HAL_Delay(350);
    // }
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

    // HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_RAM_START, I2C_MEMADD_SIZE_16BIT, mlx90640_data, MLX90640_DATA_LENGTH / 2, MLX90640_DELAY_MS);
    // HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, 
    //     MLX90640_RAM_START + MLX90640_DATA_LENGTH / 2, 
    //     I2C_MEMADD_SIZE_16BIT, 
    //     mlx90640_data + MLX90640_DATA_LENGTH / 2, 
    //     MLX90640_DATA_LENGTH / 2, 
    //     MLX90640_DELAY_MS);
    mlx90640.state = MLX90640_STATE_READING_SUBPAGE0;
    error = HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_RAM_START, 
        I2C_MEMADD_SIZE_16BIT, (uint8_t *)mlx90640.frameData - 1, MLX90640_FRAMEDATA_LENGTH * 2);
    if (error != HAL_OK) {
        return error;
    }
    // status[1] = 0b00000000;
    // HAL_I2C_Mem_Write(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);

    // HAL_Delay(350);
    timeout = 0;
    while(mlx90640_checkStatus() != 0x09 || mlx90640.state != MLX90640_STATE_COMPLETE_SUBPAGE0) {
        timeout++;
        HAL_Delay(1);
        if(timeout > 1000){
            return MLX90640_TIMEOUT_ERROR;
        }
    }
    
    // HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, MLX90640_RAM_START, I2C_MEMADD_SIZE_16BIT, dataBuff, MLX90640_DATA_LENGTH / 2, MLX90640_DELAY_MS);
    // HAL_I2C_Mem_Read(&hi2c1, MLX90640_I2C_ADDR, 
    //     MLX90640_RAM_START + MLX90640_DATA_LENGTH / 2, 
    //     I2C_MEMADD_SIZE_16BIT, 
    //     dataBuff + MLX90640_DATA_LENGTH / 2, 
    //     MLX90640_DATA_LENGTH / 2, 
    //     MLX90640_DELAY_MS);
    mlx90640.state = MLX90640_STATE_READING_SUBPAGE1;
    error = HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_RAM_START, 
        I2C_MEMADD_SIZE_16BIT, (uint8_t *)dataBuff - 1, MLX90640_FRAMEDATA_LENGTH * 2);
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
    // status[1] = 0b00000001;
    // HAL_I2C_Mem_Write(&hi2c1, MLX90640_I2C_ADDR, MLX90640_STATUS_REG, I2C_MEMADD_SIZE_16BIT, status, MLX90640_STATUS_LENGTH, MLX90640_DELAY_MS);
    return MLX90640_NO_ERROR;
}

static void mlx90640_mergeData(uint16_t *dataBuff) {
    for(int i=1; i<MLX90640_FRAMEDATA_LENGTH; i += 2) {
        mlx90640.frameData[i] = dataBuff[i];
        // mlx90640.frameData[i + 1] = dataBuff[i + 1];
    }
    mlx90640.state = MLX90640_STATE_IDLE;
}

void mlx90640_test(void) {
    HAL_I2C_Mem_Read_DMA(&hi2c1, MLX90640_I2C_ADDR, MLX90640_CALI_START, I2C_MEMADD_SIZE_16BIT, mlx90640_caliData, MLX90640_FRAMEDATA_LENGTH);
}
