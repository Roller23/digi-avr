#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Do nothing for all unhandled interrupts
EMPTY_INTERRUPT(BADISR_vect);

ISR(ADC_vect) {
  _delay_us(1);
}

int main(void) {
  _delay_us(1);
  return 0;
}