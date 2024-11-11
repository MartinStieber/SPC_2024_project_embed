#include "Serial.h"

void USART_Init(unsigned int ubrr)
{
    // Set 9600 baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    // 8 bit, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

Serial::Serial(unsigned int baudrate)
{
    // Set baud rate
    UBRR0H = (unsigned char)((FOSC / 16 / 115200 - 1) >> 8);
    UBRR0L = (unsigned char)(FOSC / 16 / 115200 - 1);
    // 8 bit, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void Serial::send(char &data)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;

    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = '\r'; // Odeslání návratu vozíku

    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = '\n'; // Odeslání nového řádku
    return;
}