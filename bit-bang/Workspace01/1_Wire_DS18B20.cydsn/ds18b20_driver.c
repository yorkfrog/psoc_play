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

/*******************************************************************************
 * Module DS18B20 temp sensor
 ****************************************************************************//**
 *
 * \Uses 1-wire communications to interact with temp sensor.
 * \Assumptions
 * - DS18B20 is powered (not using parasitic power)
 * - there is only 1 device (DS18B20) on the 1-wire bus
 * - 12bit temp resolution - changing this changes the temp read timings.
 *
 * \Reference
 * datasheet: https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 *
 *********************************************************************************************/


#include "stdio.h"
#include <ds18b20_driver.h>

#ifdef SENSOR_DS18B20_ENABLED


/*******************************************************************************
 * Function Name: Initialise_1_wire
 ****************************************************************************//**
 *
 * \Initialises the 1 wire communication, Master pulls the channel low for 480us at least.
 * For which the slaves will reply with a 0, within one time slot.
 *
 * <b>Note</b> Done initially to find the presence of devices in the network
 * See https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf : P15 sec. 1-Wire Signalling
 * \return
 *  returns the status
 *  0 - success (presence pulse detected)
 *  1 - failure ( no device responded)
 *
 *********************************************************************************************/
uint8 Initialize_1_wire() {
	uint8 status;
	Wire1_Write(0);

	/*  Minimum reset time for 1-Wire Protocol  */
	CyDelayUs(RST_TIME);
	Wire1_Write(1);
	/* Slave should respond within one time slot(60us) */
	CyDelayUs(HALF_TIME_SLOT);
	status = Wire1_Read();
	return status;
}

/*******************************************************************************
 * Function Name: Write_bit_1_wire
 ****************************************************************************//**
 *
 * \Master Writes one bit of data to the slave
 *
 * <b>Note</b> All master writes are done based on this function
 *
 *
 * \param value
 * 1 bit write data
 *
 * \return
 *  void function
 *
 *********************************************************************************************/
void Write_bit_1_wire(uint8 value) {
	/* Minimum recovery time 1us between write steps in necessary, 20us is used */
	CyDelayUs(RCVRY_TIME);
	/* Write is initiated from this step*/
	Wire1_Write(0);

	if (value == 0) {
		/* Minimum 60us for logic0 */
		CyDelayUs(WRITE_0_TIME);
	} else {
		/* Release to 1 before 15us for logic1 */
		CyDelayUs(WRITE_1_TIME);
	}
	Wire1_Write(1);
}

/*******************************************************************************
 * Function Name: Read_bit_1_wire
 ****************************************************************************//**
 *
 * \Master Reads one bit of data from slave
 *
 * <b>Note</b> All master read actions are done based on this function. Master initiates read by pulling the line low.
 * Data should be read before 15us
 *
 * \param
 * no parameter
 *
 * \return
 *  1 bit data from slave
 *
 *********************************************************************************************/
uint8 Read_bit_1_wire() {
	/* Minimum recovery time 1us */
	CyDelayUs(RCVRY_TIME);
	/* Initiate read by pulling line low */
	Wire1_Write(0);
	/* Minimum time 1us, to initiate delay */
	CyDelayUs(READ_INIT_DLY);
	Wire1_Write(1);
	/* Data valid before 15us, should be sampled before that*/
	CyDelayUs(READ_VALID_DLY);
	return (Wire1_Read());
}

/*******************************************************************************
 * Function Name: Write8_1_wire
 ****************************************************************************//**
 *
 * \Master Writes one byte of data to slave
 *
 * <b>Note</b> Write 1 byte of data using bit write function
 *
 * \param payload
 * Write data
 *
 * \return
 *  void function
 *
 *********************************************************************************************/
void Write8_1_wire(uint8 payload) {
	uint8 BitPayload, shiftcount;
	for (shiftcount = 0; shiftcount <= 7; shiftcount++) {
		BitPayload = (payload >> shiftcount) & 0x01;
		Write_bit_1_wire(BitPayload);
	}
}

/*******************************************************************************
 * Function Name: Read8_1_wire
 ****************************************************************************//**
 *
 * \Master Reads one byte data from slave
 *
 * <b>Note</b> Reads 1 byte of data using bit read function
 *
 * \param mode
 * no parameter
 *
 * \return
 *  1 byte read data
 *
 *********************************************************************************************/
uint8 Read8_1_wire() //Read 8 bits "clocked" out by the slave.
{
	uint8 IncomingByte = 0, shiftcount = 0;
	for (shiftcount = 0; shiftcount <= 7; shiftcount++) {
		IncomingByte |= (Read_bit_1_wire()) << shiftcount;
	}
	return (IncomingByte);
}


/*******************************************************************************
 * Function Name: Config_ds18d20
 ****************************************************************************//**
 *
 * \Set up the DS18D20
 *
 * <b>Note</b>
 * - Skip channel as only one sensor
 * - writes scratchpad
 * - ignores Alarm registers
 * - set resolution to 12bit
 *
 * \param mode
 * no parameter
 *
 * \return
 * void function
 *
 *********************************************************************************************/
void Config_ds18d20() {
    /* As only single sensor is present in channel-Skip command */
     Write8_1_wire(SKIP_ADDR_CMND);
    /*Write Scratch Pad command*/
     Write8_1_wire(WRITE_SC_PAD);

    /* Alarm registers are unused, Writing Random values*/
    /*TH*/
    Write8_1_wire(0x55);
    /*TL*/
    Write8_1_wire(0xA2);

    // Config temp read resolution - 9 bit resolution
    Write8_1_wire(RESOL_9BIT);
}

