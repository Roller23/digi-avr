#if !defined(__AVR_ATmega328P__)
  #define __AVR_ATmega328P__
#endif

#include <stdio.h>
#include <util/delay.h>

int main(void) {
  _delay_ms(10);
  // printf("Hello\n");
  asm("BREAK;"); // the only way to stop the MCU
  return 0;
}