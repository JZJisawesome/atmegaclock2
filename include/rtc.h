#ifndef RTC_H
#define RTC_H

/* Settings */

#ifndef RTC_ADDRESS
    #define RTC_ADDRESS 0x68
#endif

/* Public Functions and Macros */

#include <stdbool.h>
#include <stdint.h>

//RTC Communication
//Refreshing provides getter functions with new values
#define RTC_refreshAll()            do {RTC_refreshDataRange(0x0, 19);} while (0)
#define RTC_refreshTime()           do {RTC_refreshDataRange(0x0, 3);} while (0)
#define RTC_refreshDay()            do {RTC_refreshDataRange(0x3, 1);} while (0)
#define RTC_refreshDate()           do {RTC_refreshDataRange(0x4, 3);} while (0)
#define RTC_refreshA1()             do {RTC_refreshDataRange(0x7, 4);} while(0)//Alarm 1
#define RTC_refreshA2()             do {RTC_refreshDataRange(0xB, 3);} while(0)//Alarm 2
#define RTC_refreshControl()        do {RTC_refreshDataRange(0xE, 1);} while (0)
#define RTC_refreshCSR()            do {RTC_refreshDataRange(0xF, 1);} while (0)
#define RTC_refreshAging()          do {RTC_refreshDataRange(0x10, 1);} while (0)
#define RTC_refreshTempMSB()        do {RTC_refreshDataRange(0x11, 1);} while (0)//Integer
#define RTC_refreshTempLSB()        do {RTC_refreshDataRange(0x12, 1);} while (0)//Fractional
#define RTC_refreshTemp()           do {RTC_refreshDataRange(0x11, 2);} while (0)
#define RTC_refreshTimeAndDate()    do {RTC_refreshDataRange(0x0, 7);} while (0)
#define RTC_refreshDateAndDay()     do {RTC_refreshDataRange(0x3, 4);} while (0)
#define RTC_refreshAlarms()         do {RTC_refreshDataRange(0x7, 7);} while(0)
#define RTC_refreshControlAndCSR()  do {RTC_refreshDataRange(0xE, 2);} while (0)
//Sending sends values set by setter functions to the RTC
#define RTC_sendAll()               do {RTC_sendDataRange(0x0, 19);} while (0)
#define RTC_sendTime()              do {RTC_sendDataRange(0x0, 3);} while (0)
#define RTC_sendDay()               do {RTC_sendDataRange(0x3, 1);} while (0)
#define RTC_sendDate()              do {RTC_sendDataRange(0x4, 3);} while (0)
#define RTC_sendA1()                do {RTC_sendDataRange(0x7, 4);} while(0)
#define RTC_sendA2()                do {RTC_sendDataRange(0xB, 3);} while(0)
#define RTC_sendControl()           do {RTC_sendDataRange(0xE, 1);} while (0)
#define RTC_sendCSR()               do {RTC_sendDataRange(0xF, 1);} while (0)
#define RTC_sendAging()             do {RTC_sendDataRange(0x10, 1);} while (0)
#define RTC_sendTimeAndDate()       do {RTC_sendDataRange(0x0, 7);} while (0)
#define RTC_sendDateAndDay()        do {RTC_sendDataRange(0x3, 4);} while (0)
#define RTC_sendAlarms()            do {RTC_sendDataRange(0x9, 6);} while(0)
#define RTC_sendControlAndCSR()     do {RTC_sendDataRange(0xE, 2);} while (0)

