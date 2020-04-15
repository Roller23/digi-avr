#include <stdlib.h>

#include "atmega328p.h"
#include "tests.h"

static void execute(const char *code) {
  mcu_init();
  if (!mcu_load_code(code)) {
    exit(EXIT_FAILURE);
  }
  mcu_start();
}

int main(void) {
  run_test("LDI", 
    execute(
      "LDI R16, 5\n"
      "LDI R17, 254\n"
      "LDI R18, 300\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[16] == 5);
    assert(mcu.R[17] == 254);
    assert(mcu.R[18] == (uint8_t)300);
  )
  run_test("MOV", 
    execute(
      "LDI R17, 0\n"
      "LDI R16, 5\n"
      "MOV R17, R16\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[16] == mcu.R[17]);
  )
  run_test("RJMP", 
    execute(
      "LDI r23, 0\n"
      "RJMP ok\n"
      "LDI r23, 1\n"
      "ok: NOP\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[23] == 0);
    assert(mcu.pc == 4);
  )
  run_test("IJMP",
    // TO DO: test again with MOVW instead of LDI
    execute(
      "LDI R23, 10\n"
      "LDI R30, 4\n" // low byte of the Z Reg
      "IJMP\n" // PC = 4
      "LDI R23, 13\n"
      "BREAK" // 4
    );
    assert(mcu_get_copy().pc == 4);
  )
  run_test("JMP",
    execute(
      "LDI R23, 55\n"
      "JMP halt\n"
      "NOP\n"
      "NOP\n"
      "LDI R23, 0\n"
      "NOP\n"
      "NOP\n"
      "halt: NOP\n"
      "BREAK"
    );
  )
  tests_summary();
  return 0;
}