void Request_scratchpad_read() {
	/*Skip address check command */
	Write8_1_wire(SKIP_ADDR_CMND);
	/* Start Convert*/
	Write8_1_wire(START_CONVERT);
}

void Do_scratchpad_read(uint8 scratchpad[9]) {
	/*Skip address check command */
	Write8_1_wire(SKIP_ADDR_CMND);
	/*Read Scratch Pad command*/
	Write8_1_wire(READ_SC_PAD);
	/*Copy the scratpad from slave*/
	for (int i = 0; i < 9; i++) {
		scratchpad[i] = Read8_1_wire();
	}
}



/*******************************************************************************
 * Function Name:  measure_celsius
 ****************************************************************************//**
 *
 * \Calculates the DS18B20 temp reading CRC
 *
 * <b>Note</b>
 * Calculates CRC based upon Maxim's 8 bit polynimia X^8 + X^5 + X^4 + 1.
 * Data is read starting at the least significant bit.
 * from: thepiandi.blogspot.com MJL  071014
 *
 * \param inByte[]
 * array of 8 bytes read from the sensor
 * [0] temp LSB
 * [1] temp MSB
 * [2..7] remaining data bytes
 *
 * \param num_bytes
 * The number of bytes to use to calculate the CRC sum.
 *
 * \return
 * The CRC sum for the input data
 *
 *********************************************************************************************/
uint8_t calculateCRC(const uint8_t inByte[]) {
	uint16_t crc = 0;
	uint8_t current_byte;

	for (int i = 0; i < 8; i++) {
		current_byte = inByte[i];  // make non-destructive
		for (int j = 0; j < 8; j++) {
			if ((crc % 2) ^ (current_byte % 2)) {
				crc ^= 0b100011000;
			} else {
				crc ^= 0b000000000;
			}
			crc >>= 1;
			current_byte >>= 1;
		}
	}
	return (uint8_t)crc;
}

/*******************************************************************************
 * Function Name: check_crc
 ****************************************************************************//**
 *
 * \check the scratchpad CRC against the Calculated CRC
 *
 * \param
 * scratchpad[9]: the 9 bytes of the scratch pad.  Byte[8] is the read CRC.
 *
 * \return
 * uint8_t 1 if the CRC is ok, 0 if the CRC check fails
 *
 *********************************************************************************************/
uint8_t check_crc(const uint8_t scratchpad[9]) {
	// check CRC for scratchpad
	uint16_t is_crc_ok = 0;
	uint16_t scratchpad_crc = calculateCRC(scratchpad);

	char xmsg[40];
	sprintf(xmsg, "CRC: CRC:%d CalcCRC:%d", scratchpad[8], scratchpad_crc);
	UART_PutString(xmsg);
	UART_PutCRLF();

	if (scratchpad[8] == scratchpad_crc) {
		is_crc_ok = 1;
	} else {
//#ifdef DEBUG
		char msg[40];
		sprintf(msg, "CRC FAILED: CRC:%d CalcCRC:%d", scratchpad[8], scratchpad_crc);
		UART_PutString(msg);
		UART_PutCRLF();
//#endif // DEBUG
	}
	return is_crc_ok;
}

/*******************************************************************************
 * Function Name:  convert_2_celsius
 ****************************************************************************//**
 *
 * \Converts the 12 bit sensor data to celcius
 *
 * <b>Note</b>
 * Converts 2 byte sensor scratch pad data into accurate float data.
 * Handles negative temperature conversions.
 *
 * \param mode
 * 2 byte sensor data
 *
 * \return
 *  temperature value in celsius float format
 *
 *********************************************************************************************/
float convert_2_celsius(uint16_t temp) {

	float temp_c = 0;

	if (temp >= 0x800) { //temperature is negative
		//calculate the fractional part
		if (temp & 0x0001) {
			temp_c += 0.06250;
		}
		if (temp & 0x0002) {
			temp_c += 0.12500;
		}
		if (temp & 0x0004) {
			temp_c += 0.25000;
		}
		if (temp & 0x0008) {
			temp_c += 0.50000;
		}

		//calculate the whole number part
		temp = (temp >> 4) & 0x00FF;
		temp = temp - 0x0001; //subtract 1
		temp = ~temp; //ones compliment
		temp_c = temp_c - (float) (temp & 0xFF);
	} else { //temperature is positive
		temp_c = 0;
		//calculate the whole number part
		temp_c = (temp >> 4) & 0x00FF;
		//calculate the fractional part
		if (temp & 0x0001) {
			temp_c = temp_c + 0.06250;
		}
		if (temp & 0x0002) {
			temp_c = temp_c + 0.12500;
		}
		if (temp & 0x0004) {
			temp_c = temp_c + 0.25000;
		}
		if (temp & 0x0008) {
			temp_c = temp_c + 0.50000;
		}
	}
	return temp_c;

}

/*******************************************************************************
 * Function Name:  convert_2_fahrenheit
 ****************************************************************************//**
 *
 * \Converts the 12 bit sensor data to Fahrenheit
 *
 * <b>Note</b>
 * Converts 2 byte sensor scratch pad data into accurate float data.
 * Handles negative temperature conversions.
 *
 * \param mode
 * 2 byte sensor data
 *
 * \return
 *  temperature value in Fahrenheit float format
 *
 *********************************************************************************************/
float convert_2_fahrenheit(uint16_t temp) {
	float temp_celsius = convert_2_celsius(temp);
	return (temp_celsius * 1.8 + 32.0);
}

#endif  // SENSOR_DS18B20_ENABLED
