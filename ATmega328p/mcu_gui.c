#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "atmega328p.h"

static ATmega328p_t mcu;

static void show_state(void) {
  mcu_get_copy(&mcu);
  printf("Registers:\n");
  for (int i = 0; i < 32; i++) {
    printf("R[%.02d] = %*d%s", i, 3, mcu.R[i], i % 2 ? "\n" : "  ");
  }
  printf("PC    = %*d  Opcode = 0x%.4X\n", 3, mcu.pc, mcu.opcode);
  printf("Instruction = %s\n", mcu.instruction->name);
}

int main(void) {
  mcu_init();
  mcu_load_file("program.hex");
  while (mcu_execute_cycle());
  // while (true) {
  //   int res = system("clear");
  //   bool running = mcu_execute_cycle();
  //   show_state();
  //   if (!running) {
  //     break;
  //   }
  //   getchar();
  // }
  return 0;
}