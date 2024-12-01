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

    static const uint16_t bit_delay = 100;
    uint8_t brightness = 12;
    uint8_t num_2_digit[10]{
        // XGFEDCBA
        0b00111111,
        0b00000110,
        0b01011011,
        0b01001111,
        0b01100110,
        0b01101101,
        0b01111101,
        0b00000111,
        0b01111111,
        0b01101111};

    void start();
    void stop();
    void writeByte(uint8_t b);
    void setSegments(const uint8_t *segments, uint8_t length, uint8_t pos);
    uint16_t countDigits(uint64_t num);

public:
    void clear();
    void printNum(uint16_t num);
    void printNumChar(const char *str, uint8_t digits);
    void printInit();
    void printMute();
};