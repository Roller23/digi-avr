#if !defined(__AVR_ATmega328P__)
  #define __AVR_ATmega328P__
#endif

// default F_CPU is 1MHz
#define Hz (1UL)
#define KHz (Hz * 1000UL)
#define MHz (KHz * 1000UL)
#define F_CPU (150 * Hz)
#define ASM(_asm) do { asm volatile(_asm); } while(0)

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdbool.h>

#define R ((volatile unsigned char *)0x0)
#define IO ((volatile unsigned char *)0x20)
#define EXT_IO ((volatile unsigned char *)0x60)
#define SRAM ((volatile unsigned char *)0x100)

inline void _putchar(int c)  {
  EXT_IO[1] = c;
}

inline void _puts(const char *string) {
	int i = 0;
	while (string[i]) {
		_putchar(string[i]);
		i++;
	}
}

int main(void) {
	_puts("Hello world\n");
	unsigned char value = 0;
	while (true) {
		EXT_IO[0] = value;
		value++;
		_delay_ms(50);
	}
  return 0;
}