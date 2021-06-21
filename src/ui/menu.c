#include "ui/menu.h"
#include "ui/clock.h"
#include "ui/alarm.h"

#include "rtc.h"
#include "lcd.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <stdbool.h>
#include <stdint.h>

/* Typedefs and macros */

#define LAST_SCREEN TIMEOUT

//#define getScreenArrayMemberByte(screen, member) (pgm_read_byte(&ScreenArray[(screen)].member))
typedef enum {ALARM = 0, TIME = 1, DATE = 2, DAY = 3, TIMEOUT = 4} menuScreen_t;

#define getCurrentButtonAction(buttons) ((buttonAction_t)(~(buttons) & 0b11110011))
typedef enum    {LEFT = 1, RIGHT = 1 << 1, UP = 1 << 4, DOWN = 1 << 5, ENTER = 1 << 6,
                EXIT = 1 << 7} buttonAction_t;//NOTE: The enum values chosen map to button pins

typedef struct
{
    const char flavourChar;
    
    const char* PROGMEM flavourTextPointer;
    const uint8_t flavourTextStartPosition;
    
    const uint8_t rightArrowBoundary;//Inclusive (Left boundary is always 1 inclusive)
} ScreenConstants_t;

/* Constants */

//Used to overcome limitation that PSTR() can't be used inside of the ScreenArray initializer
//See https://www.nongnu.org/avr-libc/user-manual/pgmspace.html
const char literal0[] PROGMEM = "Alarm";
const char literal1[] PROGMEM = " Time";
const char literal2[] PROGMEM = "Date";
const char literal3[] PROGMEM = " Day";
const char literal4[] PROGMEM = "Timeout";

const static ScreenConstants_t ScreenConstants[5] PROGMEM =
{
    {'\x6', literal0/*"Alarm"*/, 0x4B, 6},//ALARM
    {'\x7', literal1/*" Time"*/, 0x4B, 8},//TIME
    {'\x5', literal2/*"Date"*/, 0x4C, 10},//DATE
    {'\x5', literal3/*" Day"*/, 0x4C, 1},//DAY
    {'\x7', literal4/*"Timeout"*/, 0x49, 2}//TIMEOUT
};

/* Static Variables */
                
static bool menuReadyToExit;
static menuScreen_t currentMenuScreen;
static bool menuScreenChanged;
static uint8_t arrowPosition;//Only change this with moveArrow (reading is ok)

//For ALARM (used to reduce EEPROM writes and to only apply the setting if enter is pressed)
static bool alarmEnableCache;

//For TIMEOUT (used to reduce EEPROM writes and speed up menu code)
static uint8_t timeoutCache;
static uint8_t timeout10Cache;

/* Static Function Definitions */

static void moveArrow(uint8_t newPosition);

static void oneTime();
static void update();
static void enterButtonResponse();

static uint8_t getValueAtArrowPosition();
static void setValueAtArrowPosition(uint8_t value);
static bool boundsCheckArrowPosition(uint8_t newValue);

/* Public functions */

void MENU_setup()
{
    currentMenuScreen = 0;//Start by configuring the alarm
    menuScreenChanged = true;
    
    //NOTE: No need to turn on the display here because it should already be on from CLOCK mode
    LCD_clear();
    LCD_setDisplayAddress(0xD);
    LCD_print_P(PSTR("Set"));
}

