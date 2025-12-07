#pragma once

#include <stdint.h>


#define BLACK			0x0000
#define RED				0x00f8
#define GREEN			0xe007
#define BLUE			0x1f00
#define YELLOW			0xe0ff
#define MAGENTA			0x1ff8
#define CYAN			0xff07
#define WHITE			0xffff

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128

void screenInit(void);

void screenPutPixel(int x, int y, uint16_t color);

void screenCopy(void);

void screenFillBox(int x, int y, int width, int height, uint16_t color);

void screenShowImage(int x, int y, int width, int height, const uint8_t* data);
