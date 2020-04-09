#include "atmega328p.h"
#include "tests.h"

int main(void) {
  run_test("General", 
    mcu_init();
    if (mcu_load_file("test.hex")) {
      mcu_start();
    }
  )
  run_test("Code execution", 
    mcu_init();
    mcu_load_code(
      "ADD r1, r2, r3\n"
      "ADC r3, r2, r2\n"
      "BREAK\n"
      "ADD r0, r0, r0\n"
      "BREAK"
    );
    mcu_start();
    printf("PC = %d\n", mcu_get_copy().pc);
    mcu_resume();
    printf("PC = %d\n", mcu_get_copy().pc);
  )
  tests_summary();
  return 0;
}