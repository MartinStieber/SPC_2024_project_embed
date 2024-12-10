#include "Serial.h"

// Inline function to calculate baud rate register value
inline uint16_t Serial::calculateBaud(uint32_t baudrate)
{
    return FOSC / 16 / baudrate - 1;
}

// Function to count the number of digits in a number
uint16_t Serial::countDigits(uint64_t num)
{
    if (num / 10 == 0)
    {
        return 1;
    }
    return 1 + countDigits(num / 10);
}

// Constructor to initialize Serial communication
Serial::Serial(uint32_t baudrate, uint8_t median_filter_size, const uint8_t sending_bias, char double_speed)
{
    // Set baud rate
    UBRR0H = (unsigned char)(calculateBaud(baudrate) >> 8);
    UBRR0L = (unsigned char)calculateBaud(baudrate);

    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    // Enable RX complete interrupt
    UCSR0B |= (1 << RXCIE0);
    if (double_speed)
        UCSR0A |= (1 << U2X0); // Enable double speed

    // Initialize serial buffer queue
    queue_init(&ser_buf);

    // Initialize median filter queue if size is greater than 0
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

// Function to send a single character over serial
void Serial::sendChar(char data)
{
    // Wait for the transmit buffer to be empty
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    // Put data into buffer, sends the data
    UDR0 = data;
}

// Function to send a string over serial
void Serial::sendString(const char *data)
{
    // Loop through each character in the string and send it
    while (*data)
        sendChar(*data++);
}

// Function to send a number over serial
void Serial::sendNum(uint64_t data)
{
    // Buffer to hold the string representation of the number
    char buffer[20];        // Max length of uint64_t is 20 digits
    utoa(data, buffer, 10); // Convert unsigned integer to string
    sendString(buffer);     // Send the string
}

// Function to send a number with median filtering over serial
void Serial::sendMedianFilter(uint64_t num)
{
    // If still in cold start phase, fill the median filter queue
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
        // Shift the values in the median filter queue
        for (uint8_t i = 0; i < filter_size - 1; ++i)
        {
            medianFilterQueue[i] = medianFilterQueue[i + 1];
        }
        medianFilterQueue[filter_size - 1] = num;

        // Create a copy of the median filter queue for sorting
        uint64_t medianFilter[filter_size];
        for (uint8_t i = 0; i < filter_size; ++i)
        {
            medianFilter[i] = medianFilterQueue[i];
        }

        // Optimized bubble sort to find the median
        for (uint8_t i = 0; i < filter_size - 1; ++i)
        {
            for (uint8_t j = i + 1; j < filter_size; ++j)
            {
                if (medianFilter[i] > medianFilter[j])
                {
                    // Swap the values
                    uint64_t temp = medianFilter[i];
                    medianFilter[i] = medianFilter[j];
                    medianFilter[j] = temp;
                }
            }
        }

        // Send the median value if it differs from the last sent value by more than the bias
        if (abs(medianFilter[uint8_t(filter_size / 2) + 1] - last_sended) > BIAS)
        {
            sendNum(medianFilter[uint8_t(filter_size / 2) + 1]);
            last_sended = medianFilter[uint8_t(filter_size / 2) + 1];
            sendChar('\n');
        }
    }
}

// Function to read a single character from the serial buffer
char Serial::readChar()
{
    uint8_t value;
    queue_front(&ser_buf, &value);
    queue_pop(&ser_buf);
    return value;
}

// Function to check if there are any characters available in the serial buffer
char Serial::available()
{
    return !queue_is_empty(&ser_buf);
}

// Static variable to indicate if a character has been received
volatile char Serial::rec = 0;

// Static variable for the serial buffer queue
struct TQueue Serial::ser_buf;

// Interrupt service routine for USART RX complete
ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    // Push the received data into the serial buffer queue
    if (!queue_push(&Serial::ser_buf, data))
    {
        // If the queue is full, set PB5 to indicate an error
        PORTB |= (1 << PB5);
    }
    else
    {
        // Set the received flag
        Serial::rec = 1;
    }
}