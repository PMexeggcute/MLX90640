#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>

#define Color_Black       0x0000
#define Color_White       0xFFFF
/* GPIO 初始化 */
// void LCD_GPIO_Init(void);

// void LCD_WriteByte(uint8_t byte);

// void LCD_WriteCommand(uint8_t cmd);

// void LCD_WriteData(uint8_t data);

// void LCD_Reset(void);

void LCD_Init(void);

void LCD_SetAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

void LCD_FillScreen(uint16_t color);

void LCD_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bgcolor);

void LCD_ShowString(uint8_t x, uint8_t y, char* str, uint16_t color, uint16_t bgcolor);

// void LCD_Test(void);

uint32_t LCD_Pow(uint32_t X, uint32_t Y);

void LCD_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor);

void LCD_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor);

void LCD_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor);

void LCD_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor);

void LCD_DrawPoint(uint8_t x, uint8_t y, uint16_t color);

void LCD_ShowCharTrpbg(uint8_t x, uint8_t y, char ch, uint16_t color);

void LCD_ShowSignedNumTrpbg(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length, uint16_t color);

void LCD_ShowStringTrpbg(uint8_t x, uint8_t y, char* str, uint16_t color);

#endif
