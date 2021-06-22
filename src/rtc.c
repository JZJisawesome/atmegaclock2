#include "rtc.h"
#include "i2c.h"

#include <avr/io.h>
#include <stdint.h>

/* Variables */

uint8_t RTC_data[19];

/* Functions */

void RTC_init()
{
    //Perform the initial read of data from the RTC to the RTC_data[] buffer
    RTC_refreshA2();
    
    //Ensure only minutes and hours are used for the alarm comparison by configuring mask values
    RTC_setMaskA2(2, 0);
    RTC_setMaskA2(3, 0);
    RTC_setMaskA2(4, 1);
    
    //Update the RTC with those new values in the buffer
    RTC_sendA2();
}

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
    
    //Write to all I2C registers
    for (uint_fast8_t i = 0; i < count; ++i)
        I2C_sendByte(RTC_data[startIndex + i]);
    
    I2C_endTransfer();//Done sending data
}
