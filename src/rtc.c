#include "rtc.h"
#include "i2c.h"

#include <avr/io.h>
#include <stdint.h>

/* Variables */

uint8_t RTC_data[19];

/* Functions */

void RTC_refreshDataRange(uint8_t startIndex, uint8_t count)
{
    I2C_beginTransfer(RTC_ADDRESS, 0);//Write address to start reading from
    I2C_sendByte(startIndex);//Set address pointer to startIndex
    I2C_beginTransfer(RTC_ADDRESS, 1);//Now we start reading from address 0 (repeated start)
    
    //Read registers from RTC
    for (uint_fast8_t i = 0; i < (count - 1); ++i)//All registers except last
        RTC_data[startIndex + i] = I2C_recieveByte();
    
    //The last register is the last byte read, so we use I2C_recieveLastByte instead
    RTC_data[startIndex + count - 1] = I2C_recieveLastByte();
    
    I2C_endTransfer();//Done receiving data
}

void RTC_sendDataRange(uint8_t startIndex, uint8_t count)
{
    I2C_beginTransfer(RTC_ADDRESS, 0);//Writing
    I2C_sendByte(startIndex);//Set address pointer to startIndex
    
    //Write to all I2C registers (starting with seconds)
    for (uint_fast8_t i = 0; i < count; ++i)
        I2C_sendByte(RTC_data[startIndex + i]);
    
    I2C_endTransfer();//Done sending data
}
