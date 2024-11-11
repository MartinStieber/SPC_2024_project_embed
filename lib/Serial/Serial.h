#pragma once

#include <avr/interrupt.h>

#define FOSC 16000000UL // Clock Speed

class Serial
{
public:
    Serial(unsigned int baudrate);
    void send(char& data);
};