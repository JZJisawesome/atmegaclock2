//NOTE: Buttons must be debounced in hardware
#include "ui/ui.h"
#include "ui/alarm.h"
#include "ui/clock.h"
#include "ui/menu.h"

#include "rtc.h"
#include "lcd.h"
#include "buzzer.h"
#include "i2c.h"
#include "eeprom.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

/* Constants/Macros and Typedefs */

typedef enum {CLOCK, SLEEP, MENU, ALARM} mode_t;
typedef enum {INIT, BUTTON, RTC_INTERRUPT} wakeupReason_t;

#define clockTimeout (EEPROM_read(1))

/* Static Variables */

static mode_t currentMode = CLOCK;//Start with displaying time and date
static bool updatedMode = true;//The mode was changed (from no previous mode in this case)

static volatile wakeupReason_t wakeupReason = INIT;

static volatile uint8_t portDCapture = 0;//Updated whenever we emerge from sleep

/* Static Function Definitions */

static void sleepUntilInterrupt();
static void decideNextMode();

/* Public Functions */

//Performs all important clock functions
//Constantly loops through, checking buttons, updating screen, checking clock continuously
void UI_scheduler()
{
    while (true)
    {
        //Things that only need to happen when the mode changes
        if (updatedMode)
        {
            //RTC EXTI0 configuration
            switch (currentMode)
            {
                case CLOCK:
                case ALARM:
                {
                    //Square wave used for reading RTC synchronously with time and beeping once/second
                    RTC_setControl(0b00000010);//Enable 1hz output on ~INT/SQW pin (PD2/EXTI0)
                    break;
                }
                case SLEEP:
                case MENU:
                {
                    RTC_setControl(0b00000110);//Enable alarm 2 interrupt (to save more power)
                    break;
                }
            }
            
            RTC_sendControl();//Apply settings set above to RTC
            
            //Mode specific code
            switch (currentMode)
            {
                case CLOCK:
                {
                    //For the clock display to look right, we must reinitialize the display
                    //This is because not everything is updated every second (and there might
                    //be garbage left/the screen might be off)
                    CLOCK_setup();
                    break;
                }
                case SLEEP:
                {
                    //Save power during sleep
                    LCD_off();
                    break;
                }
                case MENU:
                {
                    MENU_setup();
                    break;
                }
                case ALARM:
                {
                    ALARM_setup();
                    break;
                }
            }
            
            updatedMode = false;//Finished with first time mode code
        }
        
        //Things that must happen every loop
        switch (currentMode)
        {
            case CLOCK://Display time, date, day of week, alarm symbol, and temperature
            {
                if (!updatedMode)//The mode switch code above handles the first time for us
                    CLOCK_update();
                break;
            }
            case SLEEP://Low power mode until alarm or button push
            {
                //No need to do anything (we will only wake up now for an alarm/button push)
                break;
            }
            case MENU://Change settings
            {
                MENU_update(portDCapture);
                break;
            }
            case ALARM://Alarm interrupt occurred
            {
                ALARM_update();
                break;
            }
        }
            
        sleepUntilInterrupt();
        
        decideNextMode();
    }
}

void UI_setTimeout(uint8_t newTimeout)
{
    
}

/* Static Functions */

static void sleepUntilInterrupt()
{
    #ifdef DEBUG
        PORTB &= ~(1 << 5);//TEST how MCU is awake (LED is active low)
    #endif
    
    I2C_peripheralDisable();
    
    cli();//Disable BOD before sleep
    MCUCR |= 0b01100000;//Start of timed sequence
    MCUCR |= 0b01000000;
    sei();
    __asm__ __volatile__ ("sleep");//Blocks until interrupt fires (end of BOD timed sequence)
    
    I2C_peripheralEnable();
    
    #ifdef DEBUG
        PORTB |= 1 << 5;//TEST how MCU is awake (LED is active low)
    #endif
}

static void decideNextMode()//Polls buttons and alarms to decide on next mode
{
    static uint8_t timeoutCounter = 0;//Used to decide if it's time to SLEEP
    
    switch (wakeupReason)
    {
        case BUTTON://Push or release (pin change)
        {
            switch (currentMode)
            {
                case ALARM://Does not matter whether button was pushed or released
                {
                    ALARM_stop();//Stop alarm from firing again and turn off buzzer
                }//Fallthrough
                case SLEEP://Does not matter whether button was pushed or released
                {
                    currentMode = CLOCK;//Exit to clock display
                    updatedMode = true;
                    timeoutCounter = 0;//Reset timeout counter for CLOCK
                    break;
                }
                case CLOCK:
                {
                    bool aButtonWasPushed = ((~portDCapture) & 0b11110011) != 0;
                    if (aButtonWasPushed)//A button was pushed (not released)
                    {
                        //Must only trigger when a button was pushed, otherwise releasing a button
                        //after coming out of SLEEP would trigger the MENU
                        currentMode = MENU;
                        updatedMode = true;
                    }
                    
                    break;
                }
                case MENU:
                {
                    //MENU_readyToExit will be updated to true when exit or the last enter is pushed
                    //We will only see that it is true here however after it is released, because
                    //the flag only updates after a call to MENU_update, which happens before the
                    //sleep after the push.
                    if (MENU_readyToExit())
                    {
                        MENU_clearExitFlag();
                        currentMode = CLOCK;//Exit to clock display
                        updatedMode = true;
                        timeoutCounter = 0;//Reset timeout counter for CLOCK
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            
            break;
        }
        case RTC_INTERRUPT:
        {
            switch (currentMode)
            {
                case CLOCK:
                {
                    if (timeoutCounter >= clockTimeout)
                    {
                        currentMode = SLEEP;//Only keep display on for clockTimeout # of updates
                        updatedMode = true;
                    }
                    else
                        ++timeoutCounter;
                }//Fallthrough
                case SLEEP:
                {
                    if (ALARM_isEnabled())
                    {
                        if (ALARM_match())//Poll alarm
                        {
                            currentMode = ALARM;//Override mode with ALARM
                            updatedMode = true;
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case INIT:
        default:
        {
            break;
        }
    }
}

//ISRs

//Fires once per second by RTC 1hz output
//Nice because the interrupt is synchronous with the RTC time incrementing.
//This way seconds on the clock don't skip or get updated at inconsistent intervals
//Also set to fire when an alarm match occurs during SLEEP and MENU
ISR(INT0_vect)
{
    wakeupReason = RTC_INTERRUPT;
    return;//Exit sleep and return to loop
}

//Occurs whenever a button changes state
ISR(PCINT2_vect)
{
    portDCapture = PIND;//Poll the buttons as soon as we awake from sleep
    wakeupReason = BUTTON;
    return;//Exit sleep and return to loop
}
