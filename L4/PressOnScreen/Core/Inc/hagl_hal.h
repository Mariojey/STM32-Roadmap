#pragma once

#include "lcd.h"
#include "bitmap.h"


#define DISPLAY_WIDTH 	(SCREEN_WIDTH)
#define DISPLAY_HEIGHT 	(SCREEN_HEIGHT)
#define DISPLAY_DEPTH 	16
typedef uint16_t color_t;
#define hagl_hal_put_pixel screenPutPixel
