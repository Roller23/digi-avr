#include "atmega328p.h"
#include "instructions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/stat.h>

#if defined(SHARED)

  #pragma GCC diagnostic ignored "-Wformat-zero-length"

  #define CYAN ""
  #define RED ""
  #define RESET ""

#else

  #define CYAN "\033[36m"
  #define RED "\x1B[1;31m"
  #define RESET "\x1B[0m"

#endif

#define DEBUG_MODE 1
#define BUFFER_LENGTH 1024

static inline int print(const char *format, ...) {
  int a = 0;
  #if DEBUG_MODE == 1
    va_list args;
    va_start(args, format);
    a += printf(CYAN "[Debug] ");
    a += vfprintf(stdout, format, args);
    a += printf(RESET);
    va_end(args);
  #endif
  return a;
}

#define B_GET(number, n) ((number) & (1LLU << (n)))
#define MS 1000
#define SEC (MS * 1000)
#define Hz (1UL)
#define KHz (Hz * 1000UL)
#define MHz (KHz * 1000UL)
#define CLOCK_SPEED (KHz)
#define CLOCK_FREQ (SEC / CLOCK_SPEED)
#define TMP "./tmp/"

typedef struct {
  int16_t number : 12;
} int12_t;

typedef struct {
  int8_t number : 7;
} int7_t;

static inline void print_bits(uint32_t number) {
  char bits[35];
  memset(bits, 0, 35);
  for (int i = sizeof(number) * 8 - 1, j = 0; i >= 0; i--, j++) {
    bits[j] = B_GET(number, i) ? '1' : '0';
    if (i == 16) {
      bits[j + 1] = '\n';
      j++;
    }
  }
  print("0x%.8X bits\n%s\n", number, bits);
}

static ATmega328p_t mcu;
static const Instruction_t *opcode_lookup[LOOKUP_SIZE];

