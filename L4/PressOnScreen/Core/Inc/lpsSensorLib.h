/*
 * lps25hb_lib.h
 *
*/

#pragma once

#include "stm32l4xx.h"

HAL_StatusTypeDef lpsSensorInit(void);

float lpsSensorGetTemp(void);

float lpsSensorGetPress(void);
