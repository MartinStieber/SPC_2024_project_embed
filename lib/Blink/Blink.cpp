#include "Blink.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <Serial.h>

static volatile uint8_t toggle = 0;
static uint8_t coldstart = 1;

// Počítadlo pro zpomalení blikání LED
static uint16_t count = 0;

static Serial serial(115200);

ISR(TIMER0_COMPA_vect)
{
    // Nastavení příznaku pro přepnutí LED
    toggle = 1;
}

void timer0_init()
{
    // Nastavení CTC režimu (WGM02 = 0, WGM01 = 1, WGM00 = 0)
    TCCR0A = (1 << WGM01);

    // Nastavení prescaleru na 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    // Nastavení OCR0A pro 1ms interval při 16MHz a prescaleru 64
    OCR0A = 124; // (16MHz / 64) / 1000 - 1 = 124

    // Povolení přerušení při srovnání na OCR0A
    TIMSK0 |= (1 << OCIE0A);
}

void blink()
{
    if (coldstart == 1)
    {
        // Nastavení PB5 jako výstup (LED na pinu 13 pro Arduino Uno)
        DDRB |= (1 << DDB5);
        PORTB &= ~(1 << PORTB5); // Inicializace LED jako vypnutá

        // Inicializace timeru
        timer0_init();
        // Povolení globálních přerušení
        sei();
        coldstart = 0;
    }
    if (toggle)
    {
        toggle = 0;
        count++;

        // Přepnutí LED přibližně každých 200 ms
        if (count >= 200)
        {
            PORTB ^= (1 << PORTB5); // Přepnutí stavu LED
            char data = 'b';
            serial.send(data);
            count = 0;
        }
    }
    return;
}