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
/*******************************************************************************
    * Function Name: Initialize_1_wire
    ****************************************************************************//**
    *
    * \initializes the 1 wire communication, Master pulls the channel low for 480us atleast.
    * For which the slaves will reply with a 0, within one time slot.
    *
    * <b>Note</b> Done initially to find the presence of devices in the network
    *
    *
    * \param mode
    * no parameter
    *
    * \return
    *  returns the status.
    *
*********************************************************************************************/

uint8 Initialize_1_wire()
{
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
    * \param mode
    * 1 bit write data
    *
    * \return
    *  void function
    *
*********************************************************************************************/
void Write_bit_1_wire(uint8 value)
{   
    /* Minimum recovery time 1us between write steps in ncessary, 20us is used */
    CyDelayUs(RCVRY_TIME);
    /* Write is initiated from this step*/
    Wire1_Write(0); 
    
    if(value==0)
    {
        /* Minimum 60us for logic0 */
        CyDelayUs(WRITE_0_TIME);
    }
    else
    {
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
    * \param mode
    * no parameter
    *
    * \return
    *  1 bit data from slave
    *
*********************************************************************************************/
uint8 Read_bit_1_wire()
{
    /* Minimum recovery time 1us */
    CyDelayUs(RCVRY_TIME);
    /* Initiate read by pulling line low */
    Wire1_Write(0);
    /* Minimum time 1us, to initiate delay */
    CyDelayUs(READ_INIT_DLY); 
    Wire1_Write(1);
    /* Data valid before 15us, should be sampled before that*/
    CyDelayUs(READ_VALID_DLY); 
    return(Wire1_Read());
}

/*******************************************************************************
    * Function Name: Write8_1_wire
    ****************************************************************************//**
    *
    * \Master Writes one byte of data to slave
    * 
    * <b>Note</b> Write 1 byte of data using bit write function 
    * 
    *
    * \param mode
    * Write data
    *
    * \return
    *  void function
    *
*********************************************************************************************/
void Write8_1_wire(uint8 payload) 
    {
	    uint8 BitPayload,  shiftcount;
	    for (shiftcount=0; shiftcount<=7;shiftcount++) 
        {
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
	uint8 IncomingByte=0, shiftcount=0 ;
	for (shiftcount=0; shiftcount<=7; shiftcount++) 
    {
       IncomingByte |= (Read_bit_1_wire())<<shiftcount;
    }
    return(IncomingByte);
}

/*******************************************************************************
    * Function Name:  measure_celsius
    ****************************************************************************//**
    *
    * \Converts the 12 bit sensor data to celcius
    * 
    * <b>Note</b>  
    * Converts 2 byte sensor scratch pad data into accurate float data. 
    *
    * \param mode
    * 2 byte sensor data
    *
    * \return
    *  temperature value in celcius float format
    *
*********************************************************************************************/
float measure_celsius(uint16 temp)
{
    
float temp_c = 0;	
    
if (temp >= 0x800) //temperture is negative 
{	

//calculate the fractional part 
if(temp & 0x0001) temp_c += 0.06250; 
if(temp & 0x0002) temp_c += 0.12500; 
if(temp & 0x0004) temp_c += 0.25000; 
if(temp & 0x0008) temp_c += 0.50000; 

//calculate the whole number part 
temp = (temp >> 4) & 0x00FF; 
temp = temp - 0x0001; //subtract 1 
temp = ~temp; //ones compliment 
temp_c = temp_c - (float)(temp & 0xFF); 
} 
else //temperture is positive 
{ 
temp_c = 0; 
//calculate the whole number part 
temp_c = (temp >> 4) & 0x00FF; 
//calculate the fractional part 
if(temp & 0x0001) temp_c = temp_c + 0.06250; 
if(temp & 0x0002) temp_c = temp_c + 0.12500; 
if(temp & 0x0004) temp_c = temp_c + 0.25000; 
if(temp & 0x0008) temp_c = temp_c + 0.50000; 
}
return temp_c;
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
   
    uint8 scratchpad[9];
    uint16 tempr;
    float temp_cel;
    uint8 connect;
    char msg[20];
    
    UART_Start();
    
    connect= Initialize_1_wire();
    
    if(connect==0)
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
    
    /* As only single sensor is present in channel-Skip command */
     Write8_1_wire(SKIP_ADDR_CMND);
    /*Write Scratch Pad command*/
     Write8_1_wire(WRITE_SC_PAD);
    
    /* Alarm registers are unused, Writing Random values*/
    /*TH*/
    Write8_1_wire(0x55);
    /*TL*/
    Write8_1_wire(0xA2);
    /* Config(resoluion)7F- 12 bit resolution */
    Write8_1_wire(RESOL_12BIT);
    

/*  Each loop contains one one reset operation followed by temperature conversion start command
    
    Once the conversion is complete the Scratch pad from the sensor is copied from the sensor.
    
    First 16 bits of the sensor is the temperature value which is converted into float format and displayed via UART
*/   
    for(;;)
    {
        connect= Initialize_1_wire();
        if(connect==0)
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

        /*Skip address check command */
        Write8_1_wire(SKIP_ADDR_CMND); 
        /* Start Convert*/
        Write8_1_wire(START_CONVERT);
        
        /* Wait until conversion completes */
        /* 
         * CF - this could be the minimum 750ms delay it takes for sensor to 
         * process temp reading ready to read.  
         * Could split this call and use an interupt to trigger the temp read?
         * This way we don't block for this 750ms.
         */
        while(Read_bit_1_wire()==0);
        
         connect= Initialize_1_wire();
        if(connect==0)
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

        /*Skip address check command */
        Write8_1_wire(SKIP_ADDR_CMND);
        /*Read Scratch Pad command*/
        Write8_1_wire(READ_SC_PAD);
        
        /*Copy the scratpad from slave*/
        for(int i=0;i<9;i++)
        {
            scratchpad[i]=Read8_1_wire();
        }
        
        
        /* Combine the result to 16 bit register */
        tempr=(scratchpad[1]<<8)+scratchpad[0];

        temp_cel= measure_celsius(tempr);
       
        
        
        sprintf(msg,"TEMPERATURE = %f C",temp_cel);
        UART_PutString(msg);
        UART_PutCRLF();
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
