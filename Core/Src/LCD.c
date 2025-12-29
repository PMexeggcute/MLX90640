#include "main.h"
#include "LCD_Font.h"   // F8x16 字库
#include "stm32f401xc.h"    // STM32F401xC 头文件

/* 引脚配置 */
//寄存器快速操作
#define LCD_SCK(x)   LCD_SCL_GPIO_Port->BSRR = LCD_SCL_Pin << (x ? 0 : 16)
#define LCD_SDA(x)   LCD_SDA_GPIO_Port->BSRR = LCD_SDA_Pin << (x ? 0 : 16)
#define LCD_RES(x)   LCD_RES_GPIO_Port->BSRR = LCD_RES_Pin << (x ? 0 : 16)
#define LCD_DC(x)    LCD_DC_GPIO_Port->BSRR = LCD_DC_Pin << (x ? 0 : 16)
#define LCD_CS(x)    LCD_CS_GPIO_Port->BSRR = LCD_CS_Pin << (x ? 0 : 16)

void LCD_WriteByte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        LCD_SDA((byte & (0x80 >> i)) != 0);
        LCD_SCK(1);
        LCD_SCK(0);
    }
}

void LCD_WriteCommand(uint8_t cmd)
{
    LCD_CS(0);
    LCD_DC(0);
    LCD_WriteByte(cmd);
    LCD_CS(1);
}

void LCD_WriteData(uint8_t data)
{
    LCD_CS(0);
    LCD_DC(1);
    LCD_WriteByte(data);
    LCD_CS(1);
}

void LCD_Reset(void)
{
    LCD_RES(0);
    for (volatile int i = 0; i < 50000; i++);
    LCD_RES(1);
    for (volatile int i = 0; i < 50000; i++);
}

void LCD_Init(void)
{
    LCD_Reset();

    LCD_WriteCommand(0x01); // Software reset
    for (volatile int i = 0; i < 50000; i++);

    LCD_WriteCommand(0x11); // Sleep out
    for (volatile int i = 0; i < 50000; i++);

    LCD_WriteCommand(0x3A); // Interface Pixel Format
    LCD_WriteData(0x05);    // 16-bit color

    LCD_WriteCommand(0x36); // MADCTL
    LCD_WriteData(0x00);    // normal orientation

    LCD_WriteCommand(0x29); // Display ON
}

void LCD_SetAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    x0 += 2; // 根据实际连接调整偏移
    x1 += 2;
    y0 += 1;
    y1 += 1;

    LCD_WriteCommand(0x2A); // Column
    LCD_WriteData(0);
    LCD_WriteData(x0);
    LCD_WriteData(0);
    LCD_WriteData(x1);

    LCD_WriteCommand(0x2B); // Row
    LCD_WriteData(0);
    LCD_WriteData(y0);
    LCD_WriteData(0);
    LCD_WriteData(y1);

    LCD_WriteCommand(0x2C); // Memory write
}

void LCD_FillScreen(uint16_t color)
{
    LCD_SetAddrWindow(0, 0, 127, 159);
    for (uint16_t i = 0; i < 160*128; i++)
    {
        LCD_WriteData(color >> 8);
        LCD_WriteData(color & 0xFF);
    }
}

//void LCD_DrawChar(uint8_t x, uint8_t y, char ch)
//{
//    for (uint8_t i = 0; i < 16; i++)
//    {
//        uint8_t line = OLED_F8x16[ch - ' '][i];
//        for (uint8_t j = 0; j < 8; j++)
//        {
//            if (line & (0x80 >> j))
//            {
//                LCD_SetAddrWindow(x + j, y + i, x + j, y + i);
//                LCD_WriteData(0xFF);
//                LCD_WriteData(0xFF); // 白色
//            }
//        }
//    }
//}

void LCD_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bgcolor)
{
	y = y * 8 + 1;
	x = 112 - 16 * x;		//针对设备修正
	
    for (uint8_t i = 0; i < 8; i++)  // 每行
    {
        uint8_t line = LCD_F8x16[ch - ' '][i];
        LCD_SetAddrWindow(x + 8, y + i, x + 7 + 8, y + i); // 一次写整行
        for (uint8_t j = 0; j < 8; j++)
        {
            if (line & (0x80 >> j))
            {
                LCD_WriteData(color >> 8); // 高字节颜色
                LCD_WriteData(color & 0xFF); // 低字节颜色
            }
            else
            {
                LCD_WriteData(bgcolor >> 8); // 背景
                LCD_WriteData(bgcolor & 0xFF);
            }
        }
    }
	
	for (uint8_t i = 8; i >= 8 && i < 16; i++)  // 每行
    {
        uint8_t line = LCD_F8x16[ch - ' '][i];
        LCD_SetAddrWindow(x - 8 + 8, y + i - 8, x - 1 + 8, y + i - 8); // 一次写整行
        for (uint8_t j = 0; j < 8; j++)
        {
            if (line & (0x80 >> j))
            {
                LCD_WriteData(color >> 8); // 高字节白色
                LCD_WriteData(color & 0xFF); // 低字节白色
            }
            else
            {
                LCD_WriteData(bgcolor >> 8); // 黑色背景
                LCD_WriteData(bgcolor & 0xFF);
            }
        }
    }
}

void LCD_ShowString(uint8_t x, uint8_t y, char* str, uint16_t color, uint16_t bgcolor)
{
    while (*str)
    {
        LCD_ShowChar(x, y, *str++, color, bgcolor);
        y += 1;
        if (y > 159) break;
    }
}


uint32_t LCD_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

void LCD_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		LCD_ShowChar(Line, Column + i, Number / LCD_Pow(10, Length - i - 1) % 10 + '0', color, bgcolor);
	}
}

void LCD_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		LCD_ShowChar(Line, Column, '+', color, bgcolor);
		Number1 = Number;
	}
	else
	{
		LCD_ShowChar(Line, Column, '-', color, bgcolor);
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		LCD_ShowChar(Line, Column + i, Number1 / LCD_Pow(10, Length - i - 1) % 10 + '0', color, bgcolor);
	}
}

/**
  * @brief  LCD显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */

void LCD_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / LCD_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			LCD_ShowChar(Line, Column + i, SingleNumber + '0', color, bgcolor);
		}
		else
		{
			LCD_ShowChar(Line, Column + i, SingleNumber - 10 + 'A', color, bgcolor);
		}
	}
}

/**
  * @brief  LCD显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void LCD_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length, uint16_t color, uint16_t bgcolor)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		LCD_ShowChar(Line, Column + i, Number / LCD_Pow(2, Length - i - 1) % 2 + '0', color, bgcolor);
	}
}

