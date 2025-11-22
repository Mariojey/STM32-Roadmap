/*
 * lps25gb_lib.c
 */

#include "lps25hb_lib.h"

#include "i2c.h"

//Device Address if A0 didn't have low

#define SENSOR_ADDR 0xBA

#define WHOAMI 0x0F
#define CTRL_REG_1 0x20
#define CTRL_REG_2 0x21
#define CTRL_REG_3 0x22
#define CTRL_REG_4 0x23

#define PRESS_OUT_XL 0x28
#define PRESS_OUT_L 0x29
#define PRESS_OUT_H 0x2A

#define TEMP_OUT_L 0x2B
#define TEMP_OUT_h 0x2C

#define MAX_TIME_TO_WAIT 100

static write_to_register(uint8_t reg, uint8_t value){
	HAL_I2C_Mem_Write(&hi2c1, SENSOR_ADDR, reg, 1, &value, sizeof(value), MAX_TIME_TO_WAIT);
}

static read_from_register(uint8_t reg)
{
	uint8_t readed_value = 0;

	HAL_I2C_Mem_Read(&hi2c1, SENSOR_ADDR,reg, 1, &readed_value, sizeof(readed_value), MAX_TIME_TO_WAIT);

	return readed_value;
}

HAL_StatusTypeDef lps25hb_sensor_init(void)
{
	if (read_from_register(WHOAMI) != 0xBD)
	{

		return HAL_ERROR;
	}

	write_to_register(CTRL_REG_1, 0xC0);
	return HAL_OK;
}

float lps_25hb_get_temperature(void)
{
	int16_t readedTemp = 0;

	if(HAL_I2C_Mem_Read(&hi2c1, SENSOR_ADDR, TEMP_OUT_L | 0x80, 1, (uint8_t*)&readedTemp, sizeof(readedTemp), MAX_TIME_TO_WAIT) != HAL_OK)
	{
		Error_Handler();
	}

	return 42.5f + readedTemp / 480.0f;
}

float lps25hb_get_pressure(void)
{
	int32_t readedPressure = 0;

	if(HAL_I2C_Mem_Read(&hi2c1, SENSOR_ADDR, PRESS_OUT_XL | 0x80, 1, (uint8_t*)&readedPressure, 3, MAX_TIME_TO_WAIT) != HAL_OK)
	{
		Error_Handler();
	}

	return readedPressure / 4096.0f;
}



