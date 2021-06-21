#include "ui/alarm.h"

#include "rtc.h"
#include "lcd.h"
#include "buzzer.h"

static bool buzzerEnabled;

void ALARM_setup()
{
    //Turn on the display
    LCD_on();//Also sets display address to 0x00
    
    //Update display to notify about alarm
    LCD_setDisplayAddress(0x00);
    LCD_writeCharacter('\x6');
    char alarmSnippet[5];
    ALARM_fillBufferWithAlarmTimeSnippet(alarmSnippet);
    LCD_printAmount(alarmSnippet, 5);
    LCD_setDisplayAddress(0x0A);
    LCD_print_P(PSTR("Alarm!"));
    
    LCD_setDisplayAddress(0x40);
    LCD_printAmount_P(PSTR(ALARM_STRING), 16);
    
    //Setup the buzzer
    buzzerEnabled = false;
    buzzer_setFrequency(1000);
}

void ALARM_update()
{
    if (buzzerEnabled)
        buzzer_disable();
    else
        buzzer_enable();
    
    buzzerEnabled = !buzzerEnabled;//Toggle buzzer every update period
}

bool ALARM_match()//Checks if RTC alarm2 flag is set
{
    RTC_refreshCSR();
    return (RTC_getCSR() & (1 << 1)) != 0;
}

void ALARM_stop()
{
    //Clear RTC match flag
    RTC_refreshCSR();
    RTC_setCSR(RTC_getCSR() & ~(1 << 1));//Clear alarm 2 flag
    RTC_sendCSR();//Store back to RTC
    
    //Disable the buzzer
    buzzer_disable();
}

void ALARM_fillBufferWithAlarmTimeSnippet(char alarmSnippet[5])
{
    alarmSnippet[0] = RTC_get10HoursA2() + '0';//Tens of hours (24 hour time)
    alarmSnippet[1] = RTC_getHoursA2() + '0';//Hours
    alarmSnippet[2] = ':';
    alarmSnippet[3] = RTC_get10MinutesA2() + '0';//Tens of minutes
    alarmSnippet[4] = RTC_getMinutesA2() + '0';//Minutes
}
