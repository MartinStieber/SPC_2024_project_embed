#pragma once

#include <avr/interrupt.h>
#include <stdlib.h>
#include "TQueue.h"

#define FOSC 16000000UL // Clock Speed

class Serial
{
    inline uint16_t calculateBaud(uint32_t baudrate);
    uint16_t countDigits(uint64_t num);
    uint64_t* medianFilterQueue = nullptr;
    uint8_t _coldstart_median_count = 0;
    uint8_t filter_size = 0;
    uint64_t last_sended = 0;
    uint64_t BIAS = 0;


public:
    static volatile char rec;

    static struct TQueue ser_buf;

    Serial(uint32_t baudrate, uint8_t median_filter_size = 0, const uint8_t sending_bias = 2);
    void sendChar(char data);
    void sendString(const char *data);
    void sendNum(uint64_t data);
    void sendMedianFilter(uint64_t data);
    char readChar();
    char available();
};

ISR(USART_RX_vect);