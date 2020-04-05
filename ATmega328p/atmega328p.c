#include "atmega328p.h"
#include "instrctions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Instruction_t opcodes[] = {
  {"ADD",ADD,0b1111110000000000,0b0000110000000000,1},
  {"ADC",ADC,0b1111110000000000,0b0001110000000000,1},
  {"ADIW",ADIW,0b1111111100000000,0b1001011000000000,2},
  {"SUB",SUB,0b1111110000000000,0b0001100000000000,1},
  {"SUBI",SUBI,0b1111000000000000,0b0101000000000000,1},
  {"SBC",SBC,0b1111110000000000,0b0000100000000000,1},
  {"SBCI",SBCI,0b1111000000000000,0b0100000000000000,1},
  {"SBIW",SBIW,0b1111111100000000,0b1001011100000000,2},
  {"AND",AND,0b1111110000000000,0b0010000000000000,1},
  {"ANDI",ANDI,0b1111000000000000,0b0111000000000000,1},
  {"OR",OR,0b1111110000000000,0b0010100000000000,1},
  {"ORI",ORI,0b1111000000000000,0b0110000000000000,1},
  {"EOR",EOR,0b1111110000000000,0b0010010000000000,1},
  {"COM",COM,0b1111111000001111,0b1001010000000000,1},
  {"NEG",NEG,0b1111111000001111,0b1001010000000001,1},
  {"INC",INC,0b1111111000001111,0b1001010000000011,1},
  {"DEC",DEC,0b1111111000001111,0b1001010000001010,1},
  {"SER",SER,0b1111111100001111,0b1110111100001111,1},
  {"MUL",MUL,0b1111110000000000,0b1001110000000000,2},//??1
  {"MULS",MULS,0b1111111100000000,0b0000001000000000,2},//??1
  {"MULSU",MULSU,0b1111111110001000,0b0000001100000000,2},
  {"FMUL",FMUL,0b1111111110001000,0b0000001100001000,2},
  {"FMULS",FMULS,0b1111111110001000,0b0000001110000000,2},
  {"FMULSU",FMULSU,0b1111111110001000,0b0000001110001000,2},

  {"RJMP",RJMP,0b1111000000000000,0b1100000000000000,2},
  {"IJMP",IJMP,0b1111111111111111,0b1001010000001001,2},
  {"JMP",JMP,0b1111111000001110,0b1001010000001100,3},
  {"RCALL",RCALL,0b1111000000000000,0b1101000000000000,3},
  {"ICALL",ICALL,0b1111111111111111,0b1001010100001001,3},
  {"CALL",CALL,0b1111111000001110,0b1001010000001110,4},
  {"RET",RET,0b1111111111111111,0b1001010100001000,4},
  {"RETI",RETI,0b1111111111111111,0b1001010100011000,4},
  {"CPSE",CPSE,0b1111110000000000,0b0001000000000000,1},//2/3
  {"CP",CP,0b1111110000000000,0b0001010000000000,1},
  {"CPC",CPC,0b1111110000000000,0b0000010000000000,1},
  {"CPI",CPI,0b1111000000000000,0b0011000000000000,1},
  {"SBRC",SBRC,0b1111111000001000,0b1111110000000000,1},//2/3
  {"SBRS",SBRS,0b1111111000001000,0b1111111000000000,1},//2/3
  {"SBIC",SBIC,0b1111111100000000,0b1001100100000000,1},//2/3
  {"SBIS",SBIS,0b1111111100000000,0b1001101100000000,1},//2/3
  {"BRBS",BRBS,0b1111110000000000,0b1111000000000000,1},//2
  {"BRBC",BRBC,0b1111110000000000,0b1111010000000000,1},//2

  {"SBI",SBI,0b1111111100000000,0b1001101000000000,2},
  {"CBI",CBI,0b1111111100000000,0b1001100000000000,2},
  {"LSR",LSR,0b1111111000001111,0b1001010000000110,1},
  {"ROR",ROR,0b1111111000001111,0b1001010000000111,1},
  {"ASR",ASR,0b1111111000001111,0b1001010000000101,1},
  {"SWAP",SWAP,0b1111111000001111,0b1001010000000010,1},
  {"BSET",BSET,0b1111111110001111,0b1001010000001000,1},
  {"BCLR",BCLR,0b1111111110001111,0b1001010010001000,1},
  {"BST",BST,0b1111111000001000,0b1111101000000000,1},
  {"BLD",BLD,0b1111111000001000,0b1111100000000000,1},

  {"MOV",MOV,0b1111110000000000,0b0010110000000000,1},
  {"MOVW",MOVW,0b1111111100000000,0b0000000100000000,1},
  {"LDI",LDI,0b1111000000000000,0b1110000000000000,2},//???1
  //....
  {"IN",IN,0b1111100000000000,0b1011000000000000,1},
  {"OUT",OUT,0b1111100000000000,0b1011100000000000,1},
  {"PUSH",PUSH,0b1111111000001111,0b1001001000001111,2},
  {"POP",POP,0b1111111000001111,0b1001000000001111,2},

  {"NOP",NOP,0b1111111111111111,0b0000000000000000,1},
  {"SLEEP",SLEEP,0b1111111111111111,0b1001010110001000,1},
  {"WDR",WDR,0b1111111111111111,0b1001010110101000,1},
  {"BREAK",BREAK,0b1111111111111111,0b1001010110011000,0}//???
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
