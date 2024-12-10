#pragma once

#include <avr/interrupt.h>
#include <stdlib.h>
#include "TQueue.h"

#define FOSC 16000000UL // Clock Speed

class Serial
{
    // Inline function to calculate baud rate register value
    inline uint16_t calculateBaud(uint32_t baudrate);
    // Function to count the number of digits in a number
    uint16_t countDigits(uint64_t num);
    // Pointer to the median filter queue
    uint64_t* medianFilterQueue = nullptr;
    // Counter for the cold start phase of the median filter
    uint8_t _coldstart_median_count = 0;
    // Size of the median filter
    uint8_t filter_size = 0;
    // Last sent value
    uint64_t last_sended = 0;
    // Bias value for sending data
    uint64_t BIAS = 0;

public:
    // Static variable to indicate if a character has been received
    static volatile char rec;

    // Static variable for the serial buffer queue
    static struct TQueue ser_buf;

    // Constructor to initialize Serial communication
    Serial(uint32_t baudrate, uint8_t median_filter_size = 0, const uint8_t sending_bias = 2, char double_speed = 0);
    // Function to send a single character over serial
    void sendChar(char data);
    // Function to send a string over serial
    void sendString(const char *data);
    // Function to send a number over serial
    void sendNum(uint64_t data);
    // Function to send a number with median filtering over serial
    void sendMedianFilter(uint64_t data);
    // Function to read a single character from the serial buffer
    char readChar();
    // Function to check if there are any characters available in the serial buffer
    char available();
};

// Interrupt service routine for USART RX complete
ISR(USART_RX_vect);