void MENU_update(uint8_t buttons)
{
    if (menuScreenChanged)//1 time operations (only when menu screen changes)
    {
        menuScreenChanged = false;//Clear flag
        
        oneTime();//Do one time operations
    }
    else//Operations that respond to buttons being pushed (ignored when the screen first changes)
    {
        switch (getCurrentButtonAction(buttons))
        {
            case LEFT:
            {
                uint8_t newArrowPosition = arrowPosition - 1;
    
                if (newArrowPosition >= 1)//Left boundary is column 1 (inclusive)
                    moveArrow(newArrowPosition);
                break;
            }
            case RIGHT:
            {
                uint8_t newArrowPosition = arrowPosition + 1;
                uint8_t boundary = 
                    pgm_read_byte(&ScreenConstants[currentMenuScreen].rightArrowBoundary);
                
                if (newArrowPosition <= boundary)//Left boundary is column "boundary" (inclusive)
                    moveArrow(newArrowPosition);
                break;
            }
            case UP:
            {
                const uint8_t newValue = getValueAtArrowPosition() + 1;
    
                if (boundsCheckArrowPosition(newValue))
                    setValueAtArrowPosition(newValue);
                break;
            }
            case DOWN:
            {
                const uint8_t newValue = getValueAtArrowPosition() - 1;
    
                if (boundsCheckArrowPosition(newValue))
                    setValueAtArrowPosition(newValue);
                break;
            }
            case ENTER:
            {
                enterButtonResponse();
                
                if (currentMenuScreen == LAST_SCREEN)
                    menuReadyToExit = true;
                else
                {
                    ++currentMenuScreen;
                    menuScreenChanged = true;
                }
                break;
            }
            case EXIT:
            {
                menuReadyToExit = true;
                break;
            }
            default:
            {
                break;
            }
        }
    }
    
    //Call refresh function for current screen each time this function is called
    update();
}

bool MENU_readyToExit()
{
    return menuReadyToExit;
}

void MENU_clearExitFlag()
{
    menuReadyToExit = false;
}

/* Static Functions */

static void moveArrow(uint8_t newPosition)
{
    //Overwrite old arrow
    LCD_setDisplayAddress(0x40 + arrowPosition);
    LCD_writeCharacter(' ');
    
    //Update arrow position and draw new arrow
    arrowPosition = newPosition;
    LCD_setDisplayAddress(0x40 + arrowPosition);
    LCD_writeCharacter('\x2');
}

static void oneTime()
{
    //Delete old arrow and write new one in rightmost possible position
    uint8_t boundary = pgm_read_byte(&ScreenConstants[currentMenuScreen].rightArrowBoundary);
    moveArrow(boundary);
    
    //Draw "flavour" character
    LCD_setDisplayAddress(0x00);
    char flavourCharacter = pgm_read_byte(&ScreenConstants[currentMenuScreen].flavourChar);
    LCD_writeCharacter(flavourCharacter);
    
    //Set display address to proper position for flavour text
    uint8_t address = pgm_read_byte(&ScreenConstants[currentMenuScreen].flavourTextStartPosition);
    LCD_setDisplayAddress(address);
    
    //Draw flavour text at proper position
    const char* PROGMEM ptr = pgm_read_ptr(&ScreenConstants[currentMenuScreen].flavourTextPointer);
    LCD_print_P(ptr);
    
    switch (currentMenuScreen)
    {
        case ALARM:
        {
            RTC_refreshA2();
            alarmEnableCache = ALARM_isEnabled();
            break;
        }
        case TIME:
        {
            RTC_refreshTime();
            break;
        }
        case DATE:
        {
            RTC_refreshDate();
            break;
        }
        case DAY:
        {
            RTC_refreshDay();
            
            LCD_setDisplayAddress(0x4);//Erase date leftover from DATE
            LCD_printAmount_P(PSTR("       "), 7);
            break;
        }
        case TIMEOUT:
        {
            //Refresh timeout cache from EEPROM
            uint8_t currentTimeoutValue = EEPROM_read(1);
            timeoutCache = (currentTimeoutValue % 10);//1s column
            timeout10Cache = (currentTimeoutValue / 10);//10s column
            
            LCD_setDisplayAddress(0x03);
            LCD_writeCharacter(' ');//To erase third character from DAY
            break;
        }
        default:
        {
            break;
        }
    }
}

static void update()
{
    switch (currentMenuScreen)
    {
        case ALARM:
        {
            char alarmSnippet[5];
    
            ALARM_fillBufferWithAlarmTimeSnippet(alarmSnippet);
            
            LCD_setDisplayAddress(0x01);
            LCD_printAmount(alarmSnippet, 5);
            LCD_writeCharacter(alarmEnableCache ? '\x6' : 'X');//Bell icon if enabled, else an X
            break;
        }
        case TIME:
        {
            CLOCK_printTimeSnippet();
            break;
        }
        case DATE:
        {
            CLOCK_printDateAndDaySnippet();
            break;
        }
        case DAY:
        {
            CLOCK_printDaySnippet();
            break;
        }
        case TIMEOUT:
        {
            LCD_setDisplayAddress(0x01);
            LCD_writeCharacter(timeout10Cache + '0');//10s column
            LCD_writeCharacter(timeoutCache + '0');//1s column
            
            break;
        }
        default:
        {
            break;
        }
    }
}