//Getters
//Time
#define RTC_getSeconds()            (RTC_getLowNibble  (0x0))
#define RTC_get10Seconds()          (RTC_getHighNibble (0x0))
#define RTC_getMinutes()            (RTC_getLowNibble  (0x1))
#define RTC_get10Minutes()          (RTC_getHighNibble (0x1))
#define RTC_getHours()              (RTC_getLowNibble  (0x2))
#define RTC_get10Hours()            (RTC_getHighNibble (0x2))//24 hour time
#define RTC_get10Hours_12()         (RTC_getHighNibble (0x2) & 0x01)//12 hour time
#define RTC_getPM()                 ((RTC_data[0x2] >> 5) & 0x01)//Is PM (Bit 5)
#define RTC_get12HourTime()         ((RTC_data[0x2] >> 6))//Is in 12 hour time mode (Bit 6)
//Date
#define RTC_getDay()                (RTC_data[0x3])//1 to 7
#define RTC_getDate()               (RTC_getLowNibble  (0x4))//Day of month
#define RTC_get10Date()             (RTC_getHighNibble (0x4))
#define RTC_getMonths()             (RTC_getLowNibble  (0x5))
#define RTC_get10Months()           ((RTC_data[0x5] >> 4) & 0x01)//Bit 4
#define RTC_getYears()              (RTC_getLowNibble  (0x6))
#define RTC_get10Years()            (RTC_getHighNibble (0x6))
#define RTC_getCenturies()          (RTC_data[0x5] >> 7)//Bit 7
//Alarm 1
#define RTC_getSecondsA1()          (RTC_getLowNibble  (0x7))
#define RTC_get10SecondsA1()        (RTC_getHighNibble (0x7) & 0x07)
#define RTC_getMinutesA1()          (RTC_getLowNibble  (0x8))
#define RTC_get10MinutesA1()        (RTC_getHighNibble (0x8) & 0x07)
#define RTC_getHoursA1()            (RTC_getLowNibble  (0x9))
#define RTC_get10HoursA1()          (RTC_getHighNibble (0x9) & 0x03)//24 hour time
#define RTC_get10Hours_12A1()       (RTC_getHighNibble (0x9) & 0x01)//12 hour time
#define RTC_getPMA1()               ((RTC_data[0x9] >> 5) & 0x01)//Is PM (Bit 5)
#define RTC_get12HourTimeA1()       ((RTC_data[0x9] >> 6) & 0x01)//Is in 12 hour time mode(Bit6)
#define RTC_getDayA1()              (RTC_getDateA1())
#define RTC_getDateA1()             (RTC_getLowNibble  (0xA))
#define RTC_get10DateA1()           (RTC_getHighNibble (0xA) & 0x03)
#define RTC_getDayMatchA1()         ((RTC_data[0xA] >> 6) & 0x01)//Match day of week, !month
#define RTC_getMaskA1(number)       (RTC_data[0x7 + (number) - 1] >> 7)//Mask bits are bit 7
//Alarm 2
#define RTC_getMinutesA2()          (RTC_getLowNibble  (0xB))
#define RTC_get10MinutesA2()        (RTC_getHighNibble (0xB) & 0x07)
#define RTC_getHoursA2()            (RTC_getLowNibble  (0xC))
#define RTC_get10HoursA2()          (RTC_getHighNibble (0xC) & 0x03)//24 hour time
#define RTC_get10Hours_12A2()       (RTC_getHighNibble (0xC) & 0x01)//12 hour time
#define RTC_getPMA2()               ((RTC_data[0xC] >> 5) & 0x01)//Is PM (Bit 5)
#define RTC_get12HourTimeA2()       ((RTC_data[0xC] >> 6) & 0x01)//Is in 12 hour time mode(Bit6)
#define RTC_getDayA2()              (RTC_getDateA2())
#define RTC_getDateA2()             (RTC_getLowNibble  (0xD))
#define RTC_get10DateA2()           (RTC_getHighNibble (0xD) & 0x03)
#define RTC_getDayMatchA2()         ((RTC_data[0xD] >> 6) & 0x01)//Match day of week, !month
#define RTC_getMaskA2(number)       (RTC_data[0xB + (number) - 2] >> 7)//Mask bits are bit 7
//Control and Aging
#define RTC_getControl()            (RTC_data[0xE])//Control register
#define RTC_getCSR()                (RTC_data[0xF])//Control/status register
#define RTC_getAging()              (RTC_data[0x10])//Aging register
//Temperature
#define RTC_getTemperatureMSB()     (RTC_data[0x11])//Integer Portion
#define RTC_getTemperatureLSB()     (RTC_data[0x12] >> 6)//Fractional portion (align to right)

