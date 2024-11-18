#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Serial.h"

#define BIAS 1

volatile uint16_t adc_val = 0;
volatile char new_adc_val = 0;

uint8_t last_volume = 0;

Serial serial(57600);

void ADC_Init()
{
    ADMUX = (1 << REFS0);                                                            // AVCC reference, MUXx = 0 for ADC0 (A0)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE); // enable, prescaler, interrupt
    ADCSRB = (1 << ADTS1) | (1 << ADTS0);                                            // Timer/Counter0 Compare Match A
    ADCSRA |= (1 << ADSC);                                                           // ADC start
}

void Timer0_Init()
{
    TCCR0A = (1 << WGM01);              // CTC
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
    OCR0A = 156;                        // Compare every ~10ms
}

inline uint16_t calculate_percent(uint16_t adc_out)
{
    return 100 * adc_out / 1024;
}

inline char check_vol_val(char vol)
{
    return (0 <= vol && vol <= 100) ? 1 : 0;
}

ISR(ADC_vect)
{
    adc_val = ADC;
    new_adc_val = 1;
}

int main(void)
{
    while (1)
    {
        DDRB = (1 << PB5);
        PORTB = 0;

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

        // Wait for first ADC
        ADC_Init();
        Timer0_Init();
        TCNT0 = 0;
        sei();

        do
        {
            if (new_adc_val)
            {
                new_adc_val = 0;
                uint8_t volume = calculate_percent(adc_val);
                if (check_vol_val(volume))
                {
                    serial.sendNum(volume);
                    serial.sendChar('\n');
                    last_volume = volume;
                    welcome = 0;
                }
            }
        } while (welcome);

        // Main loop
        while (1)
        {
            if(new_adc_val){
                new_adc_val = 0;
                uint8_t volume = calculate_percent(adc_val);
                if(check_vol_val(volume)){          //  && (abs(last_volume - volume) > BIAS)
                    serial.sendNum(volume);
                    serial.sendChar('\n');
                    last_volume = volume;
                }
            }
        }
    }
}