#include "config.h"
#include "TC72.h"
#include "Slave_ModbusRTU.h"
#include "PIC16F877A_timer0.h"
#include "HCH_1000.h"

// Define the external crystal frequency
#define _XTAL_FREQ 16000000

// Variable to read data from TC72
uint16_t count_temp = 0;
uint16_t count_humid = 0;
int temp = 0;
uint16_t humid_freq = 0;
uint8_t rcv_buffer[LEN + 5];

// Function to handle interrupt
void __interrupt() ISR(void){
    // Timer 0 overflow interrupt
    if(TMR0IF){
        TMR0 = 6;  
        // Read the value from TC72 every 500 millisecond
        if(count_temp++ == 500){
            count_temp = 0;           
            temp = TC72_Read(); 
        }
        // Read the humidity data every 1 second
        if(count_humid++ == 1000){
            count_humid = 0;
            HCH1000_startMeasure();
        }
        // Clear flag bit
        TMR0IF = 0;
    }
    // Timer 1 overflow interrupt
    if(TMR1IF)
    {
        TMR1 = 0;
        HCH1000_timer1Ovf();
        TMR1IF = 0;
    }
    // Input capture interrupt
    if(CCP1IF)
    {
        if(HCH1000_readFreq())
        {
            humid_freq = HCH1000_getFreq();
        }
        CCP1IF = 0;
    }
    // UART reception interrupt
    if(RCIF)
    {
        if(UARTrcvString((char*)rcv_buffer, LEN + 4))  
        {
            if(Slave_DataHandling(rcv_buffer))
            {
                Slave_SendData(temp, humid_freq, Temp_Humid);
            }
        }    
    }
}

void main(void) {
    // ModbusRTU initialization
    ModbusRTUslaveInit();
    // Timer0 timer module initiate
    TMR0 = 6;
    timer0TimerInit(TIMER0_DIV_16);
    // Initiate the HCH-1000 humidity sensor
    HCH1000_Init();
    // Configure the TC72 temperature sensor
    TC72_Init();
    __delay_ms(100);
    // Main duty
    while(1)
    {
    }
    return;
}
