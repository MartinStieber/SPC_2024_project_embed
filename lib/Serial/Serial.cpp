#include "Serial.h"

inline uint16_t Serial::calculateBaud(uint32_t baudrate)
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

Serial::Serial(uint32_t baudrate, uint8_t median_filter_size, const uint8_t sending_bias)
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

    // Double speed
    UCSR0A |= (1 << U2X0);

    queue_init(&ser_buf);

    if (median_filter_size > 0)
    {
        medianFilterQueue = new uint64_t[median_filter_size];
        for (uint8_t i = 0; i < median_filter_size; ++i)
        {
            medianFilterQueue[i] = 0;
        }
    }
    filter_size = median_filter_size;
    BIAS = sending_bias;
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

void Serial::sendMedianFilter(uint64_t num)
{
    if (_coldstart_median_count < filter_size)
    {
        medianFilterQueue[_coldstart_median_count] = num;
        _coldstart_median_count++;
        sendNum(medianFilterQueue[_coldstart_median_count - 1]);
        sendChar('\n');
        last_sended = num;
    }
    else
    {
        for (uint8_t i = 0; i < filter_size - 1; ++i)
        {
            medianFilterQueue[i] = medianFilterQueue[i + 1];
        }
        medianFilterQueue[filter_size - 1] = num;

        // Make a filter
        uint64_t medianFilter[filter_size];
        //Copy values from queue to array
        for (uint8_t i = 0; i < filter_size; ++i)
        {
            medianFilter[i] = medianFilterQueue[i];
        }


        // Optimalized bubble sort
        for (uint8_t i = 0; i < filter_size - 1; ++i)
        {
            for (uint8_t j = i + 1; j < filter_size; ++j)
            {
                if (medianFilter[i] > medianFilter[j])
                {
                    uint64_t temp = medianFilter[i];
                    medianFilter[i] = medianFilter[j];
                    medianFilter[j] = temp;
                }
            }
        }

        // Send median value
        if (abs(medianFilter[uint8_t(filter_size / 2) + 1] - last_sended) > BIAS){
            sendNum(medianFilter[uint8_t(filter_size / 2) + 1]);
            last_sended = medianFilter[uint8_t(filter_size / 2) + 1];
            sendChar('\n');
        }
    }
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