/*
 * lps25hb_lib.h
 *
*/

#pragma once

#include "stm32l4xx.h"

HAL_StatusTypeDef lps25hb_sensor_init(void);

float lps_25hb_get_temperature(void);

float lps25hb_get_pressure(void);
