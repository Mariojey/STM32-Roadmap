/*
 * display.c
 *
 *  Created on: Dec 1, 2025
 *  Based on: https://forbot.pl/blog/kurs-stm32l4-kolorowy-wyswietlacz-tft-spi-id48620
 */

#include "display.h"
#include "spi.h"


#define DISPLAY_SLPOUT			0x11
#define DISPLAY_DISPOFF			0x28
#define DISPLAY_DISPON			0x29
#define DISPLAY_CASET			0x2a
#define DISPLAY_RASET			0x2b
#define DISPLAY_RAMWR			0x2c
#define DISPLAY_MADCTL			0x36
#define DISPLAY_COLMOD			0x3a
#define DISPLAY_FRMCTR1			0xb1
#define DISPLAY_FRMCTR2			0xb2
#define DISPLAY_FRMCTR3			0xb3
#define DISPLAY_INVCTR			0xb4
#define DISPLAY_PWCTR1			0xc0
#define DISPLAY_PWCTR2			0xc1
#define DISPLAY_PWCTR3			0xc2
#define DISPLAY_PWCTR4			0xc3
#define DISPLAY_PWCTR5			0xc4
#define DISPLAY_VMCTR1			0xc5
#define DISPLAY_GAMCTRP1		0xe0
#define DISPLAY_GAMCTRN1		0xe1

#define COMMAND(x)				((x) | 0x100)

static void displaySendCommand(uint8_t command)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &command, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void displaySendData(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void displaySendFrame(uint8_t data)
{
	if(data & 0x100)
	{
		displaySendCommand(data);
	}else{
		displaySendData(data);
	}
}

static const uint16_t initConfig[] = {
		COMMAND(DISPLAY_FRMCTR1), 0x01, 0x2c, 0x2d,
		COMMAND(DISPLAY_FRMCTR2), 0x01, 0x2c, 0x2d,
		COMMAND(DISPLAY_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
		COMMAND(DISPLAY_INVCTR), 0x07,
		COMMAND(DISPLAY_PWCTR1), 0xa2, 0x02, 0x84,
		COMMAND(DISPLAY_PWCTR2), 0xc5,
		COMMAND(DISPLAY_PWCTR3), 0x0a, 0x00,
		COMMAND(DISPLAY_PWCTR4), 0x8a, 0x2a,
		COMMAND(DISPLAY_PWCTR5), 0x8a, 0xee,
		COMMAND(DISPLAY_VMCTR1), 0x0e,
		COMMAND(DISPLAY_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
						 COMMAND(DISPLAY_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
						 COMMAND(0xf0), 0x01,
						 COMMAND(0xf6), 0x00,
						 COMMAND(DISPLAY_COLMOD), 0x05,
						 COMMAND(DISPLAY_MADCTL), 0xa0,
};


void displayInit()
{
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);



	for(int initIterator = 0; initIterator < sizeof(initConfig) / sizeof(uint16_t); initIterator++)
	{
		displaySendFrame(initConfig[initIterator]);
	}

	HAL_Delay(250);

	displaySendCommand(DISPLAY_SLPOUT);
	HAL_Delay(250);
	displaySendCommand(DISPLAY_DISPON);
}

static void displaySendData16(uint16_t data)
{
	displaySendData(data >> 8);
	displaySendData(data);
}

static void displayConfigureWindow(int x, int y, int width, int height)
{
	displaySendCommand(DISPLAY_CASET);
	displaySendData16(x);
	displaySendData16(x + width - 1);

	displaySendCommand(DISPLAY_RASET);
	displaySendData16(y);
	displaySendData16(y + height - 1);
}
