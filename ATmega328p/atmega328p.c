#include "atmega328p.h"
#include "instructions.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define b_get(number, n) (number & (1LU << n))

static ATmega328p_t mcu;

static inline void ADD(uint32_t opcode) {
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode,8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode,9) >> 5;
  uint8_t result = mcu.R[reg_d] + mcu.R[reg_r];
  mcu.SREG.flags.H = !!(b_get(mcu.R[reg_d],3) & b_get(mcu.R[reg_r],3) | b_get(mcu.R[reg_r],3) & ~b_get(result,3) | ~b_get(result,3) & b_get(mcu.R[reg_d],3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d],7) & b_get(mcu.R[reg_r],7) & ~b_get(result,7) | ~b_get(mcu.R[reg_d],7) & ~b_get(mcu.R[reg_r],7) & b_get(result,7));
  mcu.SREG.flags.N = !!b_get(result,7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(b_get(mcu.R[reg_d],7) & b_get(mcu.R[reg_r],7) | b_get(mcu.R[reg_r],7) & ~b_get(result,7) | ~b_get(result,7) & b_get(mcu.R[reg_d],7));
  mcu.R[reg_d] = result;
  mcu.pc += WORD_SIZE;
}
static inline void ADC(uint32_t opcode){
  uint8_t reg_d = (opcode & 0xF0) >> 4;
  reg_d |= b_get(opcode,8) >> 4;
  uint8_t reg_r = (opcode & 0xF);
  reg_r |= b_get(opcode,9) >> 5;
  uint8_t result = mcu.R[reg_d] + mcu.R[reg_r] + mcu.SREG.flags.C;
  mcu.SREG.flags.H = !!(b_get(mcu.R[reg_d],3) & b_get(mcu.R[reg_r],3) | b_get(mcu.R[reg_r],3) & ~b_get(result,3) | ~b_get(result,3) & b_get(mcu.R[reg_d],3));
  mcu.SREG.flags.V = !!(b_get(mcu.R[reg_d],7) & b_get(mcu.R[reg_r],7) & ~b_get(result,7) | ~b_get(mcu.R[reg_d],7) & ~b_get(mcu.R[reg_r],7) & b_get(result,7));
  mcu.SREG.flags.N = !!b_get(result,7);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !!(b_get(mcu.R[reg_d],7) & b_get(mcu.R[reg_r],7) | b_get(mcu.R[reg_r],7) & ~b_get(result,7) | ~b_get(result,7) & b_get(mcu.R[reg_d],7));
  mcu.R[reg_d] = result;
  mcu.pc += WORD_SIZE;
}
static inline void ADIW(uint32_t opcode){
  uint8_t k = (opcode & 0xF);
  k |= b_get(opcode,6) >> 2;
  k |= b_get(opcode,7) >> 2;
  uint8_t reg_d = ((!!b_get(opcode,5)) << 1) | (!!b_get(opcode,4));
  reg_d = reg_d * 2 + 24;
  uint16_t rd = ((uint16_t)mcu.R[reg_d+1] << 8) | mcu.R[reg_d];
  uint16_t result = rd + k;
  mcu.SREG.flags.V = !b_get(rd,15) & !!b_get(result,15);
  mcu.SREG.flags.N = !!b_get(result,15);
  mcu.SREG.flags.Z = (result == 0);
  mcu.SREG.flags.C = !b_get(result,15) &  !!b_get(rd,15);
  mcu.SREG.flags.S = mcu.SREG.flags.N ^ mcu.SREG.flags.V;
  mcu.R[reg_d] = result;
  mcu.R[reg_d+1] = result >> 8;
  mcu.pc += WORD_SIZE;
}

static inline void RJMP(uint32_t opcode) {
  // 1100 kkkk kkkk kkkk
  // Relative jump to PC + k + 1
  uint16_t k = opcode & 0x0FFF;
  mcu.pc += k + WORD_SIZE;
}

static inline void IJMP(uint32_t opcode) {
  // Indirect jump to address at Z register
  mcu.pc = Z_reg_get();
}

static inline void JMP(uint32_t opcode) {
  // 1001 010k kkkk 110k
  // kkkk kkkk kkkk kkkk
  // Jump to address k, PC = k
  uint32_t k = opcode & 0xFFFF;
  k |= b_get(opcode, 16);
  k |= (opcode & 0xF00000) >> 3;
  k |= b_get(opcode, 24) >> 3;
  mcu.pc = k;
}

static inline void RCALL(uint32_t opcode) {
  // 1101 kkkk kkkk kkkk
  // Jump to address + 1 + PC, push current PC + 1 onto stack (relative call)
  uint16_t k = opcode & 0xFFF;
  stack_push(mcu.pc + WORD_SIZE);
  mcu.pc += k + WORD_SIZE;
}

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

static Instruction_t *find_opcode(uint16_t opcode) {
  for (int i = 0; i < opcodes_count; i++) {
    if ((opcodes[i].mask1 & opcode) == opcodes[i].mask2) {
      return &opcodes[i];
    }
  }
  // opcode not found!
  return NULL;
}

void mcu_init(const char *filename) {
  mcu.R = &mcu.data_memory[0];
  mcu.IO = &mcu.R[REGISTER_COUNT];
  mcu.ext_IO = &mcu.IO[IO_REGISTER_COUNT];
  mcu.RAM = &mcu.ext_IO[EXT_IO_REGISTER_COUNT];
  mcu.sp = &mcu.RAM[RAM_SIZE];
  mcu.pc = 0;
  mcu.SREG.value = 0;
  memset(mcu.data_memory, 0, DATA_MEMORY_SIZE);
  memset(mcu.memory, 0, MEMORY_SIZE);
  load_hex_to_flash(filename);
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

static void stack_push(uint16_t value) {
  mcu.sp -= sizeof(value);
  mcu.RAM[mcu.sp] = value;
}

static uint16_t stack_pop(void) {
  uint16_t value = mcu.RAM[mcu.sp];
  mcu.sp += sizeof(value);
  return value;
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
