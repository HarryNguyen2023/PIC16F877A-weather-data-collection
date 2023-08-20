#ifndef ModbusRTU_H
#define	ModbusRTU_H

#include <xc.h>
#include "PIC16F877A_UART.h"

// Define I/O pins for MAX487
#define TRISALARM TRISB1
#define ALARM RB1

// Slave address
#define SLAVE_ADD 0x31

// Function fields
#define Temp_Humid 0x33

// Maximum length of data block
#define LEN 4
#define MAX 0xFFFF
#define CRC16_POLYNOMIAL 0xA001

// ModbusRTU frame data
uint8_t ModbusFrame[LEN + 5];
__bit send = 0;

// Error frame structure
typedef struct ErrorFrame{
    uint8_t Function_error;
    uint8_t CRC_error;
}ErrorFrame;

// Initiate functions
void ModbusRTUslaveInit(void);
void Slave_SendData(int temp, uint16_t humid_frq, uint8_t mode);
void SendErrorFrame(ErrorFrame Eframe,uint8_t function);
ErrorFrame ReceiveData_Check(uint8_t *str);
uint16_t CRCcheck(uint8_t *buf, uint8_t len);
__bit Slave_DataHandling(uint8_t *buf);

#endif