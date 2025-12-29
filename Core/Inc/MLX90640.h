#ifndef __MLX90640_H
#define __MLX90640_H

#include <stdint.h>
#define MLX90640_NO_ERROR              0
#define MLX90640_I2C_NACK_ERROR        1
#define MLX90640_I2C_WRITE_ERROR       2
#define MLX90640_TIMEOUT_ERROR         3
#define MLX90640_BUSY_ERROR            4

#define MLX90640_I2C_ADDR        0x33<<1

#define MLX90640_FRAMEDATA_LENGTH      768
#define MLX90640_STATUS_LENGTH    2
#define MLX90640_DELAY_MS         100

#define MLX90640_ID_REG          0x2407
#define MLX90640_STATUS_REG      0x8000
#define MLX90640_RAM_START       0x0400
#define MLX90640_CALI_START      0x2440

typedef enum {
    MLX90640_STATE_READING_SUBPAGE0,
    MLX90640_STATE_COMPLETE_SUBPAGE0,
    MLX90640_STATE_READING_SUBPAGE1,
    MLX90640_STATE_COMPLETE_SUBPAGE1,
    MLX90640_STATE_IDLE
} MLX90640_StateTypeDef;
typedef struct{
    uint8_t ID[3];
    uint16_t frameData[MLX90640_FRAMEDATA_LENGTH];
    MLX90640_StateTypeDef state;
} MLX90640_HandleTypeDef;

extern MLX90640_HandleTypeDef mlx90640;

uint8_t mlx90640_getID(void);
uint8_t mlx90640_checkStatus(void);
// void mlx90640_resetReadyStatus(void);
uint8_t mlx90640_getFrameData(void);
void mlx90640_test(void);

#endif /* __MLX90640_H */
