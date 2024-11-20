#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Serial.h"


#define BIAS 10

volatile uint16_t adc_val = 0;
volatile char new_adc_val = 0;

uint16_t last_val = 0;

Serial serial(57600);

void ADC_Init()
{
    ADMUX = (1 << REFS0);                                                             // AVCC reference, MUXx = 0 for ADC0 (A0)
    ADCSRA = (1 << ADATE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE); // enable, free running, prescaler, interrupt
    ADCSRB = (1 << ADTS1) | (1 << ADTS0);                                             // Timer/Counter0 Compare Match A
    ADCSRA |= (1 << ADEN);                                                            // ADC start
}

void Timer0_Init()
{
    TCCR0A = (1 << WGM01);              // CTC
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
    OCR0A = (unsigned char)156;         // For compare every ~10ms => 156
    TIMSK0 = 0;
}

// inline uint32_t calculate_percent(uint16_t adc_out)
// {
//     return (100 * adc_out) / 1024;
// }

inline char check_range_val(uint16_t val)
{
    return (0 <= val && val <= 1023) ? 1 : 0;
}

ISR(ADC_vect)
{
    // PORTB |= (1 << PB4);
    adc_val = ADC;
    new_adc_val = 1;
    // PORTB ^= (1 << PB4);
    TIFR0 |= (1 << OCF0A);
    // PORTB &= ~(1 << PB4);
    return;
}

int main(void)
{
    cli();
    // DDRB = (1 << PB4);
    // PORTB |= (1 << PB4);

    DDRB = (1 << PB5);
    PORTB &= ~(1 << PB5);
    // PORTB ^= ~(1 << PB4);
    sei();

    // Handshake
    char welcome = 0;
    do
    {
        if ((serial.available()) && (serial.readChar() == 'w'))
        {
            serial.sendChar('w');
            welcome = 1;
        }
    } while (welcome == 0);

    // LED indicates success connection
    PORTB |= (1 << PB5);

    ADC_Init();
    Timer0_Init();
    sei();

    // Wait for first ADC
    do
    {
        if (new_adc_val)
        {
            new_adc_val = 0;
            if (check_range_val(adc_val))
            {
                serial.sendNum(adc_val);
                serial.sendChar('\n');
                last_val = adc_val;
                welcome = 0;
            }
        }
    } while (welcome == 1);

    // Main loop

    while (1)
    {
        if (new_adc_val)
        {
            new_adc_val = 0;
            //uint32_t volume = calculate_percent(adc_val);
            if (check_range_val(adc_val) && (abs(adc_val - last_val) > BIAS))
            {
                serial.sendNum(adc_val);
                serial.sendChar('\n');
                last_val = adc_val;
            }
        }
    }
}