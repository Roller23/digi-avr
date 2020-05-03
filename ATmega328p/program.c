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

void _delay_s(int s) {
  for (int i = 0; i < s; i++) {
    for (int j = 0; j < 10; j++) {
      _delay_ms(100);
    }
  }
}

int main(void) {
  // _delay_s(5);
  sleep_cpu();
  ASM("BREAK"); // stop the MCU
  return 0;
}