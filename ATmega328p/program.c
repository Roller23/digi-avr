#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>

ISR(ADC_vect) {
  _delay_us(1);
}

int main(void) {
  _delay_us(1);
  return 0;
}