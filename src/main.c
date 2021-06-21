/* NAME//TODO
 * By: John Jekel
 *
 * TODO description
 *
*/

#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

int main()
{
    DDRB |= 1 << 5;
    
    while (true)
    {
        PORTB |= 1 << 5;
        _delay_ms(1000);
        PORTB &= ~(1 << 5);
        _delay_ms(1000);
    }
    
    return 0;//TESTING
}
