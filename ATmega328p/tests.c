#include "atmega328p.h"
#include "tests.h"

int main(void) {
  run_test("General", 
    mcu_init();
    if (mcu_load("test.hex")) {
      mcu_start();
    }
    ATmega328p_t mcu = mcu_get_copy();
    printf("MCU PC %d\n", mcu.pc);
  )
  run_test("Code execution", 
    mcu_init();
    mcu_run_code(
      "ADD r1, r2, r3\n"
      "ADC r3, r2, r2\n"
      "BREAK"
    );
    ATmega328p_t mcu = mcu_get_copy();
    printf("MCU PC %d\n", mcu.pc);
  )
  tests_summary();
  return 0;
}