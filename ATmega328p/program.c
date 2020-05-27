#if !defined(__AVR_ATmega328P__)
  #define __AVR_ATmega328P__
#endif

// default F_CPU is 1MHz
#define Hz (1UL)
#define KHz (Hz * 1000UL)
#define MHz (KHz * 1000UL)
#define F_CPU (KHz)
#define ASM(_asm) do { asm volatile(_asm); } while(0)

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

ISR(INT0_vect) {
  sleep_cpu();
}

ISR(INT1_vect) {
  wdt_reset();
}

int main(void) {
  sleep_cpu();
  ASM("BREAK");
  return 0;
}