/* NAME//TODO
 * By: John Jekel
 *
 * TODO description
 *
*/

//TESTING

/*
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#include "buzzer.h"

int main()
{
    DDRB |= 1 << 5;
    buzzer_init();
    buzzer_setFrequency(1000);
    
    while (true)
    {
        PORTB |= 1 << 5;
        //buzzer_enable();
        _delay_ms(1000);
        PORTB &= ~(1 << 5);
        buzzer_disable();
        _delay_ms(1000);
    }
    
    
    return 0;
}
*/

/* LCD alarm clock
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
*/
//Lcd address: 0x27
//External eeprom: 0x57
//RTClk: 0x68

//EEPROM map
//0x00: Alarm enabled
//0x01: Timeout value

#ifndef __AVR_ARCH__
    #error "Not an AVR; unsupported"
#endif

#include "buzzer.h"
#include "i2c.h"
#include "lcd.h"
#include "rtc.h"
#include "ui/ui.h"

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>

const PROGMEM LCD_cgram_t bitmaps;

static void splash();
static void lowPowerAndClockInit();

int main()
{
    //Initialize various stuffs
    I2C_init();
    LCD_setCGRAM_P(bitmaps);
    LCD_init();
    buzzer_init();
    lowPowerAndClockInit();
    DDRB |= 1 << 5;//Set LED pin as output for debugging, and keep it low if not (to save power)
    
    //Ensure only minutes and hours are used for the alarm comparison
    RTC_refreshA2();
    RTC_setMaskA2(2, 0);
    RTC_setMaskA2(3, 0);
    RTC_setMaskA2(4, 1);
    RTC_sendA2();
    
    //Configure pin change interrupts for buttons (NOTE: pins are inputs by default)
    PORTD |= 0b11110011;//Enable the internal pullups for button pins (buttons are active low)
    PCMSK2 |= 0b11110011;//Mask pin change interrupt enable bits for all button pins
    PCICR |= 0b00000100;//Enable PCMSK2 pin change interrupts
    
    //Configure RTC square wave external interrupt; NOTE: negedge chosen because from testing, 
    //the RTC updates its internal time on the negedge, so we will get it right after
    EICRA |= 0b00000010;//Configure the EXTI0 interrupt for a negative edge (RTC 1hz output)
    EIMSK |= 1;//Enable EXTI0 interrupt
    
    splash();//Display cool splash screen
    UI_scheduler();//Jump to UI code
}

static void splash()
{
    LCD_setDisplayAddress(0x00);//First line
    LCD_print_P(PSTR("AVRClock By: \x8\x1\x8"));//AVR Alarm Clock By JZJ
    LCD_setDisplayAddress(0x40);//Second line
    _delay_ms(30);
    
    //Fake loading bar
    LCD_writeCharacter('\x6');//Alarm bell
    for (uint_fast8_t i = 1; i < 15; ++i)
    {
        _delay_ms(30);
        LCD_writeCharacter('\x7');//Picture of clock
    }
    _delay_ms(30);
    LCD_writeCharacter('\x6');//Alarm bell
    
    _delay_ms(500);//Time to see the splash screen
}

void lowPowerAndClockInit()
{
    SMCR = 0b00000101;//Enable sleep instruction (power down mode)
    PRR = 0b01111111;//Disable all peripherals except I2C; timer 1 for the buzzer is on demand
    ACSR = 0x80;//Disable the analog comparator (set bit 7, clear bit 3)
    
    //Set unused pins for low power consumption
    PORTB |= 0b00011001;//Enable pullups on PB0, PB3, and PB4
    DIDR0 = 0b01001111;//Disable digital inputs on pins PC0, PC1, PC2, PC3, and PC6
    PORTD |= 0b00001000;//Enable pullup on PD3
    
    //NOTE: If you ever set the frequency too low, might need to change ArduinoISP settings
    //SPI_CLOCK
    #if F_CPU != 16000000//Must prescale 16MHz clock
        cli();//Beginning of timed sequence for setting prescaler
        CLKPR = 1 << 7;
        #if F_CPU == 8000000
            CLKPR = 0b00000001;
        #elif F_CPU == 4000000
            CLKPR = 0b00000010;
        #elif F_CPU == 2000000
            CLKPR = 0b00000011;
        #else
            #error "Frequency not supported."
        #endif
        sei();//End of timed sequence
    #endif
}

const PROGMEM LCD_cgram_t bitmaps =
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
