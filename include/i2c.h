/* I2C code
 * By: John Jekel
 *
 * Abstracts I2C communication for rtc.c and lcd.c
 * PC4 is SDA, PC5 is SCL
 * 
** Function Descriptions
 * NOTE: When the next byte you read from the slave will be the last, YOU MUST USE
 *  I2C_recieveLastByte INSTEAD OF I2C_recieveByte (NACK instead of ACK).
 * NOTE: MUST call busyWait after each byte / start bit. Use busyWaitStopBit for the stop bit
 *  Can put code in between however in order to not waste all of the time busy-waiting.
 * 
 * Functions used by both lower level and convenience functions
 * I2C_init                 Must be called before anything else (sets up I2C peripheral and pins)
 * I2C_TWINTIsSet           Returns true if the TWINT flag is set
 * I2C_busyWait             Blocks until the TWINT flag is set
 * I2C_getStatus            Returns the 5bit status value of the I2C peripheral
 * 
 * Convenience Functions (Much much slower, but save lots of program space and easier to use)
 * I2C_beginTransfer        Begins a transfer to the specified I2C address. 0 is write, 1 is read
 * I2C_endTransfer          Stops an I2C transfer
 * I2C_sendByte             Sends a byte to an I2C slave
 * I2C_recieveByte          Reads a byte from an I2C slave (for all except last byte read)
 * I2C_recieveLastByte      Like I2C_recieveByte, but MUST be used for the last byte received
 * 
 * Lower Level Functions (Recommended for speed)
 * I2C_sendStartBit         Sends a start or repeated start bit
 * I2C_sendStopBit          Sends a stop bit
 * I2C_setAddressToTransfer Sets address to transfer after I2C_sendStartBit
 * I2C_setByteToTransfer    Sets data to transfer
 * I2C_transferByteThenACK  Reads in a byte and responds with ACK. Used when reading
 * I2C_transferByteThenNACK Writes without ACK afterwards. Used when writing/reading last byte
 * I2C_transferAddress      Same as I2C_transferByteThenNACK, used after I2C_setAddressToTransfer
 * I2C_getByteRecieved      Returns byte read from slave after I2C_transferByteThenACK
*/

#ifndef I2C_H
#define I2C_H

//TODO get rid of the function macros and changes these to be actual functions (let the compiler do the work of inlining/optimization)

/* Settings */

#define I2C_FREQ 100000//In hz; only 100khz is supported

/* Public Functions and Macros */

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

//Common
void I2C_init();
#define I2C_TWINTIsSet() (TWCR >> 7)
#define I2C_busyWait() while (!I2C_TWINTIsSet())//Wait for TWINT to be set
#define I2C_busyWaitStopBit() while (TWCR & (1 << 4))//Wait for TWSTO to clear
#define I2C_getStatus() (TWSR >> 3)
#define I2C_peripheralEnable() do {PRR &= ~(1 << 7);} while (0)
#define I2C_peripheralDisable() do {PRR |= 1 << 7;} while (0)

//Convenience Functions (Much much slower, but can save program space and are easier to use)
#define I2C_beginTransfer(addr, rd) do {I2C_rawTransfer(((addr) << 1) | ((rd) ? 1 : 0));}while(0)
void I2C_endTransfer();
void I2C_sendByte(uint8_t byte);//NOTE: Use this for all written bytes, including the last
uint8_t I2C_recieveByte();
uint8_t I2C_recieveLastByte();//NOTE: recieveLastByte MUST be used for the last byte received

//Low Level Control (Recommended for speed)
#define I2C_sendStartBit() do {TWCR = I2C_START_BIT_COMMAND;} while (0)
#define I2C_sendStopBit() do {TWCR = I2C_STOP_BIT_COMMAND;} while (0)
#define I2C_setAddressToTransfer(addr, rd) do {TWDR = ((addr) << 1) | ((rd) ? 1 : 0);} while (0)
#define I2C_setByteToTransfer(byte) do {TWDR = (byte);} while (0)
//NACK should be used for every byte when writing, and for the last byte when reading; else ACK
#define I2C_transferByteThenACK() do {TWCR = I2C_TRANSFER_ACK_COMMAND;} while (0)
#define I2C_transferByteThenNACK() do {TWCR = I2C_TRANSFER_NACK_COMMAND;} while (0)
#define I2C_transferAddress() do {I2C_transferByteThenNACK();} while (0)//Allow slave to ACK
#define I2C_getByteRecieved() (TWDR)

/* Internal Functions/Macros */

#define I2C_START_BIT_COMMAND      0b10100100//Clear TWINT, set TWSTA (start bit), keep I2C on
#define I2C_STOP_BIT_COMMAND       0b10010100//Clear TWINT, set TWSTO (stop bit), keep I2C on
#define I2C_TRANSFER_ACK_COMMAND   0b11000100//Clear TWINT, set TWEA (send ACK), keep I2C on 
#define I2C_TRANSFER_NACK_COMMAND  0b10000100//Clear TWINT, send NACK, keep I2C on

void I2C_rawTransfer(uint8_t addressAndRWBit);//Sends start bit, address and r/w bit

#endif//I2C_H
