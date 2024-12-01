#include "TM1637.h"

void TM1637::start()
{
    DDRD |= (1 << DIO);
    _delay_us(bit_delay);
}

void TM1637::stop()
{
    DDRD |= (1 << DIO);
    _delay_us(bit_delay);
    DDRD &= ~(1 << CLK);
    _delay_us(bit_delay);
    DDRD &= ~(1 << DIO);
    _delay_us(bit_delay);
}

void TM1637::writeByte(uint8_t b)
{
    uint8_t data = b;

    // 8 Data Bits
    for (uint8_t i = 0; i < 8; i++)
    {
        // CLK low
        DDRD |= (1 << CLK);
        _delay_us(bit_delay);

        // Set data bit
        if (data & 0x01)
            DDRD &= ~(1 << DIO);
        else
            DDRD |= (1 << DIO);

        _delay_us(bit_delay);

        // CLK high
        DDRD &= ~(1 << CLK);
        _delay_us(bit_delay);
        data = data >> 1;
    }

    // Wait for acknowledge
    // CLK to zero
    DDRD |= (1 << CLK);
    DDRD &= ~(1 << DIO);
    _delay_us(bit_delay);

    // CLK to high
    DDRD &= ~(1 << CLK);
    _delay_us(bit_delay);
    uint8_t ack = PIND & (1 << DIO);
    if (ack == 0)
        DDRD |= (1 << DIO);

    _delay_us(bit_delay);
    DDRD |= (1 << CLK);
    _delay_us(bit_delay);
}

void TM1637::setSegments(const uint8_t *segments, uint8_t length, uint8_t pos)
{
    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1);
    stop();

    // Write COMM2 + first digit address
    start();
    writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

    // Write the data bytes
    for (uint8_t k = 0; k < length; k++)
        writeByte(segments[k]);

    stop();

    // Write COMM3 + brightness
    start();
    writeByte(TM1637_I2C_COMM3 + (brightness & 0x0f));
    stop();
}

void TM1637::clear()
{
    uint8_t data[] = {0, 0, 0, 0};
    setSegments(data, 4, 0);
}

uint16_t TM1637::countDigits(uint64_t num)
{
    if (num / 10 == 0)
    {
        return 1;
    }
    return 1 + countDigits(num / 10);
}

void TM1637::printNum(uint16_t num)
{
    uint8_t digits = countDigits(num);
    char buffer[digits + 1];
    itoa(num, buffer, 10);
    uint8_t segments[4] = {0, 0, 0, 0};

    for (uint8_t i = 0; i < digits; i++)
    {
        segments[3 - i - NUM_OFFSET] = num_2_digit[buffer[digits - 1 - i] - '0'];
    }
    setSegments(segments, 4, 0);
}

void TM1637::printNumChar(const char *str, uint8_t digits)
{
    uint8_t segments[4] = {0, 0, 0, 0};

    for (uint8_t i = 0; i < digits; i++)
    {
        segments[3 - i - NUM_OFFSET] = num_2_digit[str[digits - 1 - i] - '0'];
    }
    setSegments(segments, 4, 0);
}

void TM1637::printInit()
{
    uint8_t segments[] = {0b00110000, 0b01010100, 0b00110000, 0b01111000};
    setSegments(segments, 4, 0);
}

void TM1637::printMute()
{
    uint8_t segments[] = {0b00110111, 0b00011100, 0b01111000, 0b01111001};
    setSegments(segments, 4, 0);
}
