/*
 * This file is part of the SPC_2024_project_embed project.
 *
 * Copyright (C) 2024 Martin Stieber, Jan Lána
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "Serial.h"
#include "TM1637.h"

#define INT_PIN PCINT21

/**
 * @brief Global variables for mute state and ADC value
 */
volatile char is_muted = 0; ///< Mute state flag
volatile char unmute_handled = 1; ///< Unmute handled flag
volatile char mute_handled = 0; ///< Mute handled flag

volatile uint16_t adc_val = 0; ///< ADC value
volatile char new_adc_val = 0; ///< New ADC value flag

/**
 * @brief Initialize Serial communication with baud rate 57600, median filter size 21, and sending bias 1
 */
Serial serial(57600, 21, 1, 1);

/**
 * @brief Function to initialize ADC
 * 
 * @details This function sets up the ADC with AVCC as the reference voltage,
 * selects ADC0 (A0) as the input channel, enables the ADC, sets the prescaler,
 * enables auto trigger and interrupt, and sets Timer/Counter0 Compare Match A
 * as the trigger source.
 */
void ADC_Init()
{
    // Set AVCC as reference, select ADC0 (A0)
    ADMUX = (1 << REFS0);
    // Enable ADC, set prescaler, enable auto trigger and interrupt
    ADCSRA = (1 << ADATE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);
    // Set Timer/Counter0 Compare Match A as trigger source
    ADCSRB = (1 << ADTS1) | (1 << ADTS0);
    // Enable ADC
    ADCSRA |= (1 << ADEN);
}

/**
 * @brief Function to initialize Timer0
 * 
 * @details This function sets Timer0 to CTC mode, sets the prescaler to 1024,
 * and sets the compare value for approximately 10ms interval. It also disables
 * Timer0 interrupts.
 */
void Timer0_Init()
{
    // Set Timer0 to CTC mode
    TCCR0A = (1 << WGM01);
    // Set prescaler to 1024
    TCCR0B = (1 << CS02) | (1 << CS00);
    // Set compare value for ~10ms interval
    OCR0A = (unsigned char)156;
    // Disable Timer0 interrupts
    TIMSK0 = 0;
}

/**
 * @brief Inline function to check if ADC value is within range
 * 
 * @details This function checks if the given ADC value is within the range
 * of 0 to 1023.
 * 
 * @param val ADC value to check
 * @return char 1 if value is within range, 0 otherwise
 */
inline char check_range_val(uint16_t val)
{
    return (0 <= val && val <= 1023) ? 1 : 0;
}

/**
 * @brief Function to initialize mute functionality
 * 
 * @details This function disables global interrupts, sets INT0 to trigger
 * on rising edge, and enables INT0.
 */
void MUTE_Init()
{
    // Disable global interrupts
    cli();
    // Set INT0 to trigger on rising edge
    EICRA |= ((1 << ISC01) | (1 << ISC00));
    // Enable INT0
    EIMSK |= (1 << INT0);
}

/**
 * @brief ADC interrupt service routine
 * 
 * @details This ISR reads the ADC value, sets the flag for new value, and
 * clears the Timer0 compare match flag.
 */
ISR(ADC_vect)
{
    // Read ADC value and set flag for new value
    adc_val = ADC;
    new_adc_val = 1;
    // Clear Timer0 compare match flag
    TIFR0 |= (1 << OCF0A);
}

/**
 * @brief INT0 interrupt service routine
 * 
 * @details This ISR toggles PB5 and updates the mute state. It also sets
 * the appropriate flags for mute and unmute handling.
 */
ISR(INT0_vect)
{
    // Toggle PB5 and update mute state
    PORTB ^= (1 << PB5);
    is_muted = !is_muted;
    if (is_muted)
    {
        mute_handled = 0;
    }
    else
    {
        unmute_handled = 0;
    }
}

/**
 * @brief Main function
 * 
 * @details This is the main function of the program. It initializes the
 * TM1637 display, disables global interrupts, initializes pins and peripherals,
 * enables global interrupts, performs a handshake with serial communication,
 * and enters the main loop where it handles mute/unmute functionality, ADC
 * value processing, and serial communication.
 * 
 * @return int 
 */
int main(void)
{
    // Initialize TM1637 display
    TM1637 display;
    display.printInit();
    // Disable global interrupts at the beginning
    cli();

    // Initialize pins
    DDRD &= ~(1 << PD7);
    DDRB = (1 << PB5);
    PORTB &= ~(1 << PB5);

    // Initialize all peripherals
    ADC_Init();
    Timer0_Init();

    // Enable global interrupts
    sei();

    // Handshake with serial communication
    char welcome = 0;
    do
    {
        if ((serial.available()) && (serial.readChar() == 'w'))
        {
            serial.sendChar('w');
            welcome = 1;
        }
    } while (welcome == 0);

    // Wait for first ADC value
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
            serial.sendMedianFilter(adc_val);
        }
        if (serial.available())
        {
            if (end_byte_pos == 3)
            {
                end_byte_pos = -1;
            }
            end_byte_pos++;
            buffer[end_byte_pos] = serial.readChar();
            if (buffer[end_byte_pos] == 'r')
            {
                // Reset the system by entering an infinite loop, allowing the watchdog timer to trigger a reset
                display.printNum(69);
                wdt_reset();
                wdt_enable(WDTO_15MS);
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