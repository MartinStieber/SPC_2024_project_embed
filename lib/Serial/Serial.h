#pragma once

#include <avr/interrupt.h>
#include <stdlib.h>
#include "TQueue.h"

#define FOSC 16000000UL // Clock Speed

/**
 * @brief Serial communication class
 */
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

    /**
     * @brief Constructor to initialize Serial communication
     * 
     * @param baudrate Baud rate for communication
     * @param median_filter_size Size of the median filter
     * @param sending_bias Bias for sending data
     * @param double_speed Enable double speed mode
     * 
     * @details !!!For greaters baud rates than 57600, double_speed must be set to 1 and baud rate must be divided by 2!!!
     * This constructor sets the baud rate, frame format, and enables
     * the receiver and transmitter. It also initializes the serial buffer queue
     * and the median filter queue if the median filter size is greater than 0.
     */
    Serial(uint32_t baudrate, uint8_t median_filter_size = 0, const uint8_t sending_bias = 2, char double_speed = 0);

    /**
     * @brief Function to send a single character over serial
     * 
     * @details This function waits for the transmit buffer to be empty before
     * sending the character. It ensures that the character is sent correctly
     * by checking the UDRE0 flag.
     * 
     * @param data Character to send
     */
    void sendChar(char data);

    /**
     * @brief Function to send a string over serial
     * 
     * @details This function loops through each character in the string and
     * sends it using the sendChar function. It stops when it reaches the null
     * terminator of the string.
     * 
     * @param data String to send
     */
    void sendString(const char *data);

    /**
     * @brief Function to send a number over serial
     * 
     * @details This function converts the number to a string representation
     * and then sends it using the sendString function. It supports sending
     * numbers up to 64-bit unsigned integers.
     * 
     * @param data Number to send
     */
    void sendNum(uint64_t data);

    /**
     * @brief Function to send a number with median filtering over serial
     * 
     * @details This function applies a median filter to the number before
     * sending it. It maintains a queue of recent numbers and calculates the
     * median value. If the median value differs from the last sent value by
     * more than the specified bias, it sends the median value.
     * 
     * @param data Number to send
     */
    void sendMedianFilter(uint64_t data);

    /**
     * @brief Function to read a single character from the serial buffer
     * 
     * @details This function reads a character from the serial buffer queue.
     * It removes the character from the queue after reading it.
     * 
     * @return char Character read from the buffer
     */
    char readChar();

    /**
     * @brief Function to check if there are any characters available in the serial buffer
     * 
     * @details This function checks if the serial buffer queue is empty or not.
     * It returns 1 if there are characters available, otherwise it returns 0.
     * 
     * @return char 1 if characters are available, 0 otherwise
     */
    char available();
};

// Interrupt service routine for USART RX complete
ISR(USART_RX_vect);