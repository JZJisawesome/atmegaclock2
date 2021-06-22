/* atmegaclock2
 * By: John Jekel
 *
 * A neat clock running on the atmega328p, bare metal.
 * Continuation of https://gitlab.com/JZJisawesome/atmegaclock.
 * 
*** TODO revise pinouts, i2c addresses, parts, eeprom map, schematic/connections, etc
 * TODO also put this info into the readme
 *
 * 
 * PB1 is buzzer (Timer 1)
 * PC4 is SDA
 * PC5 is SDL
 * PD0 is button 0
 * PD1 is button 1
 * PD2 is RTC ~INT/SQW
 * PD4 is button 2
 * PD5 is button 3
 * PD6 is button 4
 * PD7 is button 5
 * NOTE: Buttons are debounced in hardware
 * NOTE: Buttons are active low
 * 
 * I2C addresses
 * Lcd address: 0x27
 * External eeprom: 0x57
 * RTClk: 0x68
 * 
 * EEPROM map
 * 0x00: Alarm enabled
 * 0x01: Timeout value
*/

#ifndef __AVR_ARCH__
    #error "Unsupported platform."
#endif

#if F_CPU != 16000000
    #error "Only a 16MHz system clock is supported."
#endif

#include "cmake_config_info.h"
#include "buzzer.h"
#include "i2c.h"
#include "lcd.h"
#include "rtc.h"
#include "ui/ui.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static void splash();
static void lowPowerConfig();
static void initInterrupts();

static const PROGMEM LCD_cgram_t bitmaps;

int main()
{
    //Initialize power-saving settings
    lowPowerConfig();
    
    //Initialize the I2C interface
    I2C_init();
    
    //Initialize the LCD and display the splash screen
    LCD_setCGRAM_P(bitmaps);
    LCD_init();
    splash();
    
    //Initialize the buzzer code
    buzzer_init();
    
    //Initialize the RTC
    RTC_init();
    
    //Setup interrupts (must be done after all other initialization)
    initInterrupts();
    
    //Jump to UI code (never exits)
    UI_scheduler();
}

static void splash()
{
    LCD_setDisplayAddress(0x00);//First line
    LCD_print_P(PSTR("atmegaclock2 " CMAKE_VERSION_MAJOR_STR "." CMAKE_VERSION_MINOR_STR));
    LCD_setDisplayAddress(0x40);//Second line
    LCD_print_P(PSTR("\x2\x4\x5\x6\x7    By: \x8\x1\x8"));
}

static void lowPowerConfig()
{
    PRR = 0b01111111;//Disable all peripherals except I2C; timer 1 for the buzzer is enabled on demand
    ACSR = 0x80;//Disable the analog comparator (set bit 7, clear bit 3)
    SMCR = 0b00000101;//Enable sleep instruction (power down mode)
    
    //Set unused pins for low power consumption
    PORTB |= 0b00011001;//Enable pullups on PB0, PB3, and PB4
    DIDR0 = 0b01001111;//Disable digital inputs on pins PC0, PC1, PC2, PC3, and PC6
    PORTD |= 0b00001000;//Enable pullup on PD3
    
    //Set LED pin as output for debugging, and keep it low if not (to save power)
    DDRB |= 1 << 5;
}

static void initInterrupts()
{
    //TODO shouldn't these be in the respective code that uses interrupts?
    //Configure pin change interrupts for buttons (NOTE: pins are inputs by default)
    PORTD |= 0b11110011;//Enable the internal pullups for button pins (buttons are active low)
    PCMSK2 |= 0b11110011;//Mask pin change interrupt enable bits for all button pins
    PCICR |= 0b00000100;//Enable PCMSK2 pin change interrupts
    
    //Configure RTC square wave external interrupt; NOTE: negedge chosen because from testing, 
    //the RTC updates its internal time on the negedge, so we will get it right after
    EICRA |= 0b00000010;//Configure the EXTI0 interrupt for a negative edge (RTC 1hz output)
    EIMSK |= 1;//Enable EXTI0 interrupt
}

static const PROGMEM LCD_cgram_t bitmaps =
{
    //NOTE: Use as character 8 to avoid interpreting as NULL byte
    {//Character 8 (0): 'J' in JZJ (personal style, different then ascii J)
        0b11111,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b00001,
        0b10001,
        0b01110,
    },
    {//Character 1: 'Z' in JZJ (personal style, different then ascii Z)
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b10000,
        0b10000,
        0b11111,
    },
    {//Character 2: Arrow Up
        0b00100,
        0b01110,
        0b10101,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
    },
    {//Character 3//NOTE: Unused
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
    },
    {//Character 4: Degrees Celsius
        0b11100,
        0b10100,
        0b11100,
        0b00000,
        0b00011,
        0b00100,
        0b00100,
        0b00011,
    },
    {//Character 5: Calendar Grid
        0b10101,
        0b00000,
        0b10101,
        0b00000,
        0b10101,
        0b00000,
        0b10101,
        0b00000,
    },
    {//Character 6: Alarm//TODO maybe take advantage of lower pixels?
        0b00100,
        0b01110,
        0b01110,
        0b01110,
        0b11111,
        0b00000,
        0b00100,
        0b00000,
    },
    {//Character 7: Clock
        0b00000,
        0b01110,
        0b10101,
        0b10111,
        0b10001,
        0b01110,
        0b00000,
        0b00000,
    },
};
