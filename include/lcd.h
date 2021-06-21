//Only supports writing to the LCD (no reading)
#ifndef LCD_H
#define LCD_H

#define LCD_ADDRESS 0x27

#include <stdbool.h>
#include <stdint.h>
#include <avr/pgmspace.h>

/* Typedefs */

typedef uint8_t LCD_bitmap_t[8];
typedef LCD_bitmap_t LCD_cgram_t[8];

/* Functions */

void LCD_setCGRAM_P(const LCD_cgram_t cgram);//Pointer to a LCD_cgram_t type
void LCD_init();
#define LCD_on() do {LCD_init();} while (0)//Calls LCD_init and enables backlight
void LCD_off();//Turns of LCD NPN transistor to turn module off
void LCD_clear();
#define LCD_setCGRAMAddress(address) do {LCD_sendCommand(0b01000000 | (address));} while (0)
#define LCD_setDisplayAddress(address) do {LCD_sendCommand(0b10000000 | (address));} while (0)

void LCD_writeCharacter(char character);
void LCD_print(const char* string);
void LCD_print_P(PGM_P string);//String in program space (eg. PSTR("Hello World!"))
void LCD_printAmount(const char* string, uint8_t n);
void LCD_printAmount_P(PGM_P string, uint8_t n);//String in program space (eg. PSTR("Hello World!"))

/* Internal Functions/Macros */

void LCD_sendCommand(uint8_t command);

#endif//LCD_H
