#include "atmega328p.h"
#include "instrctions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Instruction_t opcodes[] = {
  // fill in all opcodes
};

static int opcodes_count = sizeof(opcodes) / sizeof(Instruction_t);

static Instruction_t *find_opcode(uint8_t first_byte) {
  for (int i = 0; i < opcodes_count; i++) {
    if ((opcodes[i].mask1 & first_byte) == opcodes[i].mask2) {
      return &opcodes[i];
    }
  }
  // opcode not found!
  return NULL;
}

static ATmega328p_t mcu;

void mcu_init(void) {
  mcu.R = &mcu.data_memory[0];
  mcu.IO = &mcu.R[REGISTER_COUNT];
  mcu.ext_IO = &mcu.IO[IO_REGISTER_COUNT];
  mcu.RAM = &mcu.ext_IO[EXT_IO_REGISTER_COUNT];
  mcu.sp = sizeof(mcu.data_memory);
  mcu.pc = 0;
  mcu.SREG.value = 0;
  memset(mcu.data_memory, 0, sizeof(mcu.data_memory));

  load_hex_to_flash("./test.hex");
}

static bool load_hex_to_flash(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Could not open %s\n", filename);
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
    for (int i = 0; i < data_length * 2; i++) {
      data_buffer[i] = line[i];
    }
    for (int i = 0; i < data_length * 2; i += 4) {
      char low[3], high[3];
      memset(low, 0, 3);
      memset(high, 0, 3);
      sscanf(data_buffer + i, "%2s", low);
      sscanf(data_buffer + i + 2, "%2s", high);
      uint16_t word = ((uint16_t)strtol(high, 0, 16) << 8) | (uint16_t)strtol(low, 0, 16);
      if (memory_index >= MEMORY_SIZE) {
        printf("Cannot fit the whole program in memory\n");
        free(data_buffer);
        return false;
      }
      printf("Writing word 0x%.2X to flash memory\n", word);
      memcpy(mcu.memory + memory_index, &word, sizeof(word));
      memory_index += sizeof(word);
    }
    free(data_buffer);
    memset(buffer, 0, length);
  }
  printf("Done\n");
  return true;
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