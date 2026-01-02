#include "MLX90640_I2C_Driver.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_i2c.h"
#include <stdint.h>

// uint8_t DMA_OK = 0;

// void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
//     if(hi2c->Instance == hi2c1.Instance) {
//         DMA_OK = 1;
//     }
// }

// int MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
//     uint16_t timeout = 0;
//     HAL_I2C_Mem_Read_DMA(&hi2c1, slaveAddr << 1, startAddress - 1, I2C_MEMADD_SIZE_16BIT, (uint8_t *)data, nMemAddressRead * 2);

//     while(DMA_OK == 0) {
//         HAL_Delay(1);
//         timeout++;
//         if(timeout > 1000) {
//             return -1;
//         }
//     }

//     DMA_OK = 0;

//     return DMA_OK;
// }

// int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data) {
//     int ack = 0;
//     uint8_t cmd[2];
//     static uint16_t dataCheck;
//     cmd[0] = data >> 8;
//     cmd[1] = data & 0xFF;
//     ack = HAL_I2C_Mem_Write(&hi2c1, slaveAddr << 1, writeAddress, I2C_MEMADD_SIZE_16BIT, cmd , sizeof(cmd), 500);

//     if(ack != HAL_OK) {
//         return -1;
//     }

//     MLX90640_I2CRead(slaveAddr, writeAddress, 1, &dataCheck);

//     if(dataCheck != data) {
//         return -2;
//     }
    
//     return 0;

// }

int MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data) {
    uint8_t ack = 0;
    uint16_t cnt = 0;
    uint8_t* bp = (uint8_t*) data;

    if(nMemAddressRead > 300) {

        ack = HAL_I2C_Mem_Read(&hi2c1, slaveAddr << 1, startAddress, I2C_MEMADD_SIZE_16BIT, bp, nMemAddressRead, 500);

        if(ack != HAL_OK) {
            return -1;
        }

        ack = HAL_I2C_Mem_Read(&hi2c1, slaveAddr << 1, startAddress + nMemAddressRead, 
            I2C_MEMADD_SIZE_16BIT, bp + nMemAddressRead, nMemAddressRead, 500);

        if(ack != HAL_OK) {
            return -1;
        }

        for(cnt=0; cnt < nMemAddressRead*2; cnt+=2) {
            uint8_t tmpbytelsb = bp[cnt+1];
            bp[cnt+1] = bp[cnt];
            bp[cnt] = tmpbytelsb;
        }

    }

    else {

        ack = 0;                               
        int cnt = 0;
        
        ack = HAL_I2C_Mem_Read(&hi2c1, (slaveAddr<<1), startAddress, I2C_MEMADD_SIZE_16BIT, bp, nMemAddressRead*2, 500);

        if (ack != HAL_OK)
        {
            return -1;
        }
        
        for(cnt=0; cnt < nMemAddressRead*2; cnt+=2) {
            uint8_t tmpbytelsb = bp[cnt+1];
            bp[cnt+1] = bp[cnt];
            bp[cnt] = tmpbytelsb;
        }
    }

    return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data) {
    uint8_t ack = 0;
    uint8_t cmd[2] = {0};
    static uint16_t dataCheck;

    cmd[0] = data >> 8;
    cmd[1] = data & 0xFF;

    ack = HAL_I2C_Mem_Write(&hi2c1, slaveAddr << 1, writeAddress, I2C_MEMADD_SIZE_16BIT, cmd, sizeof(cmd), 500);
    
    if(ack != HAL_OK) {
        return -1;
    }

    ack = MLX90640_I2CRead(slaveAddr, writeAddress, 1, &dataCheck);

    if(ack != HAL_OK || dataCheck != data) {
        return -2;
    }

    return 0;
}