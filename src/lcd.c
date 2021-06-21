/* I2C LCD Backpack Pinout
 * P0 = RS
 * P1 = R/W
 * P2 = EN
 * P3 = Backlight On
 * P4 = D4
 * P5 = D5
 * P6 = D6
 * P7 = D7
*/

#include "lcd.h"
#include "i2c.h"

#include <stdbool.h>
#include <stdint.h>

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

/* Constants/Enums */

typedef enum {COMMAND = 0b0, DATA = 0b1} lcdByteType_t;
typedef const LCD_bitmap_t (*LCD_cgramPointer_t);

/* Static Variables */

static LCD_cgramPointer_t cgramPointer;

/* Private Functions/Macros */

static void latchInLCDByte(uint8_t byte, lcdByteType_t byteType)
{
    uint8_t backLightBit = 0b00001000;//Force backlight on
    
    //For commands, the RS and R/W lines stay low (byteType will be 0b0)
    //For data, RS goes high and the R/W lines stay low (byteType will be 0b1)
    uint8_t highNibble = (byte & 0xF0) | backLightBit | byteType;
    uint8_t lowNibble = (byte << 4) | backLightBit | byteType;
    
    //Write and latch each nibble of the byte (LCDs latch on the negative edge)
    I2C_busyWait();//Wait for previous transfer to complete if any
    
    I2C_sendByte(highNibble | 0b00000100);
    I2C_sendByte(highNibble);//Bring enable line low to create a negedge
    I2C_sendByte(lowNibble | 0b00000100);
    
    //For the last transfer (latching the low nibble), we don't busy wait right away in order to 
    //allow productive code to take up some of the busywaiting time
    I2C_setByteToTransfer(lowNibble);//Bring enable line low to create a negedge
    I2C_transferByteThenNACK();
}

static void beginTransferNoEndBusyWait()
{
    I2C_sendStartBit();
    I2C_busyWait();//Wait for start bit to be sent
    I2C_setByteToTransfer(LCD_ADDRESS << 1);//Address and write bit (0) combined
    I2C_transferAddress();
}

static void initCGRAM_P()
{
    beginTransferNoEndBusyWait();//Writing
    
    //Start by setting CGRAM address to 0
    latchInLCDByte(0b01000000, COMMAND);
    
    //Copy CGRAM contents
    for (uint_fast8_t i = 0; i < 8; ++i)
    {
        for (uint_fast8_t j = 0; j < 8; ++j)
            latchInLCDByte(pgm_read_byte(&cgramPointer[i][j]), DATA);
    }

    I2C_busyWait();
    I2C_endTransfer();//Done copying CGRAM contents
}

/* Public Functions */

void LCD_setCGRAM_P(const LCD_cgram_t cgram)
{
    cgramPointer = cgram;
}

void LCD_init()
{
    DDRB |= 1 << 2;//Set PB2 as output
    PORTB &= ~(1 << 2);//Set PB2 low to turn on PNP transistor
    
    _delay_ms(15);
    //Set LCD to 4 bit access mode and enable backlight
    //Note that I2C backback makes 4 LSBS 1 so the display is set to 2 line and 5x11 characters
    I2C_beginTransfer(LCD_ADDRESS, 0);//Writing
    //First, ensure we start in 8 bit mode
    for (uint_fast8_t i = 0; i < 3; ++i)//Useful: https://www.microchip.com/forums/m734545.aspx
    {
        //0b0011XXXX with enable line high, RS and R/W low, backlight on
        I2C_sendByte(0b00111100);
        I2C_sendByte(0b00111000);//Same, but with enable line brought low
    }
    //Now that we know we're in 8 bit mode, set to 4 bit mode
    I2C_sendByte(0b00101100);//0b001011XX with enable line high, RS and R/W low, backlight on
    I2C_sendByte(0b00101000);//Same, but with enable line brought low
    
    //Now that we're in 4 bit mode, init display w/ no cursor; also clear display
    latchInLCDByte(0b00001100, COMMAND);//Init display
    latchInLCDByte(0b00000001, COMMAND);//Clear display
    I2C_busyWait();
    I2C_endTransfer();
    _delay_us(1520);//Wait after clearing display
    
    initCGRAM_P();//Initialize the LCD's CGRAM now that it is on
}

void LCD_off()
{
    PORTB |= 1 << 2;//Set PB2 high to turn off PNP transistor
}

void LCD_clear()
{
    LCD_sendCommand(0b00000001);
    _delay_us(1520);
}

void LCD_writeCharacter(char character)
{
    beginTransferNoEndBusyWait();//Writing
    latchInLCDByte(character, DATA);
    I2C_busyWait();
    I2C_endTransfer();
}

void LCD_print(const char* string)
{
    beginTransferNoEndBusyWait();//Writing
    
    while (true)
    {
        char character = *string;
        
        if (character)//Not null byte, so write character
            latchInLCDByte(character, DATA);
        else
            break;//End printing loop
        
        ++string;
    }
    
    I2C_busyWait();
    I2C_endTransfer();//Done printing string
}

void LCD_print_P(PGM_P string)
{
    beginTransferNoEndBusyWait();//Writing
    
    while (true)
    {
        char character = pgm_read_byte(string);
        
        if (character)//Not null byte, so write character
            latchInLCDByte(character, DATA);
        else
            break;//End printing loop
        
        ++string;
    }
    
    I2C_busyWait();
    I2C_endTransfer();//Done printing string
}

void LCD_printAmount(const char* string, uint8_t n)
{
    beginTransferNoEndBusyWait();//Writing
    
    for (uint8_t i = 0; i < n; ++i)
    {
        char character = *string;
        
        latchInLCDByte(character, DATA);
        
        ++string;
    }
    
    I2C_busyWait();
    I2C_endTransfer();//Done printing string
}

void LCD_printAmount_P(PGM_P string, uint8_t n)//String in program space (eg. PSTR("Hello World!"))
{
    beginTransferNoEndBusyWait();//Writing
    
    for (uint8_t i = 0; i < n; ++i)
    {
        char character = pgm_read_byte(string);
        
        latchInLCDByte(character, DATA);
        
        ++string;
    }
    
    I2C_busyWait();
    I2C_endTransfer();//Done printing string
}

/* Internal Functions/Macros */

void LCD_sendCommand(uint8_t command)
{
    beginTransferNoEndBusyWait();//Writing
    latchInLCDByte(command, COMMAND);
    I2C_busyWait();
    I2C_endTransfer();
}
