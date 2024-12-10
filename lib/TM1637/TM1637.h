#pragma once

#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>

#define CLK PORTD5
#define DIO PORTD6

#define TM1637_I2C_COMM1 0x40
#define TM1637_I2C_COMM2 0xC0
#define TM1637_I2C_COMM3 0x80

#define NUM_OFFSET 1

class TM1637
{
    // Bit delay for communication
    static const uint16_t bit_delay = 100;
    // Brightness level of the display
    uint8_t brightness = 12;
    // Array to map numbers to their corresponding 7-segment display encoding
    uint8_t num_2_digit[10]{
        // XGFEDCBA
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111  // 9
    };

    // Function to start communication with TM1637
    void start();
    // Function to stop communication with TM1637
    void stop();
    // Function to write a byte to TM1637
    void writeByte(uint8_t b);
    // Function to set segments on the TM1637 display
    void setSegments(const uint8_t *segments, uint8_t length, uint8_t pos);
    // Function to count the number of digits in a number
    uint16_t countDigits(uint64_t num);

public:
    // Function to clear the TM1637 display
    void clear();
    // Function to display a number on the TM1637 display
    void printNum(uint16_t num);
    // Function to display a string of digits on the TM1637 display
    void printNumChar(const char *str, uint8_t digits);
    // Function to display an initialization pattern on the TM1637 display
    void printInit();
    // Function to display a mute pattern on the TM1637 display
    void printMute();
};