//Setters
//Time
#define RTC_setSeconds(number)      do {RTC_setLowNibble (0x0, (number));} while (0)
#define RTC_set10Seconds(number)    do {RTC_setHighNibble(0x0, (number));} while (0)
#define RTC_setMinutes(number)      do {RTC_setLowNibble (0x1, (number));} while (0)
#define RTC_set10Minutes(number)    do {RTC_setHighNibble(0x1, (number));} while (0)
#define RTC_setHours(number)        do {RTC_setLowNibble (0x2, (number));} while (0)
#define RTC_setHighHours(number)    do {RTC_setHighNibble(0x2, (number));} while (0)
#define RTC_set10Hours(number)      do {RTC_data[0x2] &= 0xCF; RTC_data[0x2] |= (number) << 4;}\
                                    while (0)//24 hour time
#define RTC_set10Hours_12(number)   do {RTC_data[0x2] &= 0xEF; RTC_data[0x2] |= (number) << 4;}\
                                    while (0)//12 hour time
#define RTC_setPM(number)           do {RTC_data[0x2] &= 0xDF; RTC_data[0x2] |= (number) << 5;}\
                                    while (0)//Is PM (Bit 5)
#define RTC_set12HourTime(number)   do {RTC_data[0x2] &= 0xBF; RTC_data[0x2] |= (number) << 6;}\
                                    while (0)//Is in 12 hour time mode (Bit 6)
//Date
#define RTC_setDay(number)          do {RTC_data[0x3] = (number);} while (0)//1 to 7
#define RTC_setDate(number)         do {RTC_setLowNibble (0x4, (number));} while (0)//Of Month
#define RTC_set10Date(number)       do {RTC_setHighNibble(0x4, (number));} while (0)
#define RTC_setMonths(number)       do {RTC_setLowNibble (0x5, (number));} while (0)
#define RTC_set10Months(number)     do {RTC_data[0x5] &= 0x8F; RTC_data[0x5] |= (number) << 4;}\
                                    while (0)
#define RTC_setYears(number)        do {RTC_setLowNibble (0x6, (number));} while (0)
#define RTC_set10Years(number)      do {RTC_setHighNibble(0x6, (number));} while (0)
#define RTC_setCenturies(number)    do {RTC_data[0x5] &= 0x7F; RTC_data[0x5] |= (number) << 7;}\
                                    while (0)
//Alarm 1
#define RTC_setSecondsA1(number)    do {RTC_setLowNibble (0x7, (number));} while (0)
#define RTC_set10SecondsA1(number)  do {RTC_data[0x7] &= 0x8F; RTC_data[0x7] |= (number) << 4;}\
                                    while (0)
#define RTC_setMinutesA1(number)    do {RTC_setLowNibble (0x8, (number));} while (0)
#define RTC_set10MinutesA1(number)  do {RTC_data[0x8] &= 0x8F; RTC_data[0x8] |= (number) << 4;}\
                                    while (0)
#define RTC_setHoursA1(number)      do {RTC_setLowNibble (0x9, (number));} while (0)
#define RTC_setHighHoursA1(number)  do {RTC_setHighNibble(0x9, (number));} while (0)
#define RTC_set10HoursA1(number)    do {RTC_data[0x9] &= 0xCF; RTC_data[0x9] |= (number) << 4;}\
                                    while (0)//24 hour time
#define RTC_set10Hours_12A1(number) do {RTC_data[0x9] &= 0xEF; RTC_data[0x9] |= (number) << 4;}\
                                    while (0)//12 hour time
#define RTC_setPMA1(number)         do {RTC_data[0x9] &= 0xDF; RTC_data[0x9] |= (number) << 5;}\
                                    while (0)//Is PM (Bit 5)
#define RTC_set12HourTimeA1(number) do {RTC_data[0x9] &= 0xBF; RTC_data[0x9] |= (number) << 6;}\
                                    while (0)//Is in 12 hour time mode (Bit 6)
#define RTC_setDayA1(number)        do {RTC_setDateA1(number);} while (0)//1 to 7
#define RTC_setDateA1(number)       do {RTC_setLowNibble (0xA, (number));} while (0)//Of Month
#define RTC_set10DateA1(number)     do {RTC_data[0xA] &= 0xCF; RTC_data[0xA] |= (number) << 4;}\
                                    while (0)
