/* I2C code
 * By: John Jekel
 *
 * Abstracts I2C communication for rtc.c and lcd.c
*/

#include "i2c.h"
//Things that didn't make sense as macros :)
#include <stdbool.h>
#include <stdint.h>

#include <avr/io.h>
//TODO for interrupt-based IO in the future (can sleep while busywaiting)
//#include <avr/interrupt.h>

/* Functions */

//Common

void I2C_init()
{
    //SCL (PC5) and SDA (PC4) start as inputs
    PORTC |= 0b00110000;//Set SCL and SDA high so that the internal pullups will be used
    
    //Set master SCK frequency
    //NOTE: No need to set TWBR to 0 because it is the default (Prescaler Value = 1)
    #if F_CPU < 2000000
        #error "Minimum supported frequency for I2C peripheral is 2MHz."
    #endif
    TWBR = ((F_CPU / I2C_FREQ) - 16) / 2;//See datasheet page 221/222
}

//Convenience

void I2C_endTransfer()
{
    I2C_sendStopBit();
    I2C_busyWaitStopBit();//Wait for stop bit to be sent
}

void I2C_sendByte(uint8_t byte)
{
    I2C_setByteToTransfer(byte);
    I2C_transferByteThenNACK();
    I2C_busyWait();//Wait for byte to finish transfer
}

uint8_t I2C_recieveByte()
{
    I2C_transferByteThenACK();
    I2C_busyWait();//Wait for byte to be read into data register
    return I2C_getByteRecieved();
}

uint8_t I2C_recieveLastByte()
{
    I2C_transferByteThenNACK();//Send NACK instead of ACK for the last byte to receive
    I2C_busyWait();//Wait for byte to be read into data register
    return I2C_getByteRecieved();
}

//Internal Use Functions

void I2C_rawTransfer(uint8_t addressAndRWBit)//Sends start bit, address and r/w bit
{
    I2C_sendStartBit();
    I2C_busyWait();//Wait for start bit to be sent
    I2C_setByteToTransfer(addressAndRWBit);//Address and r/w bit combined
    I2C_transferAddress();
    I2C_busyWait();//Wait for address to be transferred
}
