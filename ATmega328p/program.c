#if !defined(__AVR_ATmega328P__)
  #define __AVR_ATmega328P__
#endif

#define F_CPU 1000000UL

#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define ASM(_asm) do { asm volatile(_asm); } while(0)

// Do nothing for all unhandled interrupts
EMPTY_INTERRUPT(BADISR_vect);

int main(void) {
  sleep_enable();
  sleep_cpu();
  ASM("BREAK"); // stop the MCU
  return 0;
}