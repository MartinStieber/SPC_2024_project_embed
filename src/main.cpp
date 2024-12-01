#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "Serial.h"
#include "TM1637.h"

#define INT_PIN PCINT21

volatile char is_muted = 0;
volatile char unmute_handled = 1;
volatile char mute_handled = 0;

volatile uint16_t adc_val = 0;
volatile char new_adc_val = 0;

Serial serial(57600, 21, 1);

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

inline char check_range_val(uint16_t val)
{
    return (0 <= val && val <= 1023) ? 1 : 0;
}

void MUTE_Init()
{
    cli();                                  // Disable global interrupts
    EICRA |= ((1 << ISC01) | (1 << ISC00)); // Set INT0 to trigger on rising edge
    EIMSK |= (1 << INT0);                   // Enable INT0
}

ISR(ADC_vect)
{
    adc_val = ADC;
    new_adc_val = 1;
    TIFR0 |= (1 << OCF0A);
}

ISR(INT0_vect)
{
    PORTB ^= (1 << PB5);
    is_muted = !is_muted; // Jednodušší přepínání stavu
    if (is_muted)
    {
        mute_handled = 0;
    }
    else
    {
        unmute_handled = 0;
    }
}

int main(void)
{
    TM1637 display;
    display.printInit();
    cli(); // Vypneme přerušení na začátku

    // Inicializace pinů
    DDRD &= ~(1 << PD7);
    DDRB = (1 << PB5);
    PORTB &= ~(1 << PB5);

    // Inicializace všech periferií
    ADC_Init();
    Timer0_Init();

    sei(); // Až teď povolíme přerušení

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
    // PORTB |= (1 << PB5);

    // Wait for first ADC
    do
    {
        if (new_adc_val)
        {
            new_adc_val = 0;
            if (check_range_val(adc_val))
            {
                serial.sendMedianFilter(adc_val);
                welcome = 0;
            }
        }
    } while (welcome == 1);

    char display_change = 0;
    int end_byte_pos = -1;
    char buffer[4] = {'\0', '\0', '\0', '\0'};

    char last_display_val[4] = {'\0', '\0', '\0', '\0'};
    char last_end_byte_pos = -1;

    // Main loop
    MUTE_Init();
    sei();
    while (1)
    {
        if (!unmute_handled)
        {
            serial.sendNum(adc_val);
            serial.sendChar('\n');
            display.printNumChar(last_display_val, last_end_byte_pos);
            for (int i = last_end_byte_pos; i >= 0; --i)
            {
                last_display_val[i] = '\0';
            }
            last_end_byte_pos = -1;
            unmute_handled = 1;
        }
        if (is_muted && !mute_handled)
        {
            serial.sendNum(0);
            serial.sendChar('\n');
            display.printMute();
            mute_handled = 1;
        }
        if (display_change && !is_muted)
        {
            display.printNumChar(buffer, end_byte_pos);
            for (int i = end_byte_pos; i >= 0; --i)
            {
                buffer[i] = '\0';
            }
            end_byte_pos = -1;
            display_change = 0;
        }
        if (new_adc_val && !is_muted)
        {
            new_adc_val = 0;
            {
                serial.sendMedianFilter(adc_val);
            }
        }
        if (serial.available())
        {
            if (end_byte_pos == 3)
            {
                end_byte_pos = -1;
            }
            end_byte_pos++;
            buffer[end_byte_pos] = serial.readChar();
            if (buffer[end_byte_pos] == 'r'){
                // Reset the system by entering an infinite loop, allowing the watchdog timer to trigger a reset

                display.printNum(69);
                // Reset watchdog timer
                wdt_reset();
                // Enable watchdog timer with a timeout period
                wdt_enable(WDTO_15MS);
                // Wait for watchdog to reset the microcontroller
                while (1) {}
            }
            if (buffer[end_byte_pos] == '\n')
            {
                display_change = 1;
                for (int i = end_byte_pos; i >= 0; --i)
                {
                    last_display_val[i] = buffer[i];
                }
                last_end_byte_pos = end_byte_pos;
            }
        }
    }
}