static void enterButtonResponse()
{
    switch (currentMenuScreen)
    {
        case ALARM:
        {
            RTC_sendA2();//Update alarm 2 in the RTC
            
            if (alarmEnableCache)
                ALARM_enable();
            else
                ALARM_disable();
            
            ALARM_stop();//Clear the match flag in case it was set previously
            
            break;
        }
        case TIME:
        {
            RTC_sendTime();//Update the time in the RTC
            break;
        }
        case DATE:
        {
            RTC_sendDate();//Update the date in the RTC
            break;
        }
        case DAY:
        {
            RTC_sendDay();//Update the day in the RTC
            break;
        }
        case TIMEOUT:
        {
            uint8_t newValue = timeoutCache;//1s column
            newValue += timeout10Cache * 10;//10s column
            
            if (!newValue)
                ++newValue;//Minimum of 1
            
            EEPROM_write(newValue, 1);
            
            break;
        }
        default:
        {
            break;
        }
    }
}

static uint8_t getValueAtArrowPosition()
{
    switch (currentMenuScreen)
    {
        case ALARM:
        {
            switch (arrowPosition)
            {
                case 1:
                {
                    return RTC_get10HoursA2();
                }
                case 2:
                {
                    return RTC_getHoursA2();
                }
                case 4:
                {
                    return RTC_get10MinutesA2();
                }
                case 5:
                {
                    return RTC_getMinutesA2();
                }
                case 6:
                {
                    return alarmEnableCache;
                }
            }
            break;
        }
        case TIME:
        {
            switch (arrowPosition)
            {
                case 1:
                {
                    return RTC_get10Hours();
                }
                case 2:
                {
                    return RTC_getHours();
                }
                case 4:
                {
                    return RTC_get10Minutes();
                }
                case 5:
                {
                    return RTC_getMinutes();
                }
                case 7:
                {
                    return RTC_get10Seconds();
                }
                case 8:
                {
                    return RTC_getSeconds();
                }
            }
            break;
        }
        case DATE:
        {
            switch (arrowPosition)
            {
                case 1:
                {
                    return RTC_get10Date();
                }
                case 2:
                {
                    return RTC_getDate();
                }
                case 4:
                {
                    return RTC_get10Months();
                }
                case 5:
                {
                    return RTC_getMonths();
                }
                case 8:
                {
                    return RTC_getCenturies();
                }
                case 9:
                {
                    return RTC_get10Years();
                }
                case 10:
                {
                    return RTC_getYears();
                }
            }
            break;
        }
        case DAY:
        {
            return RTC_getDay();
            break;
        }
        case TIMEOUT:
        {
            if (arrowPosition == 1)
                return timeout10Cache;//10s column
            else//Arrow position is 2
                return timeoutCache;//1s column
            
            break;
        }
        default:
        {
            break;
        }
    }
    
    return -1;//If out of bounds
}

static void setValueAtArrowPosition(uint8_t value)
{
    switch (currentMenuScreen)
    {
        case ALARM:
        {
            switch (arrowPosition)
            {
                case 1:
                {
                    RTC_set10HoursA2(value);
                    break;
                }
                case 2:
                {
                    RTC_setHoursA2(value);
                    break;
                }
                case 4:
                {
                    RTC_set10MinutesA2(value);
                    break;
                }
                case 5:
                {
                    RTC_setMinutesA2(value);
                    break;
                }
                case 6:
                {
                    alarmEnableCache = value;//Enable alarm if value is 1; else disable it
                    
                    break;
                }
            }
            break;
        }
        case TIME:
        {
            switch (arrowPosition)
            {
                case 1:
                {
                    RTC_set10Hours(value);
                    break;
                }
                case 2:
                {
                    RTC_setHours(value);
                    break;
                }
                case 4:
                {
                    RTC_set10Minutes(value);
                    break;
                }
                case 5:
                {
                    RTC_setMinutes(value);
                    break;
                }
                case 7:
                {
                    RTC_set10Seconds(value);
                    break;
                }
                case 8:
                {
                    RTC_setSeconds(value);
                    break;
                }
            }
            break;
        }
        case DATE:
        {
            switch (arrowPosition)
            {
                case 1:
                {
                    RTC_set10Date(value);
                    break;
                }
                case 2:
                {
                    RTC_setDate(value);
                    break;
                }
                case 4:
                {
                    RTC_set10Months(value);
                    break;
                }
                case 5:
                {
                    RTC_setMonths(value);
                    break;
                }
                case 8:
                {
                    RTC_setCenturies(value);
                    break;
                }
                case 9:
                {
                    RTC_set10Years(value);
                    break;
                }
                case 10:
                {
                    RTC_setYears(value);
                    break;
                }
            }
            break;
        }
        case DAY:
        {
            RTC_setDay(value);
            break;
        }
        case TIMEOUT:
        {
            if (arrowPosition == 1)
                timeout10Cache = value;
            else//Arrow position is 2
                timeoutCache = value;
            
            break;
        }
        default:
        {
            break;
        }
    }
}

