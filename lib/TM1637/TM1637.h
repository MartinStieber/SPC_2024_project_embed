//  Original Author: avishorp@gmail.com
//  Modifications by: Martin Stieber; Jan Lána, 2024
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.4 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, see <http://www.gnu.org/licenses/>.
//
//  Notes:
//  - This library was modified to adapt it for use in SPC_2024_project_embed.
//  - Original source: https://github.com/avishorp/TM1637

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

/**
 * @brief TM1637 display driver class
 */
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

    /**
     * @brief Function to start communication with TM1637
     * 
     * @details This function sets the DIO and CLK lines high, then sets the DIO line low
     * to start communication with the TM1637 display.
     */
    void start();

    /**
     * @brief Function to stop communication with TM1637
     * 
     * @details This function sets the CLK line low, then sets the DIO line low, and finally
     * sets both the CLK and DIO lines high to stop communication with the TM1637 display.
     */
    void stop();

    /**
     * @brief Function to write a byte to TM1637
     * 
     * @details This function writes a byte to the TM1637 display by sending each bit
     * and toggling the CLK line. It also waits for an acknowledgment from the display.
     * 
     * @param b Byte to write
     */
    void writeByte(uint8_t b);

    /**
     * @brief Function to set segments on the TM1637 display
     * 
     * @details This function sets the segments on the TM1637 display by sending the
     * appropriate commands and data. It can set multiple segments starting from a
     * specified position.
     * 
     * @param segments Array of segments to set
     * @param length Length of the segments array
     * @param pos Position to start setting segments
     */
    void setSegments(const uint8_t *segments, uint8_t length, uint8_t pos);

    /**
     * @brief Function to count the number of digits in a number
     * 
     * @details This function counts the number of digits in a given number by
     * recursively dividing the number by 10.
     * 
     * @param num Number to count digits of
     * @return uint16_t Number of digits
     */
    uint16_t countDigits(uint64_t num);

public:
    /**
     * @brief Function to clear the TM1637 display
     * 
     * @details This function clears all segments on the TM1637 display.
     */
    void clear();

    /**
     * @brief Function to print a number on the display
     * 
     * @details This function prints a given number on the TM1637 display by converting
     * the number to its corresponding 7-segment display encoding and setting the segments.
     * 
     * @param num Number to print
     */
    void printNum(uint16_t num);

    /**
     * @brief Function to print a number with characters on the display
     * 
     * @details This function prints a given buffer of characters on the TM1637 display.
     * It converts each character to its corresponding 7-segment display encoding and sets
     * the segments.
     * 
     * @param buffer Buffer containing the characters to print
     * @param length Length of the buffer
     */
    void printNumChar(const char *buffer, uint8_t length);

    /**
     * @brief Function to initialize the display
     * 
     * @details This function initializes the TM1637 display by clearing all segments.
     */
    void printInit();

    /**
     * @brief Function to print "MUTE" on the display
     * 
     * @details This function prints the word "MUTE" on the TM1637 display by setting the
     * appropriate segments for each character.
     */
    void printMute();
};