#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Blink.h"
#include "Serial.h"

Serial serial(57600);

uint64_t counter = 0;

int main(void)
{
    while (1)
    {
        //blink();
        serial.sendNum(counter);
        serial.sendChar('\n');
        serial.sendChar('\r');
        counter++;
        if (counter > 200)
        {
            counter = 0;
        }
        _delay_ms(100);
    }
}