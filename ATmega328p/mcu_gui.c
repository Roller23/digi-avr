#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "atmega328p.h"

static void show_state(void) {
  ATmega328p_t mcu = mcu_get_copy();
  printf("Registers:\n");
  for (int i = 0; i < 32; i++) {
    printf("R[%.02d] = %*d%s", i, 3, mcu.R[i], i % 2 ? "\n" : "  ");
  }
  printf("PC    = %*d  Opcode = 0x%.4X\n", 3, mcu.pc, mcu.opcode);
  printf("Instruction = %s\n", mcu.instruction->name);
}

int main(void) {
  mcu_init();
  // mcu_load_code(
  //   "LDI R20, 5\n"
  //   "LDI R23, 15\n"
  //   "CALL function\n"
  //   "JMP halt\n"
  //   "function: ADD R23, R20, R24\n"
  //   "RET\n"
  //   "halt: NOP\n"
  //   "BREAK"
  // );
  mcu_load_file("program.hex");
  while (true) {
    int res = system("clear");
    bool running = mcu_execute_cycle();
    show_state();
    if (!running) {
      break;
    }
    getchar();
  }
  return 0;
}