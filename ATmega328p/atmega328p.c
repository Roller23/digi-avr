#include "atmega328p.h"
#include "instructions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdarg.h>

#define CYAN "\033[36m"
#define RESET "\x1B[0m"

#define DEBUG 1

static int print(const char *format, ...) {
  int a = 0;
  #if DEBUG == 1
    va_list args;
    va_start(args, format);
    a += printf(CYAN "[Debug] ");
    a += vfprintf(stdout, format, args);
    a += printf(RESET);
    va_end(args);
  #endif
  return a;
}

#define b_get(number, n) (number & (1LLU << (n)))
#define MS 1000
#define SEC (MS * 1000)
#define CLOCK_FREQ (SEC / 100)

static void print_bits(uint32_t number) {
  char bits[35];
  memset(bits, 0, 33);
  for (int i = sizeof(number) * 8 - 1, j = 0; i >= 0; i--, j++) {
    bits[j] = b_get(number, i) ? '1' : '0';
    if (i == 16) {
      bits[j + 1] = '\n';
      j++;
    }
  }
  print("0x%.8X bits\n%s\n", number, bits);
}

static ATmega328p_t mcu;

static inline void ADD(uint32_t opcode) {
  // 0000 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] + mcu.R[reg_r];
  mcu.SREG.flags.H = !!(b_get(mcu.R[reg_d], 3) & b_get(mcu.R[reg_r], 3) | b_get(mcu.R[reg_r], 3) & ~b_get(result, 3) | ~b_get(result, 3) & b_get(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) & ~b_get(result, 7) | ~b_get(mcu.R[reg_d], 7) & ~b_get(mcu.R[reg_r], 7) & b_get(result, 7));
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) | b_get(mcu.R[reg_r], 7) & ~b_get(result, 7) | ~b_get(result, 7) & b_get(mcu.R[reg_d], 7));
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}

