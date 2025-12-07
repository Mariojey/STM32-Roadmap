#include "lcd.h"
#include "spi.h"

//From documentation
#define SCREEN_SLPOUT			0x11
#define SCREEN_DISPOFF			0x28
#define SCREEN_DISPON			0x29
#define SCREEN_CASET			0x2a
#define SCREEN_RASET			0x2b
#define SCREEN_RAMWR			0x2c
#define SCREEN_MADCTL			0x36
#define SCREEN_COLMOD			0x3a
#define SCREEN_FRMCTR1			0xb1
#define SCREEN_FRMCTR2			0xb2
#define SCREEN_FRMCTR3			0xb3
#define SCREEN_INVCTR			0xb4
#define SCREEN_PWCTR1			0xc0
#define SCREEN_PWCTR2			0xc1
#define SCREEN_PWCTR3			0xc2
#define SCREEN_PWCTR4			0xc3
#define SCREEN_PWCTR5			0xc4
#define SCREEN_VMCTR1			0xc5
#define SCREEN_GAMCTRP1		    0xe0
#define SCREEN_GAMCTRN1			0xe1

#define SCREEN_OFFSET_X  1
#define SCREEN_OFFSET_Y  2

static void screenDefineMessage(uint8_t message)
{

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);

	//Ground
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi2, &message, 1, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void screenDefineCommand(uint8_t command)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);

	//Ground
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi2, &command, 1, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

#define COMMAND(givenLine)			((givenLine) | 0x100) //From https://forbot.pl/blog/kurs-stm32l4-kolorowy-wyswietlacz-tft-spi-id48620

static void screenSend(uint16_t value)
{
	if (value & 0x100) {
		screenDefineCommand(value);
	} else {
		screenDefineMessage(value);
	}
}

static void screenDefine16BitsMessage(uint16_t value)
{
	screenDefineMessage(value >> 8);

	screenDefineMessage(value);
}

static void screenDefineWindow(int x, int y, int width, int height)
{
  screenDefineCommand(SCREEN_CASET);

  screenDefine16BitsMessage(SCREEN_OFFSET_X + x);

  screenDefine16BitsMessage(SCREEN_OFFSET_X + x + width - 1);

  screenDefineCommand(SCREEN_RASET);

  screenDefine16BitsMessage(SCREEN_OFFSET_Y + y);
  screenDefine16BitsMessage(SCREEN_OFFSET_Y + y + height- 1);
}

static const uint16_t splashScreen[] = {
		COMMAND(SCREEN_FRMCTR1), 0x01, 0x2c, 0x2d,
		COMMAND(SCREEN_FRMCTR2), 0x01, 0x2c, 0x2d,
		COMMAND(SCREEN_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
		COMMAND(SCREEN_INVCTR), 0x07,
		COMMAND(SCREEN_PWCTR1), 0xa2, 0x02, 0x84,
		COMMAND(SCREEN_PWCTR2), 0xc5,
		COMMAND(SCREEN_PWCTR3), 0x0a, 0x00,
		COMMAND(SCREEN_PWCTR4), 0x8a, 0x2a,
		COMMAND(SCREEN_PWCTR5), 0x8a, 0xee,
  COMMAND(SCREEN_VMCTR1), 0x0e,
  COMMAND(SCREEN_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
						 COMMAND(SCREEN_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
						 COMMAND(0xf0), 0x01,
						 COMMAND(0xf6), 0x00,
						 COMMAND(SCREEN_COLMOD), 0x05,
						 COMMAND(SCREEN_MADCTL), 0xa0,
};

static uint16_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];


void screenInit(void)
{
  int itr;


  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);

  HAL_Delay(100);

  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);

  HAL_Delay(100);

  for (itr = 0; itr < sizeof(splashScreen) / sizeof(uint16_t);itr++) {
	  screenSend(splashScreen[itr]);
  }

  HAL_Delay(200);

  screenDefineCommand(SCREEN_SLPOUT);

  HAL_Delay(120);

  screenDefineCommand(SCREEN_DISPON);

}

void screenFillBox(int x, int y, int width, int height, uint16_t color)
{
	screenDefineWindow(x, y, width, height);

	screenDefineMessage(SCREEN_RAMWR);

	for (int i = 0; i < width * height; i++){

		screenDefine16BitsMessage(color);


	}
}

void screenPutPixel(int x, int y, uint16_t color)
{
	buffer[x + y * SCREEN_WIDTH] = color;
}



void screenShowImage(int x, int y, int width, int height, const uint8_t* data)
{
	screenDefineWindow(x, y, width, height);

	screenDefineCommand(SCREEN_RAMWR);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi2, (uint8_t*)data, width * height * 2, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void screenCopy(void)
{
	screenDefineWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	screenDefineCommand(SCREEN_RAMWR);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi2, (uint8_t*)buffer, sizeof(buffer), HAL_MAX_DELAY);

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}



