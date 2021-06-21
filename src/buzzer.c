/* Buzzer code
 * By: John Jekel
 *
 * Uses Timer 1 and PWM to drive a buzzer attached to PB1.
*/

#include "buzzer.h"

#include <avr/io.h>
#include <stdint.h>

//Constant Definitions
//TODO maybe don't affect channel B so it can be used for other things?

//WGM[3:0] = 0b1001 (Phase and Freq Correct PWM Mode, OCR1A as TOP)
//CS1[2:0] = 0b001 (16mhz timer freq)
#define TCCR1A_BARE_SETTINGS 0b00000001
#define TCCR1B_SETTINGS 0b00010001

//Toggle OC1A on compare match (Phase and Freq Correct PWM Mode)
#define TCCR1A_ENABLED (TCCR1A_BARE_SETTINGS | 0b01000000)
//Force OC1A (PB1) to 0
#define TCCR1A_DISABLED TCCR1A_BARE_SETTINGS

//Functions

void buzzer_init()
{
    DDRB |= 1 << 1;//Set PB1 as output for PWM
    PORTB |= 1 << 1;//Whenever PWM is disabled, force the line high so buzzer turns off
    TCCR1A = TCCR1A_DISABLED;
    TCCR1B = TCCR1B_SETTINGS;
}

void buzzer_setFrequency(uint16_t frequency)
{
    //To modify timer 1 registers, we must enable timer 1
    PRR &= 0b11110111;//Enable timer 1
    
    //OC1A is TOGGLED whenever TOP is reached, which is the value of OCR1AH and OCR1AL
    //Because the timer counts UP AND DOWN in phase and freq correct PWM mode, and the TOP value
    //must be reached twice for 1 PWM cycle (because the pin is TOGGLED when TOP is reached),
    //this means that a PWM cycle takes 4 times as long
    //Therefore, PWM frequency = (F_CPU / OCR1A) / 4  (always with a 50% duty cycle)
    uint16_t topValue = (F_CPU / 4) / frequency;//Rearranged equation
    OCR1AH = (uint8_t)(topValue >> 8);//Top byte
    OCR1AL = (uint8_t)(topValue);//Bottom byte
    
    PRR |= 0b00001000;//Disable timer 1 again
}

void buzzer_enable()
{
    PRR &= 0b11110111;//Enable timer 1
    SMCR = 0b00000001;//Change sleep mode to idle to allow timer 1 to run during sleep
    TCCR1A = TCCR1A_ENABLED;
}

void buzzer_disable()
{
    TCCR1A = TCCR1A_DISABLED;
    PRR |= 0b00001000;//Disable timer 1
    SMCR = 0b00000101;//Set sleep mode type back to power down (time 1 not needed)
}
