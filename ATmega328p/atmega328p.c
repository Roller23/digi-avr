#include "atmega328p.h";
#include <string.h>

static ATmega328p_t mcu;

static void mcu_init(void) {
  mcu.R = &mcu.data_memory[0];
  mcu.IO = &mcu.R[REGISTER_COUNT];
  mcu.ext_IO = &mcu.IO[IO_REGISTER_COUNT];
  mcu.RAM = &mcu.ext_IO[EXT_IO_REGISTER_COUNT];
  mcu.sp = &mcu.RAM[RAM_SIZE];
  mcu.pc = 0;
  mcu.SREG.value = 0;
  memset(mcu.data_memory, 0, sizeof(mcu.data_memory));
}

static uint16_t X_reg_get(void) {
  uint16_t low = mcu.R[26];
  uint16_t high = mcu.R[27];
  return (high << 8) | low;
}

static uint16_t Y_reg_get(void) {
  uint16_t low = mcu.R[28];
  uint16_t high = mcu.R[29];
  return (high << 8) | low;
}

static uint16_t Z_reg_get(void) {
  uint16_t low = mcu.R[30];
  uint16_t high = mcu.R[31];
  return (high << 8) | low;
}

static void X_reg_set(uint16_t value) {
  uint16_t low = value & 0x00FF;
  uint16_t high = (value & 0xFF00) >> 8;
  mcu.R[26] = low;
  mcu.R[27] = high;
}

static void Y_reg_set(uint16_t value) {
  uint16_t low = value & 0x00FF;
  uint16_t high = (value & 0xFF00) >> 8;
  mcu.R[28] = low;
  mcu.R[29] = high;
}

static void Z_reg_set(uint16_t value) {
  uint16_t low = value & 0x00FF;
  uint16_t high = (value & 0xFF00) >> 8;
  mcu.R[30] = low;
  mcu.R[31] = high;
}