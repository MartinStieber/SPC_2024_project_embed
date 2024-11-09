#include <avr/io.h>
#include <avr/interrupt.h>
#include "Blink.h"

int main(void)
{
    while (1)
    {
        blink();
    }
}