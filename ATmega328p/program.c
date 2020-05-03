#if !defined(__AVR_ATmega328P__)
  #define __AVR_ATmega328P__
#endif

#define F_CPU 1000000UL

#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <assert.h>

#define ASM(_asm) do { asm volatile(_asm); } while(0)

ISR(INT0_vect) {
  wdt_reset();
}

int main(void) {
  wdt_enable(WDTO_500MS);
  sleep_enable();
  sleep_cpu();
  ASM("BREAK"); // stop the MCU
  return 0;
}