#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include "atmega328p.h"

static ATmega328p_t mcu;

static void show_state(void) {
  mcu_get_copy(&mcu);
  printf("Registers:\n");
  for (int i = 0; i < 32; i++) {
    printf("R[%.02d] = %*d%s", i, 3, mcu.R[i], i % 2 ? "\n" : "  ");
  }
  printf("PC    = 0x%*x  Opcode = 0x%.4X\n", 3, mcu.pc * 2, mcu.opcode);
  printf("Instruction = %s\n", mcu.instruction->name);
}

void *handle_stdin(void *arg) {
  while (true) {
    int c = getchar();
    if (c == 'i') {
      int vector = 0;
      printf("Select the vector: ");
      int sc = scanf("%d", &vector);
      if (sc == 0) {
        continue;
      }
      mcu_send_interrupt((Interrupt_vector_t)vector);
    }
    if (c == 'q') {
      break;
    }
  }
  return 0;
}

int main(void) {
  pthread_t thread;
  pthread_create(&thread, NULL, handle_stdin, NULL);
  mcu_init();
  mcu_load_ihex("program.hex");
  mcu_run();
  return 0;
}