#ifndef __MLX90640_H
#define __MLX90640_H

// #include <stdint.h>

#include <stdint.h>
#define MLX90640_NO_ERROR              0
#define MLX90640_I2C_NACK_ERROR        1
#define MLX90640_I2C_WRITE_ERROR       2
#define MLX90640_TIMEOUT_ERROR         3
#define MLX90640_BUSY_ERROR            4

#define MLX90640_I2C_ADDR        0x33<<1

#define MLX90640_LINE_NUMBER          24
#define MLX90640_COLUMN_NUMBER        32
#define MLX90640_PIXEL_LENGTH             768
#define MLX90640_FRAMEDATA_LENGTH      832
#define MLX90640_AUXDATA_LENGTH        64
#define MLX90640_EEPROM_DUMP_LENGTH          832
#define MLX90640_STATUS_LENGTH     2
#define MLX90640_DELAY_MS         100
#define SCALEPHA                 0.000001

#define MLX90640_ID_REG          0x2407
#define MLX90640_STATUS_REG      0x8000
#define MLX90640_RAM_START       0x0400
#define MLX90640_AUXDATA_START   0x0700
#define MLX90640_EEPROM_START      0x2400

typedef enum {
    MLX90640_STATE_READING_SUBPAGE0,
    MLX90640_STATE_COMPLETE_SUBPAGE0,
    MLX90640_STATE_READING_SUBPAGE1,
    MLX90640_STATE_COMPLETE_SUBPAGE1,
    MLX90640_STATE_IDLE
} MLX90640_StateTypeDef;

typedef struct{
    uint8_t ID[3];
    int16_t frameData[MLX90640_FRAMEDATA_LENGTH];
    uint16_t mlx90640_EEPROM_Data[MLX90640_EEPROM_DUMP_LENGTH];
    MLX90640_StateTypeDef state;
    int16_t kVdd;
    int16_t vdd25;
    float KvPTAT;
    float KtPTAT;
    uint16_t vPTAT25;
    float alphaPTAT;
    int16_t gain;
    float tgc;
    float cpKv;
    float cpKta;
    uint8_t resolution;
    float ksTa;
    float ksTo[5];
    int16_t ct[5];
    uint16_t alpha[768];
    uint8_t alphaScale;
    int16_t offset[768];
    int8_t kta[768];
    uint8_t ktaScale;
    int8_t kv[768];
    uint8_t kvScale;
    float cpAlpha[2];
    int16_t cpOffset[2];
    uint8_t ilChessC[2];

} MLX90640_HandleTypeDef;

extern MLX90640_HandleTypeDef mlx90640;

uint8_t mlx90640_getID(void);
int MLX90640_ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
void MLX90640_calculateTo(uint16_t *frameData, const MLX90640_HandleTypeDef *mlx90640, float emissivity, float tr, float *result);
void mlx90640_test(void);

#endif /* __MLX90640_H */
