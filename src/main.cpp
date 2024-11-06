#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>

void setup() {
  // put your setup code here, to run once:
  DDRB = 0b00100000;
  PORTB = 0b00000000;
}

void loop() {
  PORTB = 0b00100000;
  _delay_ms(200);
  PORTB = 0b00000000;
  _delay_ms(200);
}

