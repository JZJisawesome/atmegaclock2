#ifndef ALARM_H
#define ALARM_H

//Must be 16 characters
#define ALARM_STRING "Wakey wakey, \x8\x1\x8"

#include "eeprom.h"

#include <stdbool.h>

//For ui code
void ALARM_setup();
void ALARM_update();
bool ALARM_match();//Check if hours and minutes from RTC match alarm (even if ALARM_isEnabled == 0)
#define ALARM_isEnabled() (EEPROM_read(0) != 0)
void ALARM_stop();

//For menu code
//Note: to configure the alarm itself, configure alarm2
#define ALARM_enable() do {EEPROM_write(true, 0);} while (0)
#define ALARM_disable() do {EEPROM_write(false, 0);} while (0)
void ALARM_fillBufferWithAlarmTimeSnippet(char alarmSnippet[5]);

#endif//ALARM_H
