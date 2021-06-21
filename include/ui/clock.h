#ifndef CLOCK_H
#define CLOCK_H

#include <avr/pgmspace.h>

#include <stdint.h>

//For ui scheduler
void CLOCK_setup();//When mode switch occurs
void CLOCK_update();//To update clock display (only updates what is needed)

//For menu code
void CLOCK_printTimeSnippet();//NOTE: 8 characters long, starting at display address 0x01
void CLOCK_printDaySnippet();//NOTE: 3 characters long, starting at display address 0x01
void CLOCK_printDateAndDaySnippet();//NOTE: 10 characters long, starting at display address 0x01

#endif//CLOCK_H