bool boundsCheckArrowPosition(uint8_t newValue)
{
    switch (arrowPosition)
    {
        case 1:
        {
            switch (currentMenuScreen)
            {
                case ALARM:
                {
                    if ((newValue == 2) && (RTC_getHoursA2() > 3))//TODO make not awful (make some sort of more regular way of doing this maybe)
                        return false;//Cant allow 24:00 and above (ex. 29:00)
                    else
                        return newValue < 3;//Tens of hours (24 hour time)
                }
                case TIME:
                {
                    if ((newValue == 2) && (RTC_getHours() > 3))//TODO make not awful (make some sort of more regular way of doing this maybe)
                        return false;//Cant allow 24:00 and above (ex. 29:00)
                    else
                        return newValue < 3;//Tens of hours (24 hour time)
                }
                case DAY:
                {
                    //Only values 1 to 7 allowed (Monday to Sunday)
                    return (newValue > 0) && (newValue < 8);
                }
                case DATE:
                {
                    return newValue < 4;//Tens of days
                }
                case TIMEOUT:
                {
                    return newValue < 10;//Base 10
                }
            }
        }
        case 2:
        {
            switch (currentMenuScreen)
            {
                case ALARM:
                {
                    if (RTC_get10HoursA2() < 2)//TODO make not awful (make some sort of more regular way of doing this maybe)
                        return newValue < 10;//Base 10
                    else
                        return newValue < 4;//Max of 23:59 hours
                }
                case TIME:
                {
                    if (RTC_get10Hours() < 2)//TODO make not awful (make some sort of more regular way of doing this maybe)
                        return newValue < 10;//Base 10
                    else
                        return newValue < 4;//Max of 23:59 hours
                }
                case DATE:
                case TIMEOUT:
                default:
                {
                    return newValue < 10;//Base 10
                }
            }
        }
        case 4:
        {
            switch (currentMenuScreen)
            {
                case ALARM:
                case TIME:
                {
                    return newValue < 6;//Tens of minutes
                }
                case DATE:
                {
                    return newValue < 2;//Tens of months
                }
                default:
                {
                    return false;//NOTE: This should never happen
                }
            }
        }
        case 5:
        {
            if (currentMenuScreen == DATE)
                return newValue < 3;//Months can only go up to 12
            else
                return newValue < 10;//Base 10
        }
        case 6:
        {
            if (currentMenuScreen == ALARM)
                return newValue < 2;//Alarm enabled/disabled toggle can only be 1 or 0
        }//Else fallthrough
        case 3:
        {
            return false;
        }
        case 7:
        {
            switch (currentMenuScreen)
            {
                case TIME:
                {
                    return newValue < 6;//Tens of seconds
                }
                case DATE:
                default://NOTE: Default should never happen
                {
                    return false;
                }
            }
        }
        case 8:
        {
            switch (currentMenuScreen)
            {
                case TIME:
                {
                    return newValue < 10;//Seconds (Base 10)
                }
                case DATE:
                {
                    return newValue < 2;//Centuries can only be 0 or 1 with this RTC
                }
                default:
                {
                    return false;//NOTE: This should never happen
                }
            }
        }
        case 9:
        case 10:
        default:
        {
            return newValue < 10;//Base 10 (Digits must be positive (no underflow) and under 10)
        }
    }
}
