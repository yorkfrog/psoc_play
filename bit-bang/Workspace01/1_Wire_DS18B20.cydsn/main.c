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
#include "project.h"
#include "stdio.h"

#include <ds18b20_driver.h>



int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
   
    uint8 scratchpad[9];
    uint16 tempr;
    float temp_cel;
    uint8 connect_status = 0;
    char msg[40];
    
    UART_Start();
    
    connect_status = Initialize_1_wire();
    
    if(connect_status==0)
    {
    	// device detected
    	RED_Write(1);
        GREEN_Write(0);
    }
    else
    {
    	// NO device detected
        RED_Write(0);
        GREEN_Write(1);        
    }
    CyDelay(SETUP_DLY);

    sprintf(msg,"Temp sense Started. status:%d", connect_status);
    UART_PutString(msg);
    UART_PutCRLF();

    Config_ds18d20();

/*  Each loop contains one one reset operation followed by temperature conversion start command
    
    Once the conversion is complete the Scratch pad from the sensor is copied from the sensor.
    
    First 16 bits of the sensor is the temperature value which is converted into float format and displayed via UART
*/   
    for(;;)
    {
    	UART_PutString(".");
        connect_status= Initialize_1_wire();
        if(connect_status==0)
        {
            RED_Write(0);
            GREEN_Write(1);
        }
        else
        {
            RED_Write(1);
            GREEN_Write(0);        
        }
        CyDelay(SETUP_DLY);

    	UART_PutString("-");
		Request_scratchpad_read();

        /* Wait until conversion completes */
        /* 
         * This assumes that the DS18B20 is using external power.
         * See https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf - p11: Convert T [44h] section.
         */
        while(Read_bit_1_wire()==0);
        
    	UART_PutString(".");
        connect_status= Initialize_1_wire();
        if(connect_status==0)
        {
            RED_Write(0);
            GREEN_Write(1);
        }
        else
        {
            RED_Write(1);
            GREEN_Write(0);        
        }
        CyDelay(SETUP_DLY);

    	UART_PutString("+");

        // Complete the 1Wire temp read from the DS18 scratchpad
		Do_scratchpad_read(scratchpad);

		// check CRC for scratchpad
		if (check_crc(scratchpad)) {
			// Combine the result to 16 bit register
			// [0] LSB, [1] MSB
			tempr=(scratchpad[1]<<8)+scratchpad[0];

			temp_cel= convert_2_celsius(tempr);

			sprintf(msg,"TEMPERATURE = %.10f C",temp_cel);
			UART_PutString(msg);
			UART_PutCRLF();
		} else {
			sprintf(msg,"CRC Check failed [CRC:%d]", scratchpad[8]);
			UART_PutString(msg);
			UART_PutCRLF();
		}
    }
}

/* [] END OF FILE */
