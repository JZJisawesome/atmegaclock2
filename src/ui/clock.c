#include "ui/clock.h"
#include "rtc.h"
#include "lcd.h"

#include <stdint.h>

/* Constants and static variables */

const PROGMEM char CLOCK_dayStrings[7][3] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

static char topLine[16] = "\x7  :  :        \x4";
static char bottomLine[16] = "\x5  /  /2       .";

/* Static Functions */

static void updateTime()
{
    topLine[1] = RTC_get10Hours() + '0';//Tens of hours (24 hour time)
    topLine[2] = RTC_getHours() + '0';//Hours
    topLine[4] = RTC_get10Minutes() + '0';//Tens of minutes
    topLine[5] = RTC_getMinutes() + '0';//Minutes
    topLine[7] = RTC_get10Seconds() + '0';//Tens of seconds
    topLine[8] = RTC_getSeconds() + '0';//Seconds
}

static void updateDateAndDay()
{
    bottomLine[1] = RTC_get10Date() + '0';//Tens of days
    bottomLine[2] = RTC_getDate() + '0';//Day
    bottomLine[4] = RTC_get10Months() + '0';//Tens of months
    bottomLine[5] = RTC_getMonths() + '0';//Months
    bottomLine[8] = RTC_getCenturies() + '0';//Century
    bottomLine[9] = RTC_get10Years() + '0';//Tens of years
    bottomLine[10] = RTC_getYears() + '0';//Years
    
    //Because RTC_getDay is 1-7, but we want 0-6, access RTC_dayStrings starting at a negative 
    //index. Done at compile time, saving an instruction that would be needed to subtract 1 from 
    //RTC_getDay
    const char* dayOfWeek = (CLOCK_dayStrings - 1)[RTC_getDay()];
    memcpy_P(bottomLine + 12, dayOfWeek, 3);//Copy the 3 characters of the day to the buffer
}

/* Public functions */

//For ui scheduler code

void CLOCK_setup()
{
    //The date, day, and temperature are not updated every second
    //Therefore, when switching back to CLOCK mode, we need to update them once so they aren't  
    //blank when the display turns on (because they're not being updated continuously like the)
    //seconds are.
    RTC_refreshTimeAndDate();//For time, date and day
    RTC_refreshTempMSB();//For temperature
    
    //Update topLine and bottomLine buffers
    updateTime();
    updateDateAndDay();
    //Temperature
    uint8_t temperature = RTC_getTemperatureMSB();//Just the integer part
    uint8_t temperatureWithoutSignBit = temperature & 0x7F;//Remove the sign bit
    topLine[12] = (temperature & 0x80) ? '-' : ' ';//Negative sign for negative temperatures
    topLine[13] = (temperatureWithoutSignBit / 10) + '0';
    topLine[14] = (temperatureWithoutSignBit % 10) + '0';
    
    //Print to LCD
    LCD_on();//Enable the display and backlight
    LCD_setDisplayAddress(0x00);//First line
    LCD_printAmount(topLine, 16);
    LCD_setDisplayAddress(0x40);//Second line
    LCD_printAmount(bottomLine, 16);
}

void CLOCK_update()
{
    RTC_refreshTime();//TODO refresh just needed digits for even better optimization (careful of rollover)
    
    LCD_setDisplayAddress(0x8);
    uint8_t seconds = RTC_getSeconds();
    LCD_writeCharacter(seconds + '0');//Seconds
    
    if (seconds == 0)//Seconds overflowed
    {
        LCD_setDisplayAddress(0x7);
        uint8_t seconds10 = RTC_get10Seconds();
        LCD_writeCharacter(seconds10 + '0');//Tens of seconds
        
        if (seconds10 == 0)//Tens of seconds overflowed
        {
            LCD_setDisplayAddress(0x5);
            uint8_t minutes = RTC_getMinutes();
            LCD_writeCharacter(minutes + '0');//Minutes
            
            if (minutes == 0)//Minutes overflowed
            {
                LCD_setDisplayAddress(0x4);
                uint8_t minutes10 = RTC_get10Minutes();
                LCD_writeCharacter(minutes10 + '0');//Tens of minutes
                
                if (minutes10 == 0)//Tens of minutes overflowed
                {
                    LCD_setDisplayAddress(0x2);
                    uint8_t hours = RTC_getHours();
                    LCD_writeCharacter(hours + '0');//Hours
                    
                    if (hours == 0)//Hours overflowed
                    {
                        LCD_setDisplayAddress(0x1);
                        uint8_t hours10 = RTC_get10Hours();
                        LCD_writeCharacter(hours10 + '0');//Tens of hours
                        
                        if (hours10 == 0)//Tens of hours overflowed
                        {
                            //Update date and day of week
                            RTC_refreshDateAndDay();
                            updateDateAndDay();
                            
                            LCD_setDisplayAddress(0x41);
                            LCD_printAmount(bottomLine + 1, 14);
                        }
                    }
                }
            }
        }
    }
}

//For menu code

void CLOCK_printTimeSnippet()
{
    LCD_setDisplayAddress(0x01);
    updateTime();
    LCD_printAmount(topLine + 1, 8);
}

void CLOCK_printDaySnippet()
{
    LCD_setDisplayAddress(0x01);
    //Because RTC_getDay is 1-7, but we want 0-6, access RTC_dayStrings starting at a negative 
    //index. Done at compile time, saving an instruction that would be needed to subtract 1 from 
    //RTC_getDay
    const char* dayOfWeek = (CLOCK_dayStrings - 1)[RTC_getDay()];
    LCD_printAmount_P(dayOfWeek, 3);
}

void CLOCK_printDateAndDaySnippet()
{
    LCD_setDisplayAddress(0x01);
    updateDateAndDay((CLOCK_dayStrings - 1)[RTC_getDay()]);
    LCD_printAmount(bottomLine + 1, 10);
}
