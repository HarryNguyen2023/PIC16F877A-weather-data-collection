#include "Slave_ModbusRTU.h"

// Function to initiate the ModbusRTU slave module
void ModbusRTUslaveInit()
{
    // Configure the signal LED
    TRISALARM = 0;
    ALARM = 0;
    // Initiate the UART module
    UARTTransRcvInit();
}

// Function to transfer data using ModbusRTU 
void Slave_SendData(int temp, uint16_t humid_frq, uint8_t function){
    // Derive data bytes
    uint8_t tempL = temp & 0xFF;
    uint8_t tempH = (temp >> 8) & 0xFF;
    uint8_t humidL = humid_frq & 0xFF;
    uint8_t humidH = (humid_frq >> 8) & 0xFF;
    // Mode operation
    if(function == (uint8_t)Temp_Humid){
        // Create data frame
        ModbusFrame[0] = (uint8_t)SLAVE_ADD;
        ModbusFrame[1] = function;
        ModbusFrame[2] = tempL;
        ModbusFrame[3] = tempH;
        ModbusFrame[4] = humidL;
        ModbusFrame[5] = humidH;
        uint16_t CRC = CRCcheck(ModbusFrame, LEN + 2);
        ModbusFrame[6] = CRC & 0xFF;
        ModbusFrame[7] = (CRC >> 8) & 0xFF;
        ModbusFrame[8] = '\0';
        // Send data via RS485 bus
        UARTsendString((char*)ModbusFrame);
        send = 1;
    }
}

// Function to calculate CRC
uint16_t CRCcheck(uint8_t buf[], uint8_t len){
    uint16_t crc = MAX;
    for (uint16_t pos = 0; pos < len; pos++) 
    {
        crc ^= (unsigned int)buf[pos]; // XOR byte into least //sig. byte of crc
        for (uint16_t index = 8; index != 0; index--) // Loop over each bit
        {
            if ((crc & 0x0001) != 0){
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= CRC16_POLYNOMIAL;
            } else // Else LSB is not set
            {
                crc >>= 1; // Just shift right
            }
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}

// Function to check if the data receive is valid or not
ErrorFrame ReceiveData_Check(uint8_t *str)
{
    ErrorFrame eframe;
    // Check for function error
    if(str[1] != (uint8_t)Temp_Humid){
        eframe.Function_error = 1;
    }else{
        eframe.Function_error = 0;
    }
    // Check for CRC error
    uint16_t CRC = CRCcheck(str, LEN + 2);
    uint8_t CRCL = CRC & 0xFF;
    uint8_t CRCH = CRC >> 8;
    if(str[6] != CRCL || str[7] != CRCH){
        eframe.CRC_error = 1;
    }else{
        eframe.CRC_error = 0;
    }
    return eframe;
}

// Function to send error frame
void SendErrorFrame(ErrorFrame Eframe,uint8_t function){
    if(send == 0){
        // Create data frame
        ModbusFrame[0] = (uint8_t)SLAVE_ADD;
        ModbusFrame[1] = function | 0x80;
        ModbusFrame[2] = 0x00;
        ModbusFrame[3] = Eframe.Function_error;
        ModbusFrame[4] = Eframe.CRC_error;
        ModbusFrame[5] = 0x00;
        uint16_t CRC = CRCcheck(ModbusFrame,LEN + 2);
        ModbusFrame[6] = CRC & 0xFF;
        ModbusFrame[7] = CRC >> 8;
        ModbusFrame[8] = '\0';
        // Send data via RS485 bus
        UARTsendString((char*)ModbusFrame);
        send  = 1;
    }
}

// Function for receive data handling
__bit Slave_DataHandling(uint8_t *buf)
{
    // Check if the message is transfered for this slave
    if(buf[0] != SLAVE_ADD){
        return 0;
    }
    // Check if the message is a error frame
    if((buf[1] & 0x80) == 0x80){
        ALARM = 1;
        return 0;
    }
    // Case this is a command message
    ErrorFrame eframe = ReceiveData_Check(buf);
    // Case of error occur
    if(eframe.Function_error == 0x01 || eframe.CRC_error == 0x01){
        // Send error frame
        SendErrorFrame(eframe, buf[1]);
        return 0;
    }
    // Case everything OK
    return 1;
}

