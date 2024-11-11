#include <avr/io.h>
#include <avr/interrupt.h>
#include "Blink.h"

const uint8_t coldstart = 0;

int main(void)
{
    while (1)
    {
        blink();
    }
}