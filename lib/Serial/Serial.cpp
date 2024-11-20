#include "Serial.h"

inline uint32_t Serial::calculateBaud(uint32_t baudrate)
{
    return FOSC / 16 / baudrate - 1;
}

uint16_t Serial::countDigits(uint64_t num)
{
    if (num/10 == 0)
    {
        return 1;
    }
    return 1 + countDigits(num/10);
}

Serial::Serial(uint32_t baudrate)
{
    // Set baud rate
    UBRR0H = (unsigned char)(calculateBaud(baudrate) >> 8);
    UBRR0L = (unsigned char)calculateBaud(baudrate);
    // 8 bit, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    //Interrupt
    UCSR0B |= (1 << RXCIE0);
    sei();
}

void Serial::sendChar(char data)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;
}

void Serial::sendString(char *data)
{
    while (*data != '\0')
    {
        sendChar(*data);
        ++data;
    }
}

void Serial::sendNum(uint64_t data)
{
    char buffer[countDigits(data) + 1]; //+1 for '\0'
    itoa(data, buffer, 10);
    sendString(buffer);
}

char Serial::readChar()
{
    return rec_char;
    rec = 0;
}

char Serial::available()
{
    return Serial::rec;
}

volatile char Serial::rec_char = 0;
volatile char Serial::rec = 0;

ISR(USART_RX_vect){
    Serial::rec_char = UDR0;
    Serial::rec = 1;
}