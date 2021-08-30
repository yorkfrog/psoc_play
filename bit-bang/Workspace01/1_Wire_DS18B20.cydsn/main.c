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
   
****
****
Add CRC check to Scratchpad read
****
***



    uint8 scratchpad[9];
    uint16 tempr;
    float temp_cel;
    uint8 connect_status;
    char msg[20];
    
    UART_Start();
    
    connect_status = Initialize_1_wire();
    
    if(connect_status==0)
    {
    	RED_Write(1);
        GREEN_Write(0);
    }
    else
    {
        RED_Write(0);
        GREEN_Write(1);        
    }
    CyDelay(SETUP_DLY);

    sprintf(msg,"Temp sense Started.");
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
         * CF - this could be the minimum 750ms delay it takes for sensor to 
         * process temp reading ready to read.  
         * Could split this call and use an interupt to trigger the temp read?
         * This way we don't block for this 750ms.
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

        // Complete the 1Wire temp read from the
		Do_scratchpad_read(scratchpad);

        // Combine the result to 16 bit register
		// [0] LSB, [1] MSB
        tempr=(scratchpad[1]<<8)+scratchpad[0];

        temp_cel= convert_2_celsius(tempr);
       
        
        
        sprintf(msg,"TEMPERATURE = %f.2 C",temp_cel);
        UART_PutString(msg);
        UART_PutCRLF();
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
