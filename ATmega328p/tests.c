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
  run_test("MOVW",
    execute(
      "LDI R20, 5\n"
      "LDI R21, 10\n"
      "MOVW R31:R30, R21:R20\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[30] == 5);
    assert(mcu.R[31] == 10);
  )
  run_test("BCLR and BSET",
    execute(
      "BSET 0\n"
      "BSET 1\n"
      "BSET 2\n"
      "BSET 3\n"
      "BSET 4\n"
      "BSET 5\n"
      "BSET 6\n"
      "BSET 7\n"
      "BREAK\n"
      "BCLR 0\n"
      "BCLR 1\n"
      "BCLR 2\n"
      "BCLR 3\n"
      "BCLR 4\n"
      "BCLR 5\n"
      "BCLR 6\n"
      "BCLR 7\n"
      "BREAK"
    );
    assert(mcu_get_copy().SREG.value == 0xFF);
    mcu_resume();
    assert(mcu_get_copy().SREG.value == 0x00);
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
      "JMP halt\n" // 2 words
      "NOP\n"
      "NOP\n"
      "LDI R23, 0\n"
      "NOP\n"
      "NOP\n"
      "halt: NOP\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[23] == 55);
    assert(mcu.pc == 9);
  )
  run_test("RCALL and RET",
    execute(
      "LDI R20, 5\n"
      "RCALL routine\n"
      "JMP halt\n"
      "routine: LDI R20, 10\n"
      "RET\n"
      "halt: NOP\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[20] == 10);
  )
  run_test("ICALL and RET",
    execute(
      "LDI R20, 5\n"
      "LDI R30, 5\n" // low byte of Z reg
      "ICALL\n"
      "JMP halt\n"
      "routine: LDI R20, 10\n"
      "RET\n"
      "halt: NOP\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[20] == 10);
  )
  run_test("RCALL and RET",
    execute(
      "LDI R20, 5\n"
      "CALL routine\n"
      "JMP halt\n"
      "routine: LDI R20, 10\n"
      "RET\n"
      "halt: NOP\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[20] == 10);
  )
  run_test("RETI",
    execute(
      "CLI\n"
      "RCALL interrupt\n"
      "JMP halt\n"
      "interrupt: NOP\n"
      "RETI\n"
      "halt: NOP\n"
      "BREAK"
    );
    assert(mcu_get_copy().SREG.flags.I == 1);
  )
  run_test("CPSE",
    execute(
      "LDI R20, 5\n"
      "LDI R21, 10\n"
      "CPSE R20, R21\n"
      "LDI R21, 5\n"
      "CPSE R20, R21\n"
      "LDI R21, 30\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[21] == 5);
  )
  tests_summary();
  return 0;
}