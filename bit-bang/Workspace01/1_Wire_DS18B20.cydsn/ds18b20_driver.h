/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#define SENSOR_DS18B20_ENABLED
#ifndef __DS18B20_driver_h__

#include "project.h"
//#include <common.h>

/*
#include <bm_280_common.h>
#include <bme280_defs.h>
#include <sensor_config.h>
*/

#ifdef SENSOR_DS18B20_ENABLED

	#define RST_TIME 480
	#define HALF_TIME_SLOT 30
	#define TIME_SLOT 60
	#define RCVRY_TIME 20
	#define WRITE_0_TIME 60
	#define WRITE_1_TIME 10
	#define READ_INIT_DLY 5
	#define READ_VALID_DLY 5

	#define SKIP_ADDR_CMND 0xCC
	#define WRITE_SC_PAD   0x4E
	#define RESOL_12BIT 0x7F
	#define START_CONVERT 0x44
	#define READ_SC_PAD 0xBE
	#define SETUP_DLY 1000

	uint8 Initialize_1_wire();
	uint8 Read_bit_1_wire();
	void Config_ds18d20();
	void Do_scratchpad_read(uint8 scratchpad[9]);
	void Request_scratchpad_read();
	float convert_2_celsius(uint16_t temp);
	float convert_2_fahrenheit(uint16_t temp) ;

#endif

#define __DS18B20_driver_h__
#endif  //__DS18B20_driver_h__

/* [] END OF FILE */