static inline void ADC(uint32_t opcode) {
  // 0001 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] + mcu.R[reg_r] + mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(b_get(mcu.R[reg_d], 3) & b_get(mcu.R[reg_r], 3) | b_get(mcu.R[reg_r], 3) & ~b_get(result, 3) | ~b_get(result, 3) & b_get(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) & ~b_get(result, 7) | ~b_get(mcu.R[reg_d], 7) & ~b_get(mcu.R[reg_r], 7) & b_get(result, 7));
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) | b_get(mcu.R[reg_r], 7) & ~b_get(result, 7) | ~b_get(result, 7) & b_get(mcu.R[reg_d], 7));
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void ADIW(uint32_t opcode) {
  // 1001 0110 KKdd KKKK
  uint8_t k = (opcode & 0xF);
  k |= b_get(opcode, 6) >> 2;
  k |= b_get(opcode, 7) >> 2;
  uint8_t reg_d = ((!!b_get(opcode, 5)) << 1) | (!!b_get(opcode, 4));
  reg_d = reg_d * 2 + 24;
  uint16_t rd = word_reg_get(reg_d);
  uint16_t result = rd + k;
  mcu.SREG.flags.V = !b_get(rd, 15) & !!b_get(result, 15);
  mcu.SREG.flags.N = !!b_get(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !b_get(result, 15) & !!b_get(rd, 15);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  word_reg_set(reg_d, result);
  mcu.pc += 1;
}
static inline void SUB(uint32_t opcode){
  // 0001 10rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] - mcu.R[reg_r];
  mcu.SREG.flags.H = !!(~b_get(mcu.R[reg_d], 3) & b_get(mcu.R[reg_r], 3) | b_get(mcu.R[reg_r], 3) & b_get(result, 3) | b_get(result, 3) & ~b_get(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d], 7) & ~b_get(mcu.R[reg_r], 7) & ~b_get(result, 7) | ~b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) & b_get(result, 7));
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(~b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) | b_get(mcu.R[reg_r], 7) & b_get(result, 7) | b_get(result, 7) & ~b_get(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void SUBI(uint32_t opcode){
  // 0101 kkkk dddd kkkk
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t k = (opcode & 0xF) | ((opcode & 0xF00) >> 4);
  uint8_t result = mcu.R[reg_d] - k;
  mcu.SREG.flags.H = !!(~b_get(mcu.R[reg_d], 3) & b_get(k, 3) | b_get(k, 3) & b_get(result, 3) | b_get(result, 3) & ~b_get(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d], 7) & ~b_get(k, 7) & ~b_get(result, 7) | ~b_get(mcu.R[reg_d], 7) & b_get(k, 7) & b_get(result, 7));
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(~b_get(mcu.R[reg_d], 7) & b_get(k, 7) | b_get(k, 7) & b_get(result, 7) | b_get(result, 7) & ~b_get(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void SBC(uint32_t opcode){
  // 0000 10rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] - mcu.R[reg_r] - mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(~b_get(mcu.R[reg_d], 3) & b_get(mcu.R[reg_r], 3) | b_get(mcu.R[reg_r], 3) & b_get(result, 3) | b_get(result, 3) & ~b_get(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d], 7) & ~b_get(mcu.R[reg_r], 7) & ~b_get(result, 7) | ~b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) & b_get(result, 7));
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0) & mcu.SREG.flags.Z;
  mcu.SREG.flags.C = !!(~b_get(mcu.R[reg_d], 7) & b_get(mcu.R[reg_r], 7) | b_get(mcu.R[reg_r], 7) & b_get(result, 7) | b_get(result, 7) & ~b_get(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void SBCI(uint32_t opcode){
  // 0100 kkkk dddd kkkk
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t k = (opcode & 0xF) | ((opcode & 0xF00) >> 4);
  uint8_t result = mcu.R[reg_d] - k - mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(~b_get(mcu.R[reg_d], 3) & b_get(k, 3) | b_get(k, 3) & b_get(result, 3) | b_get(result, 3) & ~b_get(mcu.R[reg_d], 3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d], 7) & ~b_get(k, 7) & ~b_get(result, 7) | ~b_get(mcu.R[reg_d], 7) & b_get(k, 7) & b_get(result, 7));
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0) & mcu.SREG.flags.Z;
  mcu.SREG.flags.C = !!(~b_get(mcu.R[reg_d], 7) & b_get(k, 7) | b_get(k, 7) & b_get(result, 7) | b_get(result, 7) & ~b_get(mcu.R[reg_d], 7));
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void SBIW(uint32_t opcode){
  // 1001 0111 KKdd KKKK
  uint8_t k = (opcode & 0xF);
  k |= b_get(opcode, 6) >> 2;
  k |= b_get(opcode, 7) >> 2;
  uint8_t reg_d = ((!!b_get(opcode, 5)) << 1) | (!!b_get(opcode, 4));
  reg_d = reg_d * 2 + 24;
  uint16_t rd = word_reg_get(reg_d);
  uint16_t result = rd - k;
  mcu.SREG.flags.V = !!b_get(result, 15) & !b_get(rd, 15);
  mcu.SREG.flags.N = !!b_get(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!b_get(result, 15) & !b_get(rd, 15);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  word_reg_set(reg_d, result);
  mcu.pc += 1;
}
static inline void AND(uint32_t opcode){
  // 0010 00rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] & mcu.R[reg_r];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void ANDI(uint32_t opcode){
  // 0111 KKKK dddd KKKK
  uint8_t k = (opcode & 0xF);
  k |= (opcode & 0xF00) >> 4;
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint16_t result = mcu.R[reg_d] & k;
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void OR(uint32_t opcode){
  // 0010 10rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] | mcu.R[reg_r];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void ORI(uint32_t opcode){
  // 0110 KKKK dddd KKKK
  uint8_t k = (opcode & 0xF);
  k |= (opcode & 0xF00) >> 4;
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint16_t result = mcu.R[reg_d] | k;
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void EOR(uint32_t opcode){
  // 0010 01rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint8_t result = mcu.R[reg_d] ^ mcu.R[reg_r];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void COM(uint32_t opcode){
  // 1001 010d dddd 0000
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t result = 0xFF - mcu.R[reg_d];
  mcu.SREG.flags.V = 0;
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = 1;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void NEG(uint32_t opcode){
  // 1001 010d dddd 0001
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t result = 0x00 - mcu.R[reg_d];
  mcu.SREG.flags.H = !!b_get(result, 3) | !b_get(mcu.R[reg_d], 3); //TD: check H
  mcu.SREG.flags.V = !!b_get(result, 7) & ((result & 0b01111111) == 0);
  mcu.SREG.flags.N = !!b_get(result, 7);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = (result != 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.pc += 1;
}
static inline void INC(uint32_t opcode){
  // 1001 010d dddd 0011
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  mcu.R[reg_d] = mcu.R[reg_d] + 1;
  mcu.SREG.flags.V = !!b_get(mcu.R[reg_d], 7) & ((mcu.R[reg_d] & 0b01111111) == 0);
  mcu.SREG.flags.N = !!b_get(mcu.R[reg_d], 7);
  mcu.SREG.flags.Z = (mcu.R[reg_d] == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}
static inline void DEC(uint32_t opcode){
  // 1001 010d dddd 1010
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  mcu.R[reg_d] = mcu.R[reg_d] - 1;
  mcu.SREG.flags.V = !b_get(mcu.R[reg_d], 7) & ((mcu.R[reg_d] & 0b01111111) == 0b01111111);
  mcu.SREG.flags.N = !!b_get(mcu.R[reg_d], 7);
  mcu.SREG.flags.Z = (mcu.R[reg_d] == 0);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}
static inline void SER(uint32_t opcode){
  // 1110 1111 dddd 1111
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  mcu.R[reg_d] = 0xFF;
  mcu.pc += 1;
}
static inline void MUL(uint32_t opcode){
  // 1001 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  uint16_t result = mcu.R[reg_d] * mcu.R[reg_r];
  mcu.SREG.flags.C = !!b_get(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  word_reg_set(0, result);
  mcu.pc += 1;
}
static inline void MULS(uint32_t opcode){
  // 0000 0010 dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0xF);
  reg_r += 16;
  int16_t result = (int8_t)mcu.R[reg_d] * (int8_t)mcu.R[reg_r];
  mcu.SREG.flags.C = !!b_get(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  word_reg_set(0, (uint16_t)result);
  mcu.pc += 1;
}
static inline void MULSU(uint32_t opcode){
  // 0000 0011 0ddd 0rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  int16_t result = (int8_t)mcu.R[reg_d] * mcu.R[reg_r];
  mcu.SREG.flags.C = !!b_get(result, 15);
  mcu.SREG.flags.Z = (result == 0);
  word_reg_set(0, (uint16_t)result);
  mcu.pc += 1;
}
static inline void FMUL(uint32_t opcode){
  // 0000 0011 0ddd 1rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  double d = (mcu.R[reg_d] / (double)(1 << 7));
  double r = (mcu.R[reg_r] / (double)(1 << 7));
  double res = d * r;
  uint16_t result = round(res * (1 << 14));
  mcu.SREG.flags.C = b_get(result, 15) >> 15;
  mcu.SREG.flags.Z = (result == 0);
  result <<= 1;
  word_reg_set(0, result);
  mcu.pc += 1;
}
static inline void FMULS(uint32_t opcode){
  // 0000 0011 1ddd 0rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  double d = ((int8_t)mcu.R[reg_d] / (double)(1 << 7));
  double r = ((int8_t)mcu.R[reg_r] / (double)(1 << 7));
  double res = d * r;
  uint16_t result = round(res * (1 << 14));
  mcu.SREG.flags.C = b_get(result, 15) >> 15;
  mcu.SREG.flags.Z = (result == 0);
  result <<= 1;
  word_reg_set(0, result);
  mcu.pc += 1;
}
static inline void FMULSU(uint32_t opcode){
  // 0000 0011 1ddd 1rrr
  uint8_t reg_d = (opcode & 0x70) >> 4;
  reg_d += 16;
  uint8_t reg_r = (opcode & 0x7);
  reg_r += 16;
  double d = ((int8_t)mcu.R[reg_d] / (double)(1 << 7));
  double r = (mcu.R[reg_r] / (double)(1 << 7));
  double res = d * r;
  uint16_t result = round(res * (1 << 14));
  mcu.SREG.flags.C = b_get(result, 15) >> 15;
  mcu.SREG.flags.Z = (result == 0);
  result <<= 1;
  word_reg_set(0, result);
  mcu.pc += 1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static inline void MOV(uint32_t opcode){
  // 0010 11rd dddd rrrr
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode, 9) >> 5;
  mcu.R[reg_d] = mcu.R[reg_r];
  mcu.pc += 1;  
}
static inline void MOVW(uint32_t opcode){
  // 0000 0001 dddd rrrr
  uint8_t reg_d = ((opcode & 0xF0) >> 4) * 2;
  uint8_t reg_r = (opcode & 0xF) * 2;
  word_reg_set(reg_d, word_reg_get(reg_r));
  mcu.pc += 1;  
}
static inline void LDI(uint32_t opcode){
  // 1110 kkkk dddd kkkk
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d += 16;
  uint8_t k = (opcode & 0xF);
  k |= (opcode & 0xF00) >> 4;
  mcu.R[reg_d] = k;
  mcu.pc += 1;  
}
//-----------
static inline void IN(uint32_t opcode){
  // 1011 0AAd dddd AAAA
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  uint8_t a = (opcode & 0xF) | ((opcode & 0x600) >> 5);
  mcu.R[reg_d] = mcu.IO[a];
  mcu.pc += 1;  
}
static inline void OUT(uint32_t opcode);
static inline void PUSH(uint32_t opcode){
  // 1001 001d dddd 1111
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  stack_push8(mcu.R[reg_d]);
  mcu.pc += 1; 
}
static inline void POP(uint32_t opcode){
  // 1001 000d dddd 1111
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode, 8) >> 4;
  mcu.R[reg_d] = stack_pop8();
  mcu.pc += 1; 
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// this is my turf

static inline void RJMP(uint32_t opcode) {
  // 1100 kkkk kkkk kkkk
  // Relative jump to PC + k + 1
  uint16_t k = opcode & 0x0FFF;
  mcu.pc += k + 1;
}

static inline void IJMP(uint32_t opcode) {
  // Indirect jump to address at Z register
  mcu.pc = Z_reg_get();
}

static inline void JMP(uint32_t opcode) {
  // 1001 010k kkkk 110k
  // kkkk kkkk kkkk kkkk
  // Jump to address k, PC = k
  uint32_t k = opcode & 0b11111111111111111;
  k |= (opcode & 0xF00000) >> 3;
  k |= b_get(opcode, 24) >> 3;
  mcu.pc = k;
}

static inline void RCALL(uint32_t opcode) {
  // 1101 kkkk kkkk kkkk
  // Jump to address + 1 + PC, push current PC + 1 onto stack (relative call)
  uint16_t k = opcode & 0xFFF;
  stack_push16(mcu.pc + 1);
  mcu.pc += k + 1;
}

static inline void ICALL(uint32_t opcode) {
  // Indirect call, PC = Z, push PC + 1 to stack
  stack_push16(mcu.pc + 1);
  mcu.pc = Z_reg_get();
}

static inline void CALL(uint32_t opcode) {
  // 1001 010k kkkk 111k
  // kkkk kkkk kkkk kkkk
  // Long call, push PC + 2 to stack, PC = k
  uint32_t k = opcode & 0xFFFF;
  k |= b_get(opcode, 16);
  k |= (0xF00000) >> 3;
  k |= b_get(opcode, 24) >> 3;
  stack_push16(mcu.pc + 2);
  mcu.pc = k;
}

static inline void RET(uint32_t opcode) {
  // Return from subroutine, PC = stack
  mcu.pc = stack_pop16();
}

static inline void RETI(uint32_t opcode) {
  // Return from interrupt and set I to 1
  mcu.pc = stack_pop16();
  mcu.SREG.flags.I = 1;
}

static inline void CPSE(uint32_t opcode) {
  // 0001 00rd dddd rrrr
  // Compare, skip if equal
  uint16_t r = (opcode & 0xF) | (b_get(opcode, 9) >> 5);
  uint16_t d = (opcode & 0b111110000) >> 4;
  if (mcu.R[r] == mcu.R[d]) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void CP(uint32_t opcode) {
  // 0001 01rd dddd rrrr
  // Compare two registers
  uint16_t r = (opcode & 0xF) | (b_get(opcode, 9) >> 5);
  uint16_t d = (opcode & 0b111110000) >> 4;
  byte *R = mcu.R;
  uint8_t res = R[d] - R[r];
  mcu.SREG.flags.H = !b_get(R[d], 3) && b_get(R[r], 3) || b_get(R[r], 3) && b_get(res, 3) || b_get(res, 3) && !b_get(R[d], 3);
  mcu.SREG.flags.V = b_get(R[d], 7) && !b_get(R[r], 7) && b_get(res, 7) || !b_get(R[d], 7) && b_get(R[r], 7) && b_get(res, 7);
  mcu.SREG.flags.N = !!b_get(res, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (res == 0);
  mcu.SREG.flags.C = !b_get(R[d], 7) && b_get(R[r], 7) || b_get(R[r], 7) && b_get(res, 7) || b_get(res, 7) && !b_get(R[d], 7);
  mcu.pc += 1;
}

static inline void CPC(uint32_t opcode) {
  // 0000 01rd dddd rrrr
  // Compare with carry
  uint16_t r = (opcode & 0xF) | (b_get(opcode, 9) >> 5);
  uint16_t d = (opcode & 0b111110000) >> 4;
  byte *R = mcu.R;
  uint8_t res = R[d] - R[r] - mcu.SREG.flags.C;
  mcu.SREG.flags.H = !b_get(R[d], 3) && b_get(R[r], 3) || b_get(R[r], 3) && b_get(res, 3) || b_get(res, 3) && !b_get(R[d], 3);
  mcu.SREG.flags.V = b_get(R[d], 7) && !b_get(R[r], 7) && b_get(res, 7) || !b_get(R[d], 7) && b_get(R[r], 7) && b_get(res, 7);
  mcu.SREG.flags.N = !!b_get(res, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.C = !b_get(R[d], 7) && b_get(R[r], 7) || b_get(R[r], 7) && b_get(res, 7) || b_get(res, 7) && !b_get(R[d], 7);
  mcu.SREG.flags.Z = (res == 0) && mcu.SREG.flags.Z;
  mcu.pc += 1;
}

static inline void CPI(uint32_t opcode) {
  // 0011 KKKK dddd KKKK
  // Compare with immediate
  uint16_t k = (opcode & 0xF) | ((opcode & 0xF00) >> 4);
  uint16_t d = (opcode & 0xF0) >> 4;
  byte *R = mcu.R;
  uint8_t res = R[d] - k;
  mcu.SREG.flags.H = !b_get(R[d], 3) && b_get(k, 3) || b_get(k, 3) && b_get(res, 3) || b_get(res, 3) && !b_get(R[d], 3);
  mcu.SREG.flags.V = b_get(R[d], 7) && !b_get(k, 7) && !b_get(res, 7) || !b_get(R[d], 7) && b_get(k, 7) && b_get(res, 7);
  mcu.SREG.flags.N = !!b_get(res, 7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (res == 0);
  mcu.SREG.flags.C = !b_get(R[d], 7) && b_get(k, 7) || b_get(k, 7) && b_get(res, 7) || b_get(res, 7) && !b_get(R[d], 7);
  mcu.pc += 1;
}

static inline void SBRC(uint32_t opcode) {
  // 1111 110r rrrr 0bbb
  // Skip if R[r](b) is cleared
  uint8_t b = (opcode & 0b111);
  uint16_t r = (opcode & 0b111110000) >> 4;
  if (!b_get(mcu.R[r], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void SBRS(uint32_t opcode) {
  // 1111 111r rrrr 0bbb
  // Skip if R[r](b) is set
  uint8_t b = (opcode & 0b111);
  uint16_t r = (opcode & 0b111110000) >> 4;
  if (b_get(mcu.R[r], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void SBIC(uint32_t opcode) {
  // 1001 1001 AAAA Abbb
  // Skip if I/O[A](b) is cleared
  uint16_t b = (opcode & 0b111);
  uint16_t A = (opcode & 0b11111000) >> 3;
  if (!b_get(mcu.IO[A], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void SBIS(uint32_t opcode) {
  // 1001 1011 AAAA Abbb
  // Skip if I/O[A](b) is set
  uint16_t b = (opcode & 0b111);
  uint16_t A = (opcode & 0b11111000) >> 3;
  if (b_get(mcu.IO[A], b)) {
    mcu.skip_next = true;
  }
  mcu.pc += 1;
}

static inline void BRBS(uint32_t opcode) {
  // 1111 00kk kkkk ksss
  // Branch if SREG(s) is set (PC += k + 1), k is in U2
  uint8_t s = opcode & 0b111;
  int8_t k = ((b_get(opcode, 9) << 1) | (opcode & 0b1111111000)) >> 3;
  if (b_get(mcu.SREG.value, s)) {
    mcu.pc += k + 1;
    return;
  }
  mcu.pc += 1;
}

static inline void BRBC(uint32_t opcode) {
  // 1111 01kk kkkk ksss
  // Branch if SREG(s) is cleared (PC += k + 1), k is in U2
  uint8_t s = opcode & 0b111;
  int8_t k = ((b_get(opcode, 9) << 1) | (opcode & 0b1111111000)) >> 3;
  if (!b_get(mcu.SREG.value, s)) {
    mcu.pc += k + 1;
    return;
  }
  mcu.pc += 1;
}

static inline void SBI(uint32_t opcode) {
  // 1001 1010 AAAA Abbb
  // Set I/O[A](b)
  uint8_t b = opcode & 0b111;
  uint8_t A = (opcode & 0b11111000) >> 3;
  mcu.IO[A] |= (1LU << b);
  mcu.pc += 1;
}

static inline void CBI(uint32_t opcode) {
  // 1001 1000 AAAA Abbb
  // Clear I/O[A](b)
  uint8_t b = opcode & 0b111;
  uint8_t A = (opcode & 0b11111000) >> 3;
  mcu.IO[A] &= ~(1LU << b);
  mcu.pc += 1;
}

static inline void LSR(uint32_t opcode) {
  // 1001 010d dddd 0110
  // C = R[d](0), R[d] >> 1
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.SREG.flags.C = !!b_get(mcu.R[d], 0);
  mcu.R[d] >>= 1;
  mcu.SREG.flags.Z = (mcu.R[d] == 0);
  mcu.SREG.flags.N = 0;
  mcu.SREG.flags.V = mcu.SREG.flags.N ^ mcu.SREG.flags.C;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void ROR(uint32_t opcode) {
  // 1001 010d dddd 0111
  // C = R[d](0), R[d] >> 1, R[d](7) = C
  uint8_t d = (opcode & 0b111110000) >> 4;
  bit carry = !!mcu.SREG.flags.C;
  mcu.SREG.flags.C = !!b_get(mcu.R[d], 0);
  mcu.R[d] >>= 1;
  mcu.R[d] |= (carry << 7);
  mcu.SREG.flags.Z = (mcu.R[d] == 0);
  mcu.SREG.flags.N = !!b_get(mcu.R[d], 7);
  mcu.SREG.flags.V = mcu.SREG.flags.N ^ mcu.SREG.flags.C;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void ASR(uint32_t opcode) {
  // 1001 010d dddd 0101
  // Shift right without changing R[d](7), C = R[d](0)
  uint8_t d = (opcode & 0b111110000) >> 4;
  bit b7 = b_get(mcu.R[d], 7);
  mcu.SREG.flags.C = !!b_get(mcu.R[d], 0);
  mcu.R[d] >>= 1;
  mcu.R[d] |= b7;
  mcu.SREG.flags.Z = (mcu.R[d] == 0);
  mcu.SREG.flags.N = !!b_get(mcu.R[d], 7);
  mcu.SREG.flags.V = mcu.SREG.flags.N ^ mcu.SREG.flags.C;
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.pc += 1;
}

static inline void SWAP(uint32_t opcode) {
  // 1001 010d dddd 0010
  // Swap nibbles
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.R[d] = ((mcu.R[d] & 0x0F) << 4) | ((mcu.R[d] & 0xF0) >> 4);
  mcu.pc += 1;
}

static inline void BSET(uint32_t opcode) {
  // 1001 0100 0sss 1000
  // SREG(s) = 1
  uint8_t s = (opcode & 0b1110000) >> 4;
  mcu.SREG.value |= (1LU << s);
  mcu.pc += 1;
}

static inline void BCLR(uint32_t opcode) {
  // 1001 0100 1sss 1000
  // SREG(s) = 0
  uint8_t s = (opcode & 0b1110000) >> 4;
  mcu.SREG.value &= ~(1LU << s);
  mcu.pc += 1;
}

static inline void BST(uint32_t opcode) {
  // 1111 101d dddd 0bbb
  // T = R[d](b)
  uint8_t b = opcode & 0b111;
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.SREG.flags.T = !!b_get(mcu.R[d], b);
  mcu.pc += 1;
}

static inline void BLD(uint32_t opcode) {
  // 1111 100d dddd 0bbb
  // R[d](b) = T
  uint8_t b = opcode & 0b111;
  uint8_t d = (opcode & 0b111110000) >> 4;
  mcu.R[d] |= ((!!mcu.SREG.flags.T) << b);
  mcu.pc += 1;
}

static inline void NOP(uint32_t opcode) {
  mcu.pc += 1;
}

static inline void SLEEP(uint32_t opcode) {
  mcu.sleeping = true;
  mcu.pc += 1;
}

static inline void WDR(uint32_t opcode) {
  // Reset watchdog timer
  mcu.pc += 1;
}

static inline void BREAK(uint32_t opcode) {
  print("Break encountered\n");
  mcu.stopped = true;
}

static inline void XXX(uint32_t opcode) {
  // Unknown opcode
  print("Unknown opcode! 0x%.4X\n", opcode);
  mcu.pc += 1;
}

static Instruction_t opcodes[] = {
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
  {"MUL", MUL, 0b1111110000000000, 0b1001110000000000, 2, 1},   //??1
  {"MULS", MULS, 0b1111111100000000, 0b0000001000000000, 2, 1}, //??1
  {"MULSU", MULSU, 0b1111111110001000, 0b0000001100000000, 2, 1},
  // {"FMUL", FMUL, 0b1111111110001000, 0b0000001100001000, 2, 1},
  // {"FMULS", FMULS, 0b1111111110001000, 0b0000001110000000, 2, 1},
  // {"FMULSU", FMULSU, 0b1111111110001000, 0b0000001110001000, 2, 1},

  {"RJMP", RJMP, 0b1111000000000000, 0b1100000000000000, 2, 1},
  {"IJMP", IJMP, 0b1111111111111111, 0b1001010000001001, 2, 1},
  {"JMP", JMP, 0b1111111000001110, 0b1001010000001100, 3, 2},
  {"RCALL", RCALL, 0b1111000000000000, 0b1101000000000000, 3, 1},
  {"ICALL", ICALL, 0b1111111111111111, 0b1001010100001001, 3, 1},
  {"CALL", CALL, 0b1111111000001110, 0b1001010000001110, 4, 2},
  {"RET", RET, 0b1111111111111111, 0b1001010100001000, 4, 1},
  {"RETI", RETI, 0b1111111111111111, 0b1001010100011000, 4, 1},
  {"CPSE", CPSE, 0b1111110000000000, 0b0001000000000000, 1, 1}, //2/3
  {"CP", CP, 0b1111110000000000, 0b0001010000000000, 1, 1},
  {"CPC", CPC, 0b1111110000000000, 0b0000010000000000, 1, 1},
  {"CPI", CPI, 0b1111000000000000, 0b0011000000000000, 1, 1},
  {"SBRC", SBRC, 0b1111111000001000, 0b1111110000000000, 1, 1}, //2/3
  {"SBRS", SBRS, 0b1111111000001000, 0b1111111000000000, 1, 1}, //2/3
  {"SBIC", SBIC, 0b1111111100000000, 0b1001100100000000, 1, 1}, //2/3
  {"SBIS", SBIS, 0b1111111100000000, 0b1001101100000000, 1, 1}, //2/3
  {"BRBS", BRBS, 0b1111110000000000, 0b1111000000000000, 1, 1}, //2
  {"BRBC", BRBC, 0b1111110000000000, 0b1111010000000000, 1, 1}, //2

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
  {"LDI", LDI, 0b1111000000000000, 0b1110000000000000, 1, 1}, //???1

  // {"ST", ST, 0b1111111000001111, 0b1001001000001100, 1, 1},
  // {"ST", ST, 0b1111111000001111, 0b1001001000001101, 1, 1},
  // {"ST", ST, 0b1111111000001111, 0b1001001000001110, 2, 1},

  // {"SPM", SPM, 0b1111111111111111, 0b1001010111101000, 1, 1},
  // {"SPM", SPM, 0b1111111111111111, 0b1001010111111000, 1, 1},

  // //....
  // {"IN", IN, 0b1111100000000000, 0b1011000000000000, 1, 1},
  // {"OUT", OUT, 0b1111100000000000, 0b1011100000000000, 1, 1},
  {"PUSH", PUSH, 0b1111111000001111, 0b1001001000001111, 2, 1},
  {"POP", POP, 0b1111111000001111, 0b1001000000001111, 2, 1},

  {"NOP", NOP, 0b1111111111111111, 0b0000000000000000, 1, 1},
  {"SLEEP", SLEEP, 0b1111111111111111, 0b1001010110001000, 1, 1},
  {"WDR", WDR, 0b1111111111111111, 0b1001010110101000, 1, 1},
  {"BREAK", BREAK, 0b1111111111111111, 0b1001010110011000, 0, 1},

  {"XXX", XXX, 0b1111111111111111, 0b1111111111111111, 1, 1}
};

static int opcodes_count = sizeof(opcodes) / sizeof(Instruction_t);

static uint16_t get_opcode16(void) {
  if ((mcu.pc + 1) * WORD_SIZE >= MEMORY_SIZE - 1) {
    print("Out of memory bounds!\n");
    exit(EXIT_FAILURE);
  }
  return *((uint16_t *)(mcu.memory + mcu.pc * WORD_SIZE));
}

static uint32_t get_opcode32(void) {
  if ((mcu.pc + 2) * WORD_SIZE  >= MEMORY_SIZE - 1) {
    print("Out of memory bounds!\n");
    exit(EXIT_FAILURE);
  }
  return (get_opcode16() << 16) | *(uint16_t *)(mcu.memory + (mcu.pc + 1) * WORD_SIZE);
}

static void create_lookup_table(void) {
  for (int i = 0; i < LOOKUP_SIZE; i++) {
    mcu.opcode_lookup[i] = find_instruction(i);
  }
}

static Instruction_t *find_instruction(uint16_t opcode) {
  for (int i = 0; i < opcodes_count; i++) {
    if ((opcodes[i].mask1 & opcode) == opcodes[i].mask2) {
      return opcodes + i;
    }
  }
  return opcodes + opcodes_count - 1; // XXX
}

void mcu_init(void) {
  memset(&mcu, 0, sizeof(mcu));
  mcu.R = &mcu.data_memory[0];
  mcu.IO = &mcu.R[REGISTER_COUNT];
  mcu.ext_IO = &mcu.IO[IO_REGISTER_COUNT];
  mcu.RAM = &mcu.ext_IO[EXT_IO_REGISTER_COUNT];
  mcu.sp = RAM_SIZE;
  create_lookup_table();
}

bool mcu_execute_cycle(void) {
  if (mcu.cycles > 0) {
    usleep(CLOCK_FREQ);
    mcu.cycles--;
    return true;
  }
  mcu.opcode = get_opcode16();
  mcu.instruction = mcu.opcode_lookup[mcu.opcode];
  if (mcu.skip_next) {
    mcu.pc += mcu.instruction->length;
    mcu.skip_next = false;
    return true;
  }
  print("Executing %s, cycles: %d\n", mcu.instruction->name, mcu.instruction->cycles);
  if (mcu.instruction->length == 2) {
    mcu.opcode = get_opcode32();
  }
  mcu.instruction->function(mcu.opcode);
  mcu.cycles = ((mcu.instruction->cycles || 1) - 1); // BREAK
  return !mcu.stopped;
}

void mcu_run(void) {
  while (mcu_execute_cycle());
}

void mcu_resume(void) {
  mcu.stopped = false;
  mcu.pc += 1; // skip BREAK
  mcu_run();
}

bool mcu_load_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    print("Could not open %s\n", filename);
    return false;
  }
  int length = 1024;
  char buffer[length];
  memset(buffer, 0, length);
  int memory_index = 0;
  while (fgets(buffer, length, file)) {
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
      if (memory_index >= MEMORY_SIZE) {
        print("Cannot fit the whole program in memory\n");
        free(data_buffer);
        return false;
      }
      print("Writing 0x%.4X to memory\n", word);
      memcpy(mcu.memory + memory_index, &word, sizeof(word));
      memory_index += sizeof(word);
    }
    free(data_buffer);
    memset(buffer, 0, length);
  }
  print("Done\n");
  return true;
}

bool mcu_load_code(const char *code) {
  FILE *file = fopen("_t.asm", "w");
  if (file == NULL) {
    return false;
  }
  fputs(code, file);
  fclose(file);
  int status = system("avra _t.asm > /dev/null");
  if (status == -1) {
    return false;
  }
  bool loaded = mcu_load_file("_t.hex");
  char *files[] = {"_t.asm", "_t.hex", "_t.obj", "_t.eep.hex", "_t.cof"};
  for (int i = 0; i < sizeof(files) / sizeof(char *); i++) {
    remove(files[i]);
  }
  return loaded;
}

ATmega328p_t mcu_get_copy(void) {
  return mcu;
}

static void stack_push16(uint16_t value) {
  *((uint16_t *)(mcu.RAM + mcu.sp)) = value;
  mcu.sp -= 2;
}

static void stack_push8(uint8_t value) {
  mcu.RAM[mcu.sp] = value;
  mcu.sp -= 1;
}

static uint16_t stack_pop16() {
  mcu.sp += 2;
  return *(uint16_t *)(mcu.RAM + mcu.sp);
}

static uint8_t stack_pop8() {
  mcu.sp += 1;
  return mcu.RAM[mcu.sp];
}

static uint16_t word_reg_get(uint8_t d) {
  uint16_t low = mcu.R[d];
  uint16_t high = mcu.R[d + 1];
  return (high << 8) | low;
}

static uint16_t word_reg_set(uint8_t d, uint16_t value) {
  uint16_t low = value & 0x00FF;
  uint16_t high = (value & 0xFF00) >> 8;
  mcu.R[d] = low;
  mcu.R[d + 1] = high;
}

static uint16_t X_reg_get(void) {
  return word_reg_get(26);
}

static uint16_t Y_reg_get(void) {
  return word_reg_get(28);
}

static uint16_t Z_reg_get(void) {
  return word_reg_get(30);
}

static void X_reg_set(uint16_t value) {
  word_reg_set(26, value);
}

static void Y_reg_set(uint16_t value) {
  word_reg_set(28, value);
}

static void Z_reg_set(uint16_t value) {
  word_reg_set(30, value);
}