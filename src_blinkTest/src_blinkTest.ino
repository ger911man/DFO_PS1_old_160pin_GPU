//  Attiny13 pinout:
//        ,___,
// (PB5) 1|    |8  VCC (5V)
// (PB3) 2|    |7  PB2 (SCK/SCL/Analog 2)
// (PB4) 3|    |6  PB1 (MISO/PWM/Analog 1) (INT0)
//   GND 4|    |5  PB0 (MOSI/PWM/SDA/Analog 0)

//#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 9600000L

//#define PIN_LED PB3             // led for debugging purposes
#define PIN_LED 0b00000011        // led for debugging purposes

int main() {
  // -----------------------------=[ setup ]=-----------------------------------
  DDRB |= (1 << PIN_LED);              // set PIN_LED as output

  // ------------------------------=[ loop ]=-----------------------------------
  while (true) {
    PINB = (1 << PIN_LED);
    _delay_ms(1000);
  }
  return 0;
}