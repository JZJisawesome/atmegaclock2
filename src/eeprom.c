/* atmega328p EEPROM code
 * By: John Jekel
 *
 * Allows for reading, writing, and swapping of the values in an atmega328p's internal EEPROM.
*/

#include "eeprom.h" 

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t EEPROM_read(uint16_t address)
{
    while (EECR & (1 << 1));//Wait for a previous write to finish (EEPE to clear)
    
    EEARH = (uint8_t)(address >> 8);//msbs of address
    EEARL = (uint8_t)(address & 0xFF);//lsbs of address
    
    EECR |= 1;//Blocks until read finishes
    
    return EEDR;//Return data at address
}

void EEPROM_write(uint8_t data, uint16_t address)
{
    EEPROM_swap(data, address);//Same as EEPROM_swap, but discards the old value
}

uint8_t EEPROM_swap(uint8_t data, uint16_t address)//Returns old data at address
{
    //Assumes bits EEPM1 and EEPM0 within EECR are equal to zero
    
    cli();//Early to avoid any problems because of interrupts
    
    uint8_t oldData = EEPROM_read(address);//Will block while a previous write is in progress
    //NOTE: We don't have to worry about SELFPRGEN in SPMCSR because we never program flash
    
    if (data != oldData)//Save write wear
    {
        //AVREEPROM_read already set the EEARH and EEARL registers, so we can skip that here
        EEARH = (uint8_t)(address >> 8);//MSBs of address
        EEARL = (uint8_t)(address & 0xFF);//LSBs of address
        
        EEDR = data;
        
        EECR |= 1 << 2;//Set EEMPE (start of timed sequence)
        EECR |= 1 << 1;//Set EEPE (end of timed sequence)
    }
    
    sei();
    return oldData;
}
