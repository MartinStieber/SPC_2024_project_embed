#pragma once

#include <avr/interrupt.h>
#include <stdlib.h>
#include "TQueue.h"

#define FOSC 16000000UL // Clock Speed

class Serial
{
    inline uint32_t calculateBaud(uint32_t baudrate);
    uint16_t countDigits(uint64_t num);

public:
    static volatile char rec;

    static struct TQueue ser_buf;

    Serial(uint32_t baudrate);
    void sendChar(char data);
    void sendString(const char *data);
    void sendNum(uint64_t data);
    char readChar();
    char available();
};

ISR(USART_RX_vect);