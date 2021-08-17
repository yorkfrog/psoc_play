/* ==================================================
 * Followed by project posted by 
 * Bob Marlowe TxRx20
 * http://www.cypress.com/?app=forum&id=2233&rID=63901

 * Provided As-is, without any warranties to perform
 * under license terms CREATIVE COMMONS - SHARE ALIKE
 *
 * ==================================================
*/

    
#include <myUART.h>


// defines ==================================================================


uint8	RxBuffer[RxBufferSize];     // Rx circular buffer to hold all incoming command
uint8  *RxReadIndex	 = RxBuffer;    // pointer to position in RxBuffer to write incoming Rx bytes
uint8  *RxWriteIndex = RxBuffer;    // pointer to position in RxBuffer to read and process bytes


char   *RxStrIndex = RB.RxStr;      // pointer to command string buffer (processed messages)
                                    // each Rx command consists of: <byte command><string value><CR>

//===========================================================================

CY_ISR(MyRxInt) //interrupt on Rx byte received
{   
    //move all available characters from Rx queue to RxBuffer
    char c = UART_1_UartGetChar();
    while (c != 0x00u) // FIFO not empty
	{
        *RxWriteIndex++ = c;
	    if (RxWriteIndex >= RxBuffer + RxBufferSize) RxWriteIndex = RxBuffer;  
        c = UART_1_UartGetChar(); //procedd to next char
	}
    
    // Clear interrupt sources (does not clear automatically)
    UART_1_ClearRxInterruptSource(UART_1_INTR_RX_NOT_EMPTY);
}

//===========================================================================
uint8 IsCharReady(void) 
{
	return !(RxWriteIndex == RxReadIndex);
}

//===========================================================================

uint8 GetRxStr(void)
{
    uint8 RxStr_flag = 0;
    static uint8 Ch;//static?
   

	Ch = *RxReadIndex++;       //read next char in buffer
    if (RxReadIndex >= RxBuffer + RxBufferSize) RxReadIndex = RxBuffer;
    
    
    
    //if (Ch == EOM_char)
    if ( (Ch == EOM_CR) || (Ch == EOM_LF) ) //any standard terminator
    {
        *RxStrIndex = 0;        //terminate string excluding EOM_char
        RxStrIndex = RB.RxStr;  //reset pointer
        if (strlen(RB.RxStr) > 0)//non-empty message received
        {
            RxStr_flag  = 1;    //set flag to process message
        }   
    }
    else                        //string body char received
    {
        *RxStrIndex++ = Ch;     //build command message   
        //todo: problem if first char is empty space
        
        
    }   

    return RxStr_flag;        
}

//===========================================================================

//===========================================================================


/* [] END OF FILE */
