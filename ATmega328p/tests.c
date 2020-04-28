#include <stdlib.h>

#include "atmega328p.h"
#include "tests.h"

static void execute(const char *code) {
  mcu_init();
  if (!mcu_load_code(code)) {
    exit(EXIT_FAILURE);
  }
  mcu_run();
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
  run_test("OUT",
    execute(
      "LDI R20, 15\n"
      "OUT 10, R20\n"
      "BREAK"
    );
    assert(mcu_get_copy().IO[10] == 15);
  )
  run_test("IN",
    execute(
      "LDI R20, 15\n"
      "OUT 10, R20\n"
      "IN R23, 10\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[23] == 15);
  )
  run_test("SER",
    execute(
      "LDI R20, 0\n"
      "LDI R23, 0\n"
      "SER R20\n"
      "SER R23\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[20] == 0xFF);
    assert(mcu.R[23] == 0xFF);
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
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.SREG.value == 0xFF);
    mcu_resume();
    mcu = mcu_get_copy();
    assert(mcu.SREG.value == 0x00);
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
  run_test("PUSH and POP",
    execute(
      "LDI R20, 0\n"
      "call function\n"
      "JMP halt\n"
      "function: LDI R21, 10\n"
      "PUSH R21\n"
      "POP R20\n"
      "RET\n"
      "halt: NOP\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[20] == 10);
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
  run_test("SBRC and SBRS",
    execute(
      "LDI R21, 5\n"
      "LDI R20, 0xFF\n"
      "SBRC R20, 5\n"
      "SBRS R20, 5\n"
      "LDI R21, 10\n"
      "LDI R20, 0\n"
      "SBRC R20, 3\n"
      "LDI R21, 10\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[21] == 5);
  )
  run_test("SBI and CBI",
    execute(
      "LDI R20, 0\n"
      "OUT 10, R20\n"
      "SBI 10, 0\n"
      "BREAK\n"
      "LDI R20, 0xFF\n"
      "OUT 11, R20\n"
      "CBI 11, 5\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.IO[10] == 1);
    mcu_resume();
    mcu = mcu_get_copy();
    assert(mcu.IO[11] == 0b11011111);
  )
  run_test("SBIC and SBIS",
    execute(
      "LDI R23, 123\n"
      "LDI R20, 0xFF\n"
      "LDI R21, 0\n"
      "OUT 10, R20\n"
      "OUT 11, R21\n"
      "CBI 10, 5\n"
      "SBI 11, 3\n"
      "SBIC 10, 5\n"
      "LDI R23, 0\n"
      "SBIS 11, 3\n"
      "LDI R23, 0\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[23] == 123);
  )
  run_test("BRBS and BRBC",
    execute(
      "LDI R20, 5\n"
      "LDI R21, 5\n"
      "CLI\n"
      "BRBS 7, fail\n"
      "SEI\n"
      "BRBC 7, fail\n"
      "BRBS 7, continue\n"
      "JMP fail\n"
      "fail: LDI R20, 10\n"
      "BREAK\n"
      "continue: LDI R21, 15\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[20] == 5);
    assert(mcu.R[21] == 15);
  )
  run_test("SWAP",
    execute(
      "LDI R20, 0xFA\n"
      "SWAP R20\n"
      "BREAK"
    );
    assert(mcu_get_copy().R[20] == 0xAF);
  )
  run_test("BSET and BCLR",
    execute(
      "BSET 7\n"
      "BREAK\n"
      "BCLR 7\n"
      "BREAK\n"
      "BSET 7\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.SREG.flags.I == 1);
    mcu_resume();
    mcu = mcu_get_copy();
    assert(mcu.SREG.flags.I == 0);
    mcu_resume();
    mcu = mcu_get_copy();
    assert(mcu.SREG.flags.I == 1);
  )
  run_test("BST and BLD",
    execute(
      "BCLR 1\n" // clear T
      "LDI R20, 1\n" // T = R20(0)
      "BST R20, 0\n" // T = R20(0)
      "BREAK\n"
      "LDI R21, 0\n"
      "BLD R21, 0\n" // R21(0) = T
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.SREG.flags.T == 1);
    mcu_resume();
    mcu = mcu_get_copy();
    assert(mcu.R[21] == 1);
  )
  run_test("ST",
    execute(
      "LDI R20, 10\n"
      "ST X+, R20\n"
      "ST X, R20\n"
      "BREAK\n"
      "LDI R20, 15\n"
      "ST -X, R20\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    assert(mcu.R[0] == 10);
    assert(mcu.R[1] == 10);
    mcu_resume();
    mcu = mcu_get_copy();
    assert(mcu.R[0] == 15);
  )
  tests_summary();
  return 0;
}