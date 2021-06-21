/* atmega328p EEPROM code
 * By: John Jekel
 *
 * Allows for reading, writing, and swapping of the values in an atmega328p's internal EEPROM.
*/

#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

uint8_t EEPROM_read(uint16_t address);
void EEPROM_write(uint8_t data, uint16_t address);
uint8_t EEPROM_swap(uint8_t data, uint16_t address);//Returns old data at address and writes new

#endif//EEPROM_H
