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
  ATmega328p_t mcu;
  run_test("LDI",
    execute(
      "LDI R16, 5\n"
      "LDI R17, 254\n"
      "LDI R18, 300\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[16] == mcu.R[17]);
  )
  run_test("MOVW",
    execute(
      "LDI R20, 5\n"
      "LDI R21, 10\n"
      "MOVW R31:R30, R21:R20\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[30] == 5);
    assert(mcu.R[31] == 10);
  )
  run_test("OUT",
    execute(
      "LDI R20, 15\n"
      "OUT 10, R20\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.IO[10] == 15);
  )
  run_test("IN",
    execute(
      "LDI R20, 15\n"
      "OUT 10, R20\n"
      "IN R23, 10\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[23] == 15);
  )
  run_test("SER",
    execute(
      "LDI R20, 0\n"
      "LDI R23, 0\n"
      "SER R20\n"
      "SER R23\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
    assert(mcu.SREG.value == 0xFF);
    mcu_resume();
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
    assert(mcu.pc == 4);
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
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == 10);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == 10);
  )
  run_test("CALL and RET",
    execute(
      "LDI R20, 5\n"
      "CALL routine\n"
      "JMP halt\n"
      "routine: LDI R20, 10\n"
      "RET\n"
      "halt: NOP\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == 10);
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
    mcu_get_copy(&mcu);
    assert(mcu.SREG.flags.I == 1);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == 10);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[21] == 5);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[21] == 5);
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
    mcu_get_copy(&mcu);
    assert(mcu.IO[10] == 1);
    mcu_resume();
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[23] == 123);
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
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == 5);
    assert(mcu.R[21] == 15);
  )
  run_test("SWAP",
    execute(
      "LDI R20, 0xFA\n"
      "SWAP R20\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == 0xAF);
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
    mcu_get_copy(&mcu);
    assert(mcu.SREG.flags.I == 1);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.SREG.flags.I == 0);
    mcu_resume();
    mcu_get_copy(&mcu);
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
    mcu_get_copy(&mcu);
    assert(mcu.SREG.flags.T == 1);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[21] == 1);
  )
  run_test("ST X",
    execute(
      "LDI R20, 10\n"
      "ST X+, R20\n"
      "ST X, R20\n"
      "BREAK\n"
      "LDI R20, 15\n"
      "ST -X, R20\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[26] == 1); // low byte of the X register
    assert(mcu.R[0] == 10);
    assert(mcu.R[1] == 10);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[0] == 15);
    assert(mcu.R[26] == 0);
  )
  run_test("ST Y",
    execute(
      "LDI R20, 15\n"
      "STD Y+4, R20\n"
      "BREAK\n"
      "LDI R20, 20\n"
      "ST Y+, R20\n"
      "ST Y, R20\n"
      "BREAK\n"
      "LDI R20, 5\n"
      "ST -Y, R20\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[4] == 15);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[0] == 20);
    assert(mcu.R[1] == 20);
    assert(mcu.R[28] == 1); // low byte of the Y register
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[0] == 5);
    assert(mcu.R[28] == 0);
  )
  run_test("ST Z",
    execute(
      "LDI R20, 15\n"
      "STD Z+4, R20\n"
      "BREAK\n"
      "LDI R20, 20\n"
      "ST Z+, R20\n"
      "ST Z, R20\n"
      "BREAK\n"
      "LDI R20, 5\n"
      "ST -Z, R20\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[4] == 15);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[0] == 20);
    assert(mcu.R[1] == 20);
    assert(mcu.R[30] == 1); // low byte of the Z register
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[0] == 5);
    assert(mcu.R[30] == 0);
  )
  run_test("STS",
    execute(
      "LDI R23, 123\n"
      "STS 3, R23\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[3] == mcu.R[23]);
    assert(mcu.R[3] == 123);
  )
  run_test("LPM",
    execute(
      "LDI R30, 4\n" // Z = 4
      "LPM\n" // R0 = program[Z] (LPM R20, Z+) - 0x9145
      "LPM R20, Z+\n"
      "LPM R21, Z\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[0] == 0x45);
    assert(mcu.R[20] == 0x45);
    assert(mcu.R[21] == 0x91);
    assert(mcu.R[30] == 5);
  )
  run_test("LD X",
    execute(
      "LDI R20, 123\n"
      "LDI R21, 111\n"
      "LDI R26, 20\n"
      "LD R16, X+\n"
      "LD R17, X\n"
      "BREAK\n"
      "LD R18, -X\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[16] == 123);
    assert(mcu.R[17] == 111);
    assert(mcu.R[26] == 21); // low byte of X register
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[18] == 123);
    assert(mcu.R[26] == 20);
  )
  run_test("LD Y",
    execute(
      "LDI R20, 123\n"
      "LDI R21, 111\n"
      "LDI R28, 20\n"
      "LD R16, Y+\n"
      "LD R17, Y\n"
      "BREAK\n"
      "LD R18, -Y\n"
      "BREAK\n"
      "LDI R28, 10\n"
      "LDD R19, Y+10\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[16] == 123);
    assert(mcu.R[17] == 111);
    assert(mcu.R[28] == 21); // low byte of Y register
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[18] == 123);
    assert(mcu.R[28] == 20);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[19] == 123);
    assert(mcu.R[28] == 10);
  )
  run_test("LD Z",
    execute(
      "LDI R20, 123\n"
      "LDI R21, 111\n"
      "LDI R30, 20\n"
      "LD R16, Z+\n"
      "LD R17, Z\n"
      "BREAK\n"
      "LD R18, -Z\n"
      "BREAK\n"
      "LDI R30, 10\n"
      "LDD R19, Z+10\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[16] == 123);
    assert(mcu.R[17] == 111);
    assert(mcu.R[30] == 21); // low byte of Z register
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[18] == 123);
    assert(mcu.R[30] == 20);
    mcu_resume();
    mcu_get_copy(&mcu);
    assert(mcu.R[19] == 123);
    assert(mcu.R[30] == 10);
  )
  run_test("LDS",
    execute(
      "LDI R23, 123\n"
      "LDS R20, 23\n"
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[20] == mcu.R[23]);
    assert(mcu.R[20] == 123);
  )
  run_test("SPM",
    // overwrite NOP by 0xE005
    execute(
      // store 0xE005 to R0:R1
      "LDI R20, 0x0005\n"
      "LDI R21, 0x00E0\n"
      "ST Z+, R20\n"
      "ST Z, R21\n"
      // store NOP address
      "LDI R30, 12\n"
      "SPM\n" // overwrite NOP by 0xE005
      "NOP\n" // should be overwritten 'LDI R16, 5'
      "BREAK"
    );
    mcu_get_copy(&mcu);
    assert(mcu.R[16] == 5);
  )
  tests_summary();
  return 0;
}