/* Buzzer code
 * By: John Jekel
 *
 * Uses Timer 1 and PWM to drive a buzzer attached to PB1.
*/

#ifndef BUZZER_H
#define BUZZER_H

#include <avr/io.h>
#include <stdint.h>

void buzzer_init();
void buzzer_setFrequency(uint16_t frequency);
void buzzer_enable();
void buzzer_disable();

#endif//BUZZER_H