static inline void ADD(const uint32_t opcode) {
  // 0000 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] + mcu.R[reg_r];
  mcu.SREG.flags.H = !!(B_GET(mcu.R[reg_d], 3) & B_GET(mcu.R[reg_r], 3) | B_GET(mcu.R[reg_r], 3) & ~B_GET(result, 3) | ~B_GET(result, 3) & B_GET(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) & ~B_GET(result, 7) | ~B_GET(mcu.R[reg_d], 7) & ~B_GET(mcu.R[reg_r], 7) & B_GET(result, 7));
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) | B_GET(mcu.R[reg_r], 7) & ~B_GET(result, 7) | ~B_GET(result, 7) & B_GET(mcu.R[reg_d], 7));
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void ADC(const uint32_t opcode) {
  // 0001 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] + mcu.R[reg_r] + mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(B_GET(mcu.R[reg_d], 3) & B_GET(mcu.R[reg_r], 3) | B_GET(mcu.R[reg_r], 3) & ~B_GET(result, 3) | ~B_GET(result, 3) & B_GET(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) & ~B_GET(result, 7) | ~B_GET(mcu.R[reg_d], 7) & ~B_GET(mcu.R[reg_r], 7) & B_GET(result, 7));
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) | B_GET(mcu.R[reg_r], 7) & ~B_GET(result, 7) | ~B_GET(result, 7) & B_GET(mcu.R[reg_d], 7));
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void ADIW(const uint32_t opcode) {
  // 1001 0110 KKdd KKKK
  uint8_t k = (opcode & 0xF);
  k |= B_GET(opcode, 6) >> 2;
  k |= B_GET(opcode, 7) >> 2;
  uint8_t reg_d = ((!!B_GET(opcode, 5)) << 1) | (!!B_GET(opcode, 4));
  reg_d = reg_d * 2 + 24;
  uint16_t rd = word_reg_get(reg_d);
  uint16_t result = rd + k;
  mcu.SREG.flags.V = !B_GET(rd, 15) & !!B_GET(result, 15);
  mcu.SREG.flags.N = !!B_GET(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !B_GET(result, 15) & !!B_GET(rd, 15);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  word_reg_set(reg_d, result);
  mcu.pc += 1;
}

static inline void SUB(const uint32_t opcode) {
  // 0001 10rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] - mcu.R[reg_r];
  mcu.SREG.flags.H = !!(~B_GET(mcu.R[reg_d], 3) & B_GET(mcu.R[reg_r], 3) | B_GET(mcu.R[reg_r], 3) & B_GET(result, 3) | B_GET(result, 3) & ~B_GET(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(B_GET(mcu.R[reg_d], 7) & ~B_GET(mcu.R[reg_r], 7) & ~B_GET(result, 7) | ~B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) & B_GET(result, 7));
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(~B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) | B_GET(mcu.R[reg_r], 7) & B_GET(result, 7) | B_GET(result, 7) & ~B_GET(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void SUBI(const uint32_t opcode) {
  // 0101 kkkk dddd kkkk
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t k = (opcode & 0xF) | ((opcode & 0xF00) >> 4);
  uint8_t result = mcu.R[reg_d] - k;
  mcu.SREG.flags.H = !!(~B_GET(mcu.R[reg_d], 3) & B_GET(k, 3) | B_GET(k, 3) & B_GET(result, 3) | B_GET(result, 3) & ~B_GET(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(B_GET(mcu.R[reg_d], 7) & ~B_GET(k, 7) & ~B_GET(result, 7) | ~B_GET(mcu.R[reg_d], 7) & B_GET(k, 7) & B_GET(result, 7));
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(~B_GET(mcu.R[reg_d], 7) & B_GET(k, 7) | B_GET(k, 7) & B_GET(result, 7) | B_GET(result, 7) & ~B_GET(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void SBC(const uint32_t opcode) {
  // 0000 10rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] - mcu.R[reg_r] - mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(~B_GET(mcu.R[reg_d], 3) & B_GET(mcu.R[reg_r], 3) | B_GET(mcu.R[reg_r], 3) & B_GET(result, 3) | B_GET(result, 3) & ~B_GET(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(B_GET(mcu.R[reg_d], 7) & ~B_GET(mcu.R[reg_r], 7) & ~B_GET(result, 7) | ~B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) & B_GET(result, 7));
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0) & mcu.SREG.flags.Z;
  mcu.SREG.flags.C = !!(~B_GET(mcu.R[reg_d], 7) & B_GET(mcu.R[reg_r], 7) | B_GET(mcu.R[reg_r], 7) & B_GET(result, 7) | B_GET(result, 7) & ~B_GET(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void SBCI(const uint32_t opcode) {
  // 0100 kkkk dddd kkkk
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t k = (opcode & 0xF) | ((opcode & 0xF00) >> 4);
  uint8_t result = mcu.R[reg_d] - k - mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(~B_GET(mcu.R[reg_d], 3) & B_GET(k, 3) | B_GET(k, 3) & B_GET(result, 3) | B_GET(result, 3) & ~B_GET(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(B_GET(mcu.R[reg_d], 7) & ~B_GET(k, 7) & ~B_GET(result, 7) | ~B_GET(mcu.R[reg_d], 7) & B_GET(k, 7) & B_GET(result, 7));
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0) & mcu.SREG.flags.Z;
  mcu.SREG.flags.C = !!(~B_GET(mcu.R[reg_d], 7) & B_GET(k, 7) | B_GET(k, 7) & B_GET(result, 7) | B_GET(result, 7) & ~B_GET(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void SBIW(const uint32_t opcode) {
  // 1001 0111 KKdd KKKK
  uint8_t k = (opcode & 0xF);
  k |= B_GET(opcode, 6) >> 2;
  k |= B_GET(opcode, 7) >> 2;
  uint8_t reg_d = ((!!B_GET(opcode, 5)) << 1) | (!!B_GET(opcode, 4));
  reg_d = reg_d * 2 + 24;
  uint16_t rd = word_reg_get(reg_d);
  uint16_t result = rd - k;
  mcu.SREG.flags.V = !!B_GET(result, 15) & !B_GET(rd, 15);
  mcu.SREG.flags.N = !!B_GET(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!B_GET(result, 15) & !B_GET(rd, 15);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  word_reg_set(reg_d, result);
  mcu.pc += 1;
}

static inline void AND(const uint32_t opcode) {
  // 0010 00rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] & mcu.R[reg_r];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void ANDI(const uint32_t opcode) {
  // 0111 KKKK dddd KKKK
  uint8_t k = (opcode & 0xF);
  k |= (opcode & 0xF00) >> 4;
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint16_t result = mcu.R[reg_d] & k;
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void OR(const uint32_t opcode) {
  // 0010 10rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] | mcu.R[reg_r];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void ORI(const uint32_t opcode) {
  // 0110 KKKK dddd KKKK
  uint8_t k = (opcode & 0xF);
  k |= (opcode & 0xF00) >> 4;
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint16_t result = mcu.R[reg_d] | k;
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void EOR(const uint32_t opcode) {
  // 0010 01rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] ^ mcu.R[reg_r];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void COM(const uint32_t opcode) {
  // 1001 010d dddd 0000
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t result = 0xFF - mcu.R[reg_d];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = 1;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void NEG(const uint32_t opcode) {
  // 1001 010d dddd 0001
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t result = 0x00 - mcu.R[reg_d];
  mcu.SREG.flags.H = !!B_GET(result, 3) | !B_GET(mcu.R[reg_d], 3); //TD: check H
  mcu.SREG.flags.V = !!B_GET(result, 7) & ((result & 0b01111111) == 0);
  mcu.SREG.flags.N = !!B_GET(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = (result != 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void INC(const uint32_t opcode) {
  // 1001 010d dddd 0011
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  mcu.R[reg_d] = mcu.R[reg_d] + 1;
  mcu.SREG.flags.V = !!B_GET(mcu.R[reg_d], 7) & ((mcu.R[reg_d] & 0b01111111) == 0);
  mcu.SREG.flags.N = !!B_GET(mcu.R[reg_d], 7);
  mcu.SREG.flags.Z = (mcu.R[reg_d] == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void DEC(const uint32_t opcode) {
  // 1001 010d dddd 1010
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  mcu.R[reg_d] = mcu.R[reg_d] - 1;
  mcu.SREG.flags.V = !B_GET(mcu.R[reg_d], 7) & ((mcu.R[reg_d] & 0b01111111) == 0b01111111);
  mcu.SREG.flags.N = !!B_GET(mcu.R[reg_d], 7);
  mcu.SREG.flags.Z = (mcu.R[reg_d] == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void SER(const uint32_t opcode) {
  // 1110 1111 dddd 1111
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  mcu.R[reg_d] = 0xFF;
  mcu.pc += 1;
}

static inline void MUL(const uint32_t opcode) {
  // 1001 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  uint16_t result = mcu.R[reg_d] * mcu.R[reg_r];
  mcu.SREG.flags.C = !!B_GET(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  word_reg_set(0, result);
  mcu.pc += 1;
}

static inline void MULS(const uint32_t opcode) {
  // 0000 0010 dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0xF);
  reg_r += 16;
  int16_t result = (int8_t)mcu.R[reg_d] * (int8_t)mcu.R[reg_r];
  mcu.SREG.flags.C = !!B_GET(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  word_reg_set(0, (uint16_t)result);
  mcu.pc += 1;
}

static inline void MULSU(const uint32_t opcode) {
  // 0000 0011 0ddd 0rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  int16_t result = (int8_t)mcu.R[reg_d] * mcu.R[reg_r];
  mcu.SREG.flags.C = !!B_GET(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  word_reg_set(0, (uint16_t)result);
  mcu.pc += 1;
}

static inline void FMUL(const uint32_t opcode) {
  // 0000 0011 0ddd 1rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  double d = (mcu.R[reg_d] / (double)(1 << 7));
  double r = (mcu.R[reg_r] / (double)(1 << 7));
  double res = d * r;
  uint16_t result = round(res * (1 << 14));
  mcu.SREG.flags.C = B_GET(result, 15) >> 15;
  mcu.SREG.flags.Z = (result == 0);
  result <<= 1;
  word_reg_set(0, result);
  mcu.pc += 1;
}

static inline void FMULS(const uint32_t opcode) {
  // 0000 0011 1ddd 0rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  double d = ((int8_t)mcu.R[reg_d] / (double)(1 << 7));
  double r = ((int8_t)mcu.R[reg_r] / (double)(1 << 7));
  double res = d * r;
  uint16_t result = round(res * (1 << 14));
  mcu.SREG.flags.C = B_GET(result, 15) >> 15;
  mcu.SREG.flags.Z = (result == 0);
  result <<= 1;
  word_reg_set(0, result);
  mcu.pc += 1;
}

static inline void FMULSU(const uint32_t opcode) {
  // 0000 0011 1ddd 1rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  double d = ((int8_t)mcu.R[reg_d] / (double)(1 << 7));
  double r = (mcu.R[reg_r] / (double)(1 << 7));
  double res = d * r;
  uint16_t result = round(res * (1 << 14));
  mcu.SREG.flags.C = B_GET(result, 15) >> 15;
  mcu.SREG.flags.Z = (result == 0);
  result <<= 1;
  word_reg_set(0, result);
  mcu.pc += 1;
}

static inline void MOV(const uint32_t opcode) {
  // 0010 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= B_GET(opcode, 9) >> 5;
  mcu.R[reg_d] = mcu.R[reg_r];
  mcu.pc += 1;  
}

static inline void MOVW(const uint32_t opcode) {
  // 0000 0001 dddd rrrr
  uint8_t reg_d = ((opcode & 0xF0) >> 4) * 2;
  uint8_t reg_r = (opcode & 0xF) * 2;
  word_reg_set(reg_d, word_reg_get(reg_r));
  mcu.pc += 1;
}

static inline void LDI(const uint32_t opcode) {
  // 1110 kkkk dddd kkkk
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t k = (opcode & 0xF);
  k |= (opcode & 0xF00) >> 4;
  mcu.R[reg_d] = k;
  mcu.pc += 1;  
}

static inline void ST_X(const uint32_t opcode) {
  // (i)   1001 001r rrrr 1100
  // (ii)  1001 001r rrrr 1101
  // (iii) 1001 001r rrrr 1110
  uint8_t r = (opcode & 0b111110000) >> 4;
  uint8_t version = opcode & 0b11;
  uint16_t X = X_reg_get();
  if (version == 0) {
    // X unchanged
    mcu.data_memory[X] = mcu.R[r];
  } else if (version == 1) {
    // X post incremented
    mcu.data_memory[X] = mcu.R[r];
    X_reg_set(X + 1);
  } else {
    // X Pre decremented
    X_reg_set(X - 1);
    mcu.data_memory[X - 1] = mcu.R[r];
  }
  mcu.pc += 1;
}

static inline void ST_Y(const uint32_t opcode) {
  // (i)   1000 001r rrrr 1000
  // (ii)  1001 001r rrrr 1001
  // (iii) 1001 001r rrrr 1010
  // (iv)  10q0 qq1r rrrr 1qqq
  uint8_t version = opcode & 0b11;
  uint8_t r = (opcode & 0b111110000) >> 4;
  uint8_t q = (opcode & 0b111) | ((opcode & 0b110000000000) >> 7);
  q |= B_GET(opcode, 13) >> 8;
  uint16_t Y = Y_reg_get();
  if (q > 2) {
    // with q displacement
    mcu.data_memory[Y + q] = mcu.R[r];
  } else if (version == 0) {
    // Y unchanged
    mcu.data_memory[Y] = mcu.R[r];
  } else if (version == 1) {
    // Y post incremented
    mcu.data_memory[Y] = mcu.R[r];
    Y_reg_set(Y + 1);
  } else {
    // Y pre decremented
    Y_reg_set(Y - 1);
    mcu.data_memory[Y - 1] = mcu.R[r];
  }
  mcu.pc += 1;
}

static inline void ST_Z(const uint32_t opcode) {
  // (i)   1000 001r rrrr 0000
  // (ii)  1001 001r rrrr 0001
  // (iii) 1001 001r rrrr 0010
  // (iv)  10q0 qq1r rrrr 0qqq
  uint8_t version = opcode & 0b11;
  uint8_t r = (opcode & 0b111110000) >> 4;
  uint8_t q = (opcode & 0b111) | ((opcode & 0b110000000000) >> 7);
  q |= B_GET(opcode, 13) >> 8;
  uint16_t Z = Z_reg_get();
  if (q > 2) {
    // with q displacement
    mcu.data_memory[Z + q] = mcu.R[r];
  } else if (version == 0) {
    // Y unchanged
    mcu.data_memory[Z] = mcu.R[r];
  } else if (version == 1) {
    // Y post incremented
    mcu.data_memory[Z] = mcu.R[r];
    Z_reg_set(Z + 1);
  } else {
    // Y pre decremented
    Z_reg_set(Z - 1);
    mcu.data_memory[Z - 1] = mcu.R[r];
  }
  mcu.pc += 1;
}

static inline void STS(const uint32_t opcode) {
  // 1001 001d dddd 0000
  // kkkk kkkk kkkk kkkk
  uint16_t k = opcode & 0xFFFF;
  uint8_t d = ((opcode & 0xF00000) | B_GET(opcode, 24)) >> 20;
  mcu.data_memory[k] = mcu.R[d];
  mcu.data_memory_change = (int16_t)k;
  mcu.pc += 2;
}

static inline void LPM(const uint32_t opcode) {
  // (i)   1001 0101 1100 1000
  // (ii)  1001 000d dddd 0100
  // (iii) 1001 000d dddd 0101
  uint8_t version = opcode & 0x000F;
  uint8_t d = (opcode & 0b111110000) >> 4;
  uint16_t Z = Z_reg_get();
  if (version == 8) {
    // R0 implied
    mcu.R[0] = mcu.program_memory[Z];
  } else if (version == 4) {
    // Z unchanged
    mcu.R[d] = mcu.program_memory[Z];
  } else {
    // Z post incremented
    mcu.R[d] = mcu.program_memory[Z];
    Z_reg_set(Z + 1);
  }
  mcu.pc += 1;
}

static inline void LD_X(const uint32_t opcode) {
  // (i)   1001 000d dddd 1100
  // (ii)  1001 000d dddd 1101
  // (iii) 1001 000d dddd 1110
  uint8_t d = (opcode & 0b111110000) >> 4;
  uint8_t version = opcode & 0b11;
  uint16_t X = X_reg_get();
  if (version == 0) {
    // X unchanged
    mcu.R[d] = mcu.data_memory[X];
  } else if (version == 1) {
    // X post incremented
    mcu.R[d] = mcu.data_memory[X];
    X_reg_set(X + 1);
  } else {
    // X Pre decremented
    X_reg_set(X - 1);
    mcu.R[d] = mcu.data_memory[X - 1];
  }
  mcu.pc += 1;
}

static inline void LD_Y(const uint32_t opcode) {
  // (i)   1000 000d dddd 1000
  // (ii)  1001 000d dddd 1001
  // (iii) 1001 000d dddd 1010
  // (iv)  10q0 qq0d dddd 1qqq
  uint8_t version = opcode & 0b11;
  uint8_t d = (opcode & 0b111110000) >> 4;
  uint8_t q = (opcode & 0b111) | ((opcode & 0b110000000000) >> 7);
  q |= B_GET(opcode, 13) >> 8;
  uint16_t Y = Y_reg_get();
  if (q > 2) {
    // with q displacement
    mcu.R[d] = mcu.data_memory[Y + q];
  } else if (version == 0) {
    // Y unchanged
    mcu.R[d] = mcu.data_memory[Y];
  } else if (version == 1) {
    // Y post incremented
    mcu.R[d] = mcu.data_memory[Y];
    Y_reg_set(Y + 1);
  } else {
    // Y pre decremented
    Y_reg_set(Y - 1);
    mcu.R[d] = mcu.data_memory[Y - 1];
  }
  mcu.pc += 1;
}

static inline void LD_Z(const uint32_t opcode) {
  // (i)   1000 001d dddd 0000
  // (ii)  1001 001d dddd 0001
  // (iii) 1001 001d dddd 0010
  // (iv)  10q0 qq1d dddd 0qqq
  uint8_t version = opcode & 0b11;
  uint8_t d = (opcode & 0b111110000) >> 4;
  uint8_t q = (opcode & 0b111) | ((opcode & 0b110000000000) >> 7);
  q |= B_GET(opcode, 13) >> 8;
  uint16_t Z = Z_reg_get();
  if (q > 2) {
    // with q displacement
    mcu.R[d] = mcu.data_memory[Z + q];
  } else if (version == 0) {
    // Y unchanged
    mcu.R[d] = mcu.data_memory[Z];
  } else if (version == 1) {
    // Y post incremented
    mcu.R[d] = mcu.data_memory[Z];
    Z_reg_set(Z + 1);
  } else {
    // Y pre decremented
    Z_reg_set(Z - 1);
    mcu.R[d] = mcu.data_memory[Z - 1];
  }
  mcu.pc += 1;
}

static inline void LDS(const uint32_t opcode) {
  // 1001 000d dddd 0000
  // kkkk kkkk kkkk kkkk
  uint16_t k = opcode & 0xFFFF;
  uint8_t d = ((opcode & 0xF00000) | B_GET(opcode, 24)) >> 20;
  mcu.R[d] = mcu.data_memory[k];
  mcu.pc += 2;
}

static inline void SPM(const uint32_t opcode) {
  // 1001 0101 1110 1000
  *((uint16_t *)(mcu.program_memory + Z_reg_get())) = word_reg_get(0);
  mcu.pc += 1;
}
static inline void IN(const uint32_t opcode) {
  // 1011 0AAd dddd AAAA
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  uint8_t a = (opcode & 0xF) | ((opcode & 0x600) >> 5);
  mcu.R[reg_d] = mcu.IO[a];
  mcu.pc += 1;
}

static inline void OUT(const uint32_t opcode) {
  // 1011 1AAr rrrr AAAA
  uint8_t reg_r = (opcode & 0b111110000) >> 4;
  uint8_t a = (opcode & 0xF) | ((opcode & 0b11000000000) >> 5);
  mcu.IO[a] = mcu.R[reg_r];
  mcu.pc += 1;
}

static inline void PUSH(const uint32_t opcode) {
  // 1001 001d dddd 1111
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  stack_push8(mcu.R[reg_d]);
  mcu.pc += 1; 
}

static inline void POP(const uint32_t opcode) {
  // 1001 000d dddd 1111
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= B_GET(opcode, 8) >> 4;
  mcu.R[reg_d] = stack_pop8();
  mcu.pc += 1; 
}

static inline void RJMP(const uint32_t opcode) {
  // 1100 kkkk kkkk kkkk
  // Relative jump to PC + k + 1
  int12_t k = {.number = (opcode & 0x0FFF)};
  mcu.pc += k.number + 1;
}

static inline void IJMP(const uint32_t opcode) {
  // Indirect jump to address at Z register
  mcu.pc = Z_reg_get();
}

static inline void JMP(const uint32_t opcode) {
  // 1001 010k kkkk 110k
  // kkkk kkkk kkkk kkkk
  // Jump to address k, PC = k
  uint32_t k = opcode & 0b11111111111111111;
  k |= (opcode & 0xF00000) >> 3;
  k |= B_GET(opcode, 24) >> 3;
  mcu.pc = k;
}

static inline void RCALL(const uint32_t opcode) {
  // 1101 kkkk kkkk kkkk
  // Jump to address + 1 + PC, push current PC + 1 onto stack (relative call)
  uint16_t k = opcode & 0xFFF;
  stack_push16(mcu.pc + 1);
  mcu.pc += k + 1;
}

static inline void ICALL(const uint32_t opcode) {
  // Indirect call, PC = Z, push PC + 1 to stack
  stack_push16(mcu.pc + 1);
  mcu.pc = Z_reg_get();
}

static inline void CALL(const uint32_t opcode) {
  // 1001 010k kkkk 111k
  // kkkk kkkk kkkk kkkk
  // Long call, push PC + 2 to stack, PC = k
  uint32_t k = opcode & 0xFFFF;
  k |= B_GET(opcode, 16);
  k |= (opcode & 0xF00000) >> 3;
  k |= B_GET(opcode, 24) >> 3;
  stack_push16(mcu.pc + 2);
  mcu.pc = k;
}

static inline void RET(const uint32_t opcode) {
  // Return from subroutine, PC = stack
  mcu.pc = stack_pop16();
}

static inline void RETI(const uint32_t opcode) {
  // Return from interrupt and set I to 1
  mcu.pc = stack_pop16();
  mcu.SREG.flags.I = 1;
}

static inline void CPSE(const uint32_t opcode) {
  // 0001 00rd dddd rrrr
  // Compare, skip if equal
  uint16_t r = (opcode & 0xF) | (B_GET(opcode, 9) >> 5);
  uint16_t d = (opcode & 0b111110000) >> 4;
  if (mcu.R[r] == mcu.R[d]) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void CP(const uint32_t opcode) {
  // 0001 01rd dddd rrrr
  // Compare two registers
  uint16_t r = (opcode & 0xF) | (B_GET(opcode, 9) >> 5);
  uint16_t d = (opcode & 0b111110000) >> 4;
  byte *R = mcu.R;
  uint8_t res = R[d] - R[r];
  mcu.SREG.flags.H = !B_GET(R[d], 3) && B_GET(R[r], 3) || B_GET(R[r], 3) && B_GET(res, 3) || B_GET(res, 3) && !B_GET(R[d], 3);
  mcu.SREG.flags.V = B_GET(R[d], 7) && !B_GET(R[r], 7) && B_GET(res, 7) || !B_GET(R[d], 7) && B_GET(R[r], 7) && B_GET(res, 7);
  mcu.SREG.flags.N = !!B_GET(res, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (res == 0);
  mcu.SREG.flags.C = !B_GET(R[d], 7) && B_GET(R[r], 7) || B_GET(R[r], 7) && B_GET(res, 7) || B_GET(res, 7) && !B_GET(R[d], 7);
  mcu.pc += 1;
}

static inline void CPC(const uint32_t opcode) {
  // 0000 01rd dddd rrrr
  // Compare with carry
  uint16_t r = (opcode & 0xF) | (B_GET(opcode, 9) >> 5);
  uint16_t d = (opcode & 0b111110000) >> 4;
  byte *R = mcu.R;
  uint8_t res = R[d] - R[r] - mcu.SREG.flags.C;
  mcu.SREG.flags.H = !B_GET(R[d], 3) && B_GET(R[r], 3) || B_GET(R[r], 3) && B_GET(res, 3) || B_GET(res, 3) && !B_GET(R[d], 3);
  mcu.SREG.flags.V = B_GET(R[d], 7) && !B_GET(R[r], 7) && B_GET(res, 7) || !B_GET(R[d], 7) && B_GET(R[r], 7) && B_GET(res, 7);
  mcu.SREG.flags.N = !!B_GET(res, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.C = !B_GET(R[d], 7) && B_GET(R[r], 7) || B_GET(R[r], 7) && B_GET(res, 7) || B_GET(res, 7) && !B_GET(R[d], 7);
  mcu.SREG.flags.Z = (res == 0) && mcu.SREG.flags.Z;
  mcu.pc += 1;
}

static inline void CPI(const uint32_t opcode) {
  // 0011 KKKK dddd KKKK
  // Compare with immediate
  uint16_t k = (opcode & 0xF) | ((opcode & 0xF00) >> 4);
  uint16_t d = ((opcode & 0xF0) >> 4) + 16;
  byte *R = mcu.R;
  uint8_t res = R[d] - k;
  mcu.SREG.flags.H = !B_GET(R[d], 3) && B_GET(k, 3) || B_GET(k, 3) && B_GET(res, 3) || B_GET(res, 3) && !B_GET(R[d], 3);
  mcu.SREG.flags.V = B_GET(R[d], 7) && !B_GET(k, 7) && !B_GET(res, 7) || !B_GET(R[d], 7) && B_GET(k, 7) && B_GET(res, 7);
  mcu.SREG.flags.N = !!B_GET(res, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (res == 0);
  mcu.SREG.flags.C = !B_GET(R[d], 7) && B_GET(k, 7) || B_GET(k, 7) && B_GET(res, 7) || B_GET(res, 7) && !B_GET(R[d], 7);
  mcu.pc += 1;
}

static inline void SBRC(const uint32_t opcode) {
  // 1111 110r rrrr 0bbb
  // Skip if R[r](b) is cleared
  uint8_t b = (opcode & 0b111);
  uint8_t r = (opcode & 0b111110000) >> 4;
  if (!B_GET(mcu.R[r], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void SBRS(const uint32_t opcode) {
  // 1111 111r rrrr 0bbb
  // Skip if R[r](b) is set
  uint8_t b = (opcode & 0b111);
  uint8_t r = (opcode & 0b111110000) >> 4;
  if (B_GET(mcu.R[r], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void SBIC(const uint32_t opcode) {
  // 1001 1001 AAAA Abbb
  // Skip if I/O[A](b) is cleared
  uint8_t b = (opcode & 0b111);
  uint8_t A = (opcode & 0b11111000) >> 3;
  if (!B_GET(mcu.IO[A], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void SBIS(const uint32_t opcode) {
  // 1001 1011 AAAA Abbb
  // Skip if I/O[A](b) is set
  uint8_t b = (opcode & 0b111);
  uint8_t A = (opcode & 0b11111000) >> 3;
  if (B_GET(mcu.IO[A], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void BRBS(const uint32_t opcode) {
  // 1111 00kk kkkk ksss
  // Branch if SREG(s) is set (PC += k + 1), k is in U2
  uint8_t s = opcode & 0b111;
  int7_t k = {.number = ((opcode & 0b1111111000) >> 3)};
  if (B_GET(mcu.SREG.value, s)) {
    mcu.pc += k.number + 1;
    return;
  }
  mcu.pc += 1;
}

static inline void BRBC(const uint32_t opcode) {
  // 1111 01kk kkkk ksss
  // Branch if SREG(s) is cleared (PC += k + 1), k is in U2
  uint8_t s = opcode & 0b111;
  int7_t k = {.number = ((opcode & 0b1111111000) >> 3)};
  if (!B_GET(mcu.SREG.value, s)) {
    mcu.pc += k.number + 1;
    return;
  }
  mcu.pc += 1;
}

static inline void SBI(const uint32_t opcode) {
  // 1001 1010 AAAA Abbb
  // Set I/O[A](b)
  uint8_t b = opcode & 0b111;
  uint8_t A = (opcode & 0b11111000) >> 3;
  mcu.IO[A] |= (1 << b);
  mcu.pc += 1;
}

static inline void CBI(const uint32_t opcode) {
  // 1001 1000 AAAA Abbb
  // Clear I/O[A](b)
  uint8_t b = opcode & 0b111;
  uint8_t A = (opcode & 0b11111000) >> 3;
  mcu.IO[A] &= ~(1 << b);
  mcu.pc += 1;
}

static inline void LSR(const uint32_t opcode) {
  // 1001 010d dddd 0110
  // C = R[d](0), R[d] >> 1
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.SREG.flags.C = !!B_GET(mcu.R[d], 0);
  mcu.R[d] >>= 1;
  mcu.SREG.flags.Z = (mcu.R[d] == 0);
  mcu.SREG.flags.N = 0;
  mcu.SREG.flags.V = mcu.SREG.flags.N ^ mcu.SREG.flags.C;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void ROR(const uint32_t opcode) {
  // 1001 010d dddd 0111
  // C = R[d](0), R[d] >> 1, R[d](7) = C
  uint8_t d = (opcode & 0b111110000) >> 4;
  bit carry = !!mcu.SREG.flags.C;
  mcu.SREG.flags.C = !!B_GET(mcu.R[d], 0);
  mcu.R[d] >>= 1;
  mcu.R[d] |= (carry << 7);
  mcu.SREG.flags.Z = (mcu.R[d] == 0);
  mcu.SREG.flags.N = !!B_GET(mcu.R[d], 7);
  mcu.SREG.flags.V = mcu.SREG.flags.N ^ mcu.SREG.flags.C;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void ASR(const uint32_t opcode) {
  // 1001 010d dddd 0101
  // Shift right without changing R[d](7), C = R[d](0)
  uint8_t d = (opcode & 0b111110000) >> 4;
  bit b7 = B_GET(mcu.R[d], 7);
  mcu.SREG.flags.C = !!B_GET(mcu.R[d], 0);
  mcu.R[d] >>= 1;
  mcu.R[d] |= b7;
  mcu.SREG.flags.Z = (mcu.R[d] == 0);
  mcu.SREG.flags.N = !!B_GET(mcu.R[d], 7);
  mcu.SREG.flags.V = mcu.SREG.flags.N ^ mcu.SREG.flags.C;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void SWAP(const uint32_t opcode) {
  // 1001 010d dddd 0010
  // Swap nibbles
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.R[d] = ((mcu.R[d] & 0x0F) << 4) | ((mcu.R[d] & 0xF0) >> 4);
  mcu.pc += 1;
}

static inline void BSET(const uint32_t opcode) {
  // 1001 0100 0sss 1000
  // SREG(s) = 1
  uint8_t s = (opcode & 0b1110000) >> 4;
  mcu.SREG.value |= (1 << s);
  mcu.pc += 1;
}

static inline void BCLR(const uint32_t opcode) {
  // 1001 0100 1sss 1000
  // SREG(s) = 0
  uint8_t s = (opcode & 0b1110000) >> 4;
  mcu.SREG.value &= ~(1 << s);
  mcu.pc += 1;
}

static inline void BST(const uint32_t opcode) {
  // 1111 101d dddd 0bbb
  // T = R[d](b)
  uint8_t b = opcode & 0b111;
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.SREG.flags.T = !!B_GET(mcu.R[d], b);
  mcu.pc += 1;
}

static inline void BLD(const uint32_t opcode) {
  // 1111 100d dddd 0bbb
  // R[d](b) = T
  uint8_t b = opcode & 0b111;
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.R[d] |= ((!!mcu.SREG.flags.T) << b);
  mcu.pc += 1;
}

static inline void NOP(const uint32_t opcode) {
  mcu.pc += 1;
}

static inline void SLEEP(const uint32_t opcode) {
  print("Switching to sleep mode\n");
  mcu.sleeping = true;
  mcu.pc += 1;
}

static inline void WDR(const uint32_t opcode) {
  // Reset watchdog timer
  mcu.pc += 1;
}

static inline void BREAK(const uint32_t opcode) {
  mcu.stopped = true;
}

static inline void XXX(const uint32_t opcode) {
  // Unknown opcode
  print("Unknown opcode! 0x%.4X\n", opcode);
  print_bits(opcode);
  mcu.pc += 1;
}

static const Instruction_t opcodes[] = {
  {"ADD", ADD, 0b1111110000000000, 0b0000110000000000, 1, 1},
  {"ADC", ADC, 0b1111110000000000, 0b0001110000000000, 1, 1},
  {"ADIW", ADIW, 0b1111111100000000, 0b1001011000000000, 2, 1},
  {"SUB", SUB, 0b1111110000000000, 0b0001100000000000, 1, 1},
  {"SUBI", SUBI, 0b1111000000000000, 0b0101000000000000, 1, 1},
  {"SBC", SBC, 0b1111110000000000, 0b0000100000000000, 1, 1},
  {"SBCI", SBCI, 0b1111000000000000, 0b0100000000000000, 1, 1},
  {"SBIW", SBIW, 0b1111111100000000, 0b1001011100000000, 2, 1},
  {"AND", AND, 0b1111110000000000, 0b0010000000000000, 1, 1},
  {"ANDI", ANDI, 0b1111000000000000, 0b0111000000000000, 1, 1},
  {"OR", OR, 0b1111110000000000, 0b0010100000000000, 1, 1},
  {"ORI", ORI, 0b1111000000000000, 0b0110000000000000, 1, 1},
  {"EOR", EOR, 0b1111110000000000, 0b0010010000000000, 1, 1},
  {"COM", COM, 0b1111111000001111, 0b1001010000000000, 1, 1},
  {"NEG", NEG, 0b1111111000001111, 0b1001010000000001, 1, 1},
  {"INC", INC, 0b1111111000001111, 0b1001010000000011, 1, 1},
  {"DEC", DEC, 0b1111111000001111, 0b1001010000001010, 1, 1},
  {"SER", SER, 0b1111111100001111, 0b1110111100001111, 1, 1},
  {"MUL", MUL, 0b1111110000000000, 0b1001110000000000, 2, 1},
  {"MULS", MULS, 0b1111111100000000, 0b0000001000000000, 2, 1},
  {"MULSU", MULSU, 0b1111111110001000, 0b0000001100000000, 2, 1},
  {"FMUL", FMUL, 0b1111111110001000, 0b0000001100001000, 2, 1},
  {"FMULS", FMULS, 0b1111111110001000, 0b0000001110000000, 2, 1},
  {"FMULSU", FMULSU, 0b1111111110001000, 0b0000001110001000, 2, 1},

  {"RJMP", RJMP, 0b1111000000000000, 0b1100000000000000, 2, 1},
  {"IJMP", IJMP, 0b1111111111111111, 0b1001010000001001, 2, 1},
  {"JMP", JMP, 0b1111111000001110, 0b1001010000001100, 3, 2},
  {"RCALL", RCALL, 0b1111000000000000, 0b1101000000000000, 3, 1},
  {"ICALL", ICALL, 0b1111111111111111, 0b1001010100001001, 3, 1},
  {"CALL", CALL, 0b1111111000001110, 0b1001010000001110, 4, 2},
  {"RET", RET, 0b1111111111111111, 0b1001010100001000, 4, 1},
  {"RETI", RETI, 0b1111111111111111, 0b1001010100011000, 4, 1},
  {"CPSE", CPSE, 0b1111110000000000, 0b0001000000000000, 1, 1},
  {"CP", CP, 0b1111110000000000, 0b0001010000000000, 1, 1},
  {"CPC", CPC, 0b1111110000000000, 0b0000010000000000, 1, 1},
  {"CPI", CPI, 0b1111000000000000, 0b0011000000000000, 1, 1},
  {"SBRC", SBRC, 0b1111111000001000, 0b1111110000000000, 1, 1},
  {"SBRS", SBRS, 0b1111111000001000, 0b1111111000000000, 1, 1},
  {"SBIC", SBIC, 0b1111111100000000, 0b1001100100000000, 1, 1},
  {"SBIS", SBIS, 0b1111111100000000, 0b1001101100000000, 1, 1},
  {"BRBS", BRBS, 0b1111110000000000, 0b1111000000000000, 1, 1},
  {"BRBC", BRBC, 0b1111110000000000, 0b1111010000000000, 1, 1},

  {"SBI", SBI, 0b1111111100000000, 0b1001101000000000, 2, 1},
  {"CBI", CBI, 0b1111111100000000, 0b1001100000000000, 2, 1},
  {"LSR", LSR, 0b1111111000001111, 0b1001010000000110, 1, 1},
  {"ROR", ROR, 0b1111111000001111, 0b1001010000000111, 1, 1},
  {"ASR", ASR, 0b1111111000001111, 0b1001010000000101, 1, 1},
  {"SWAP", SWAP, 0b1111111000001111, 0b1001010000000010, 1, 1},
  {"BSET", BSET, 0b1111111110001111, 0b1001010000001000, 1, 1},
  {"BCLR", BCLR, 0b1111111110001111, 0b1001010010001000, 1, 1},
  {"BST", BST, 0b1111111000001000, 0b1111101000000000, 1, 1},
  {"BLD", BLD, 0b1111111000001000, 0b1111100000000000, 1, 1},

  {"MOV", MOV, 0b1111110000000000, 0b0010110000000000, 1, 1},
  {"MOVW", MOVW, 0b1111111100000000, 0b0000000100000000, 1, 1},
  {"LDI", LDI, 0b1111000000000000, 0b1110000000000000, 1, 1},

  {"ST X", ST_X, 0b1111111000001111, 0b1001001000001100, 2, 1},
  {"ST X+", ST_X, 0b1111111000001111, 0b1001001000001101, 2, 1},
  {"ST -X", ST_X, 0b1111111000001111, 0b1001001000001110, 2, 1},

  {"ST Y", ST_Y, 0b1111111000001111, 0b1000001000001000, 2, 1},
  {"ST Y+", ST_Y, 0b1111111000001111, 0b1001001000001001, 2, 1},
  {"ST -Y", ST_Y, 0b1111111000001111, 0b1001001000001010, 2, 1},
  {"STD Y", ST_Y, 0b1101001000001000, 0b1000001000001000, 2, 1},

  {"ST Z", ST_Z, 0b1111111000001111, 0b1000001000000000, 2, 1},
  {"ST Z+", ST_Z, 0b1111111000001111, 0b1001001000000001, 2, 1},
  {"ST -Z", ST_Z, 0b1111111000001111, 0b1001001000000010, 2, 1},
  {"STD Z", ST_Z, 0b1101001000001000, 0b1000001000000000, 2, 1},

  {"STS", STS, 0b1111111000001111, 0b1001001000000000, 2, 2},

  {"LPM", LPM, 0b1111111111111111, 0b1001010111001000, 2, 1},
  {"LPM Z", LPM, 0b1111111000001111, 0b1001000000000100, 2, 1},
  {"LPM Z+", LPM, 0b1111111000001111, 0b1001000000000101, 2, 1},

  {"LD X", LD_X, 0b1111111000001111, 0b1001000000001100, 1, 1},
  {"LD X+", LD_X, 0b1111111000001111, 0b1001000000001101, 2, 1},
  {"LD -X", LD_X, 0b1111111000001111, 0b1001000000001110, 3, 1},

  {"LD Y", LD_Y, 0b1111111000001111, 0b1000000000001000, 2, 1},
  {"LD Y+", LD_Y, 0b1111111000001111, 0b1001000000001001, 2, 1},
  {"LD -Y", LD_Y, 0b1111111000001111, 0b1001000000001010, 2, 1},
  {"LDD Y", LD_Y, 0b1101001000001000, 0b1000000000001000, 2, 1},

  {"LD Z", LD_Z, 0b1111111000001111, 0b1000000000000000, 2, 1},
  {"LD Z+", LD_Z, 0b1111111000001111, 0b1001000000000001, 2, 1},
  {"LD -Z", LD_Z, 0b1111111000001111, 0b1001000000000010, 2, 1},
  {"LDD Z", LD_Z, 0b1101001000001000, 0b1000000000000000, 2, 1},

  {"LDS", LDS, 0b1111111000001111, 0b1001000000000000, 2, 2},

  {"SPM", SPM, 0b1111111111111111, 0b1001010111101000, 5, 1},

  {"IN", IN, 0b1111100000000000, 0b1011000000000000, 1, 1},
  {"OUT", OUT, 0b1111100000000000, 0b1011100000000000, 1, 1},
  {"PUSH", PUSH, 0b1111111000001111, 0b1001001000001111, 2, 1},
  {"POP", POP, 0b1111111000001111, 0b1001000000001111, 2, 1},

  {"NOP", NOP, 0b1111111111111111, 0b0000000000000000, 1, 1},
  {"SLEEP", SLEEP, 0b1111111111111111, 0b1001010110001000, 1, 1},
  {"WDR", WDR, 0b1111111111111111, 0b1001010110101000, 1, 1},
  {"BREAK", BREAK, 0b1111111111111111, 0b1001010110011000, 0, 1},

  {"XXX", XXX, 0b1111111111111111, 0b1111111111111111, 1, 1}
};

static const int opcodes_count = sizeof(opcodes) / sizeof(Instruction_t);

static inline uint16_t get_opcode16(void) {
  if ((mcu.pc + 1) * WORD_SIZE >= PROGRAM_MEMORY_SIZE - 1) {
    throw_exception("Out of memory bounds!\n");
    return 0;
  }
  return *((uint16_t *)(mcu.program_memory + mcu.pc * WORD_SIZE));
}

static inline uint32_t get_opcode32(void) {
  if ((mcu.pc + 2) * WORD_SIZE  >= PROGRAM_MEMORY_SIZE - 1) {
    throw_exception("Out of memory bounds!\n");
    return 0;
  }
  return (get_opcode16() << 16) | *(uint16_t *)(mcu.program_memory + (mcu.pc + 1) * WORD_SIZE);
}

static void create_lookup_table(void) {
  for (int i = 0; i < LOOKUP_SIZE; i++) {
    opcode_lookup[i] = find_instruction(i);
  }
}

static const Instruction_t *find_instruction(const uint16_t opcode) {
  for (int i = 0; i < opcodes_count; i++) {
    if ((opcodes[i].mask1 & opcode) == opcodes[i].mask2) {
      return opcodes + i;
    }
  }
  return opcodes + opcodes_count - 1; // XXX
}

void mcu_init(void) {
  mkdir(TMP, 0777);
  memset(&mcu, 0, sizeof(mcu));
  set_mcu_pointers(&mcu);
  create_lookup_table();
  print("MCU initialized\n");
}

void mcu_send_interrupt(Interrupt_vector_t vector) {
  print("Sending an interrupt: %d\n", (int)vector);
  mcu.handle_interrupt = true;
  mcu.interrupt_address = (uint16_t)vector;
}

static inline void handle_interrupt(void) {
  print("Received an interrupt (%d)\n", mcu.interrupt_address);
  if (mcu.sleeping) {
    print("Waking up from sleep mode\n");
    mcu.sleeping = false;
  }
  uint16_t return_address = mcu.pc;
  stack_push16(return_address);
  mcu.pc = mcu.interrupt_address * WORD_SIZE;
  do {
    mcu_execute_cycle();
  } while (mcu.pc != return_address);
  print("Interrupt finished\n");
  mcu.interrupt_address = 0;
}

static inline void execute_instruction(void) {
  mcu.opcode = get_opcode16();
  mcu.instruction = opcode_lookup[mcu.opcode];
  if (mcu.skip_next) {
    mcu.pc += mcu.instruction->length;
    mcu.skip_next = false;
    return;
  }
  print("Executing %s, PC = 0x%x\n", mcu.instruction->name, mcu.pc * WORD_SIZE);
  if (mcu.instruction->length == 2) {
    mcu.opcode = get_opcode32();
  }
  mcu.instruction->execute(mcu.opcode);
  mcu.cycles = mcu.instruction->cycles - 1;
}

bool mcu_execute_cycle(void) {
  uint64_t time_start = get_micro_time();
  mcu.data_memory_change = -1; // indicate no change
  if (mcu.cycles > 0) {
    usleep(CLOCK_FREQ);
    mcu.cycles--;
    return true;
  }
  if (mcu.handle_interrupt) {
    mcu.handle_interrupt = false;
    handle_interrupt();
    time_start = get_micro_time();
  }
  if (!mcu.sleeping) {
    execute_instruction();
  }
  if (mcu.stopped) {
    mcu.cycles = 0; // fix BREAK
    return false;
  }
  uint64_t sleep_time = (CLOCK_FREQ - (get_micro_time() - time_start)) % CLOCK_FREQ;
  if (sleep_time > 0) {
    usleep(sleep_time);
  }
  return true;
}

void mcu_run(void) {
  mcu.auto_execute = true;
  while (mcu_execute_cycle());
}

void mcu_resume(void) {
  mcu.stopped = false;
  mcu.pc += 1; // skip BREAK
  if (mcu.auto_execute) {
    mcu_run();
  }
}

bool mcu_load_ihex(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    print("Could not open %s\n", filename);
    return false;
  }
  char buffer[BUFFER_LENGTH];
  memset(buffer, 0, BUFFER_LENGTH);
  int memory_index = 0;
  while (fgets(buffer, BUFFER_LENGTH, file)) {
    char *line = buffer + 1;
    char number_buffer[2];
    memset(number_buffer, 0, 2);
    sscanf(line, "%2s", number_buffer);
    int data_length = (int)strtol(number_buffer, 0, 16);
    line += 6;
    memset(number_buffer, 0, 2);
    sscanf(line, "%2s", number_buffer);
    if (strncmp(number_buffer, "01", 2) == 0) {
      break;
    } else if (strncmp(number_buffer, "00", 2) != 0) {
      continue;
    }
    line += 2;
    char *data_buffer = calloc(data_length * 2, sizeof(char));
    memcpy(data_buffer, line, data_length * 2);
    for (int i = 0; i < data_length * 2; i += 4) {
      char low[3], high[3];
      memset(low, 0, 3);
      memset(high, 0, 3);
      sscanf(data_buffer + i, "%2s", low);
      sscanf(data_buffer + i + 2, "%2s", high);
      uint16_t word = ((uint16_t)strtol(high, 0, 16) << 8) | (uint16_t)strtol(low, 0, 16);
      if (memory_index >= PROGRAM_MEMORY_SIZE) {
        print("Cannot fit the whole program in memory\n");
        free(data_buffer);
        return false;
      }
      print("Writing 0x%.4X to memory\n", word);
      memcpy(mcu.program_memory + memory_index, &word, sizeof(word));
      memory_index += sizeof(word);
    }
    free(data_buffer);
    memset(buffer, 0, BUFFER_LENGTH);
  }
  return true;
}

bool mcu_load_asm(const char *code) {
  FILE *file = fopen(TMP"_t.asm", "w");
  if (file == NULL) {
    return false;
  }
  fputs(".DEVICE ATmega328P\n", file); // for AVRA range checks
  fputs(code, file);
  fclose(file);
  int status = system("avra "TMP"_t.asm > /dev/null");
  if (status == -1) {
    return false;
  }
  bool loaded = mcu_load_ihex(TMP"_t.hex");
  const char *const files[] = {TMP"_t.asm", TMP"_t.hex", TMP"_t.obj", TMP"_t.eep.hex", TMP"_t.cof"};
  for (int i = 0; i < sizeof(files) / sizeof(char *); i++) {
    remove(files[i]);
  }
  return loaded;
}

bool mcu_load_c(const char *code) {
  FILE *file = fopen(TMP"_t.c", "w");
  if (file == NULL) {
    print("Could not create the file\n");
    return false;
  }
  fputs(code, file);
  fclose(file);
  int status = system(
    "avr-gcc "
    "-Wall -Wextra -O3 -mmcu=atmega328p "
    "-o "TMP"_t.bin "TMP"_t.c"
  );
  remove(TMP"_t.c");
  if (status == -1) {
    print("Could not compile\n");
    return false;
  }
  status = system(
    "avr-objcopy -j .text -j .data -O ihex "TMP"_t.bin "TMP"_t.hex"
  );
  remove(TMP"_t.bin");
  if (status == -1) {
    print("Could not generate ihex file\n");
    return false;
  }
  bool loaded = mcu_load_ihex(TMP"_t.hex");
  remove(TMP"_t.hex");
  return loaded;
}

void mcu_get_copy(ATmega328p_t *_mcu) {
  *_mcu = mcu;
  set_mcu_pointers(_mcu);
}

static inline void set_mcu_pointers(ATmega328p_t *const mcu) {
  mcu->boot_section = &mcu->program_memory[PROGRAM_MEMORY_SIZE - BOOTLOADER_SIZE];
  mcu->R = &mcu->data_memory[0];
  mcu->IO = &mcu->R[REGISTER_COUNT];
  mcu->ext_IO = &mcu->IO[IO_REGISTER_COUNT];
  mcu->RAM = &mcu->ext_IO[EXT_IO_REGISTER_COUNT];
  mcu->sp = RAM_SIZE - 1;
}

static inline void stack_push16(const uint16_t value) {
  *((uint16_t *)(mcu.RAM + mcu.sp)) = value;
  mcu.sp -= 2;
}

static inline void stack_push8(const uint8_t value) {
  mcu.RAM[mcu.sp] = value;
  mcu.sp -= 1;
}

static inline uint16_t stack_pop16(void) {
  mcu.sp += 2;
  return *(uint16_t *)(mcu.RAM + mcu.sp);
}

static inline uint8_t stack_pop8(void) {
  mcu.sp += 1;
  return mcu.RAM[mcu.sp];
}

static inline uint16_t word_reg_get(const uint8_t d) {
  uint16_t low = mcu.R[d];
  uint16_t high = mcu.R[d + 1];
  return (high << 8) | low;
}

static inline void word_reg_set(const uint8_t d, const uint16_t value) {
  uint16_t low = value & 0x00FF;
  uint16_t high = (value & 0xFF00) >> 8;
  mcu.R[d] = low;
  mcu.R[d + 1] = high;
}

static inline uint16_t X_reg_get(void) {
  return word_reg_get(26);
}

static inline uint16_t Y_reg_get(void) {
  return word_reg_get(28);
}

static inline uint16_t Z_reg_get(void) {
  return word_reg_get(30);
}

static inline void X_reg_set(const uint16_t value) {
  word_reg_set(26, value);
}

static inline void Y_reg_set(const uint16_t value) {
  word_reg_set(28, value);
}

static inline void Z_reg_set(const uint16_t value) {
  word_reg_set(30, value);
}

static inline uint64_t get_micro_time(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * ((uint64_t)1000000) + tv.tv_usec;
}

void mcu_set_exception_handler(void (*handler)(void)) {
  mcu.exception_handler = handler;
}

static inline void throw_exception(const char *cause, ...) {
  printf(RED "MCU exception!\n");
  va_list args;
  va_start(args, cause);
  vfprintf(stdout, cause, args);
  va_end(args);
  printf(RESET);
  mcu_send_interrupt(RESET_vect);
  if (mcu.exception_handler != NULL) {
    mcu.exception_handler();
  }
}