#define RTC_setDayMatchA1(number)   do {RTC_data[0xA] &= 0xBF; RTC_data[0xA] |= (number) << 6;}\
                                    while (0)//Alarm will match day of week, not day of month  
#define RTC_setMaskA1(maskNum, num) do {RTC_data[0x7 + (maskNum) - 1] &= 0x7F; \
                                        RTC_data[0x7 + (maskNum) - 1] |= (num) << 7;} while (0)
//Alarm 2
#define RTC_setMinutesA2(number)    do {RTC_setLowNibble (0xB, (number));} while (0)
#define RTC_set10MinutesA2(number)  do {RTC_data[0xB] &= 0x8F; RTC_data[0xB] |= (number) << 4;}\
                                    while (0)
#define RTC_setHoursA2(number)      do {RTC_setLowNibble (0xC, (number));} while (0)
#define RTC_setHighHoursA2(number)  do {RTC_setHighNibble(0xC, (number));} while (0)
#define RTC_set10HoursA2(number)    do {RTC_data[0xC] &= 0xCF; RTC_data[0xC] |= (number) << 4;}\
                                    while (0)//24 hour time
#define RTC_set10Hours_12A2(number) do {RTC_data[0xC] &= 0xEF; RTC_data[0xC] |= (number) << 4;}\
                                    while (0)//12 hour time
#define RTC_setPMA2(number)         do {RTC_data[0xC] &= 0xDF; RTC_data[0xC] |= (number) << 5;}\
                                    while (0)//Is PM (Bit 5)
#define RTC_set12HourTimeA2(number) do {RTC_data[0xC] &= 0xBF; RTC_data[0xC] |= (number) << 6;}\
                                    while (0)//Is in 12 hour time mode (Bit 6)
#define RTC_setDayA2(number)        do {RTC_setDateA1(number);} while (0)//1 to 7
#define RTC_setDateA2(number)       do {RTC_setLowNibble (0xD, (number));} while (0)//Of Month
#define RTC_set10DateA2(number)     do {RTC_data[0xD] &= 0xCF; RTC_data[0xD] |= (number) << 4;}\
                                    while (0)
#define RTC_setDayMatchA2(number)   do {RTC_data[0xD] &= 0xBF; RTC_data[0xD] |= (number) << 6;}\
                                    while (0)//Alarm will match day of week, not day of month
#define RTC_setMaskA2(maskNum, num) do {RTC_data[0xB + (maskNum) - 2] &= 0x7F; \
                                        RTC_data[0xB + (maskNum) - 2] |= (num) << 7;} while (0)
//Control and Aging
#define RTC_setControl(number)      do {RTC_data[0xE] = (number);} while (0)//Control register
#define RTC_setCSR(number)          do {RTC_data[0xF] = (number);} while (0)//Control/status reg
#define RTC_setAging(number)        do {RTC_data[0x10] = (number);} while (0)//Aging register

/* Internal Functions/Macros */
//NOTE: DO NOT USE THESE DIRECTLY UNLESS YOU KNOW WHAT YOU'RE DOING

extern uint8_t RTC_data[19];

void RTC_refreshDataRange(uint8_t startIndex, uint8_t count);//Copies RTC data to RTC_data
void RTC_sendDataRange(uint8_t startIndex, uint8_t count);//Copies RTC_data back to RTC

#define RTC_getHighNibble(index)            (RTC_data[index] >> 4)
#define RTC_getLowNibble(index)             (RTC_data[index] & 0x0F)

//Temporary variables are used here in case that a RTC_getHigh/LowNibble is used as the nibble
#define RTC_setHighNibble(index, nibble)    do {uint8_t temp = (nibble) << 4; \
                                                RTC_data[(index)] &= 0x0F; \
                                                RTC_data[(index)] |= temp;} while (0)

#define RTC_setLowNibble(index, nibble)     do {uint8_t temp = (nibble) & 0x0F; \
                                                RTC_data[(index)] &= 0xF0; \
                                                RTC_data[(index)] |= temp;} while (0)

#endif//RTC_H
