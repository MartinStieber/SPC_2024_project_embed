#include "Serial.h"

inline uint32_t Serial::calculateBaud(uint32_t baudrate)
{
    return FOSC / 16 / baudrate - 1;
}

uint16_t Serial::countDigits(uint64_t num)
{
    if (num / 10 == 0)
    {
        return 1;
    }
    return 1 + countDigits(num / 10);
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
    // Interrupt
    UCSR0B |= (1 << RXCIE0);
    sei();
    queue_init(&ser_buf);
}

void Serial::sendChar(char data)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;
}

void Serial::sendString(const char *data)
{                 // Přidán const
    while (*data) // Kratší zápis
        sendChar(*data++);
}

void Serial::sendNum(uint64_t data)
{
    // Optimalizace velikosti bufferu
    char buffer[20];        // Max délka uint64_t je 20 číslic
    utoa(data, buffer, 10); // Použití utoa místo itoa pro unsigned hodnoty
    sendString(buffer);
}

char Serial::readChar()
{
    uint8_t value;
    queue_front(&ser_buf, &value);
    queue_pop(&ser_buf);
    return value;
}

char Serial::available()
{
    return !queue_is_empty(&ser_buf);
}

volatile char Serial::rec = 0;

struct TQueue Serial::ser_buf;

ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    if (!queue_push(&Serial::ser_buf, data))
    {
        PORTB |= (1 << PB5);
    }
    else
        Serial::rec = 1;
}