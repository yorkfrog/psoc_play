/* ===========================================================================================
 * Digital Thermometer demo project
 *
 * uses
 * DS18x4: MAXIM DS18B20 Digital Thermometer component (v 0.0)
 * Read temperature simultaneously from up to 8 DS18B20 sensors
 * 
 * Using UART Terminal type command to measure temperature:
 * <cmd><CR><LF>
 * where cmd=
 * 'T' - report all sensors
 * 'T1'- report sensor 1, ..., 'T7'- report sensor 7
 * 
 *
 * ===========================================================================================
 * PROVIDED AS-IS, NO WARRANTY OF ANY KIND, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * FREE TO SHARE, USE AND MODIFY UNDER TERMS: CREATIVE COMMONS - SHARE ALIKE
 * ===========================================================================================
*/

#include <project.h>
#include <stdio.h>  //sprintf
#include <stdlib.h> //atoi

#include <myUART.h> //UART_SCB communication

/*
#if defined (__GNUC__)
    // Add an explicit reference to the floating point printf library
    // to allow the usage of floating point conversion specifiers.
    // This is not linked in by default with the newlib-nano library.
    asm (".global _printf_float");
#endif
*/

int8 SensorIndex; //sensor number to report Temperature

//===========================================
// Function prototypes
//===========================================

void ProcessCommandMsg(void);   // Process received command (do something...)
void ReportTemperature (void);  // convert temperature code to deg C and send to UART


void Initialize(void) //Initialization
{
    CyGlobalIntEnable;  // Enable global interrupts.
    
    Rx_Int_StartEx(MyRxInt);//start Rx interrupt
      
    UART_1_Start();
    CyDelay(100);// waiting for clear start after power o
    UART_1_UartPutString("\r\nTemperature sensor Maxim DS18B20:\r\n");
    UART_1_UartPutString("type command:\r\n");
    
    OneWire_Start();  
}


void main()
{
    Initialize();
    
    
    for(;;) 
    { 
        
        if(IsCharReady())               //UART_1 Rx FIFO buffer not empty
        {        
            if (GetRxStr())             //extract first command message (non-blocking)
            {
                ProcessCommandMsg();    
            }
        }               
		  
        
        if (OneWire_DataReady)          // DS18 completed temperature measurement - begin read dataa
    	{   
            OneWire_ReadTemperature();  // parse sensor data
            ReportTemperature();        //report Temperaure
        }    
        
    }   

} //main




//==============================================================================
// Process UART_Rx messages and Request Temperature
//==============================================================================

void ProcessCommandMsg(void)
{   
    char strMsg[80]={};         // output UART buffer
    
    //check received message for any valid command and execute it if necessary or report old value
    //if command not recognized, then report error
     
   
    if (
        (strcmp(RB.RxStr, "T0")==0) ||      //command to report sensor 0-7 received..
        (strcmp(RB.RxStr, "T1")==0) ||
        (strcmp(RB.RxStr, "T2")==0) ||
        (strcmp(RB.RxStr, "T3")==0) ||
        (strcmp(RB.RxStr, "T4")==0) ||
        (strcmp(RB.RxStr, "T5")==0) ||
        (strcmp(RB.RxStr, "T6")==0) ||
        (strcmp(RB.RxStr, "T7")==0)
    )
    {
        SensorIndex = atoi(RB.valstr);      //select sensor to report
        OneWire_SendTemperatureRequest();   //check temperatures for all sensors 
    }
    
    else if (strcmp(RB.RxStr, "T")==0)      //command to report all sensors received..
    {   
        SensorIndex = -1;                   //report all sensors
        OneWire_SendTemperatureRequest();   //check temperatures for all sensors 
    }
    
    else                                    //command unrecognized - echo back
    {
        sprintf(strMsg,"!%s - error\r\n", RB.RxStr);
        UART_1_UartPutString(strMsg);
    }

     
    
    
}


//==============================================================================
// Convert temperature code to degC and 
// Send result to Terminal using UART
//==============================================================================

void ReportTemperature(void) 
{
    char strMsg[80]={};         // output UART buffer
   
   
    if ((SensorIndex >= 0) && (SensorIndex < OneWire_NumSensors)) //report selected sensor
    {
        sprintf(strMsg,"sensor %d: %s\r\n", SensorIndex, OneWire_GetTemperatureAsString(SensorIndex));
    }
    
    
    else if (SensorIndex == -1) //report all sensors
    {
        strcat(strMsg, OneWire_GetTemperatureAsString(0)); strcat(strMsg, "\t");
        strcat(strMsg, OneWire_GetTemperatureAsString(1)); strcat(strMsg, "\t");
        strcat(strMsg, OneWire_GetTemperatureAsString(2)); strcat(strMsg, "\t");
        strcat(strMsg, OneWire_GetTemperatureAsString(3)); strcat(strMsg, "\t"); 
        strcat(strMsg, OneWire_GetTemperatureAsString(4)); strcat(strMsg, "\t");
        strcat(strMsg, OneWire_GetTemperatureAsString(5)); strcat(strMsg, "\t");
        strcat(strMsg, OneWire_GetTemperatureAsString(6)); strcat(strMsg, "\t");
        strcat(strMsg, OneWire_GetTemperatureAsString(7)); strcat(strMsg, "\r\n"); 
    }
    

       
    UART_1_UartPutString(strMsg);      
}





/* [] END OF FILE */
