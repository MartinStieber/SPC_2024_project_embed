#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t toggle = 0;

ISR(TIMER0_COMPA_vect) {
    // Nastavení příznaku pro přepnutí LED
    toggle = 1;
}

void timer0_init() {
    // Nastavení CTC režimu (WGM02 = 0, WGM01 = 1, WGM00 = 0)
    TCCR0A = (1 << WGM01);
    
    // Nastavení prescaleru na 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    // Nastavení OCR0A pro 1ms interval při 16MHz a prescaleru 64
    OCR0A = 124; // (16MHz / 64) / 1000 - 1 = 124

    // Povolení přerušení při srovnání na OCR0A
    TIMSK0 |= (1 << OCIE0A);
}

int main(void) {
    // Nastavení PB5 jako výstup (LED na pinu 13 pro Arduino Uno)
    DDRB |= (1 << DDB5);
    PORTB &= ~(1 << PORTB5);  // Inicializace LED jako vypnutá

    // Inicializace timeru
    timer0_init();
    // Povolení globálních přerušení
    sei();

    // Počítadlo pro zpomalení blikání LED
    uint16_t count = 0;

    while (1) {
        if (toggle) {
            toggle = 0;
            count++;
            
            // Přepnutí LED přibližně každých 200 ms
            if (count >= 200) {
                PORTB ^= (1 << PORTB5); // Přepnutí stavu LED
                count = 0;
            }
        }
    }

    return 0;
}