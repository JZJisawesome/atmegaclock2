/* NAME//TODO
 * By: John Jekel
 *
 * TODO description
 *
*/

//TESTING

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
        buzzer_enable();
        _delay_ms(1000);
        PORTB &= ~(1 << 5);
        buzzer_disable();
        _delay_ms(1000);
    }
    
    
    return 0;
}
