#include "atmega328p.h"

int main(void) {
  if (mcu_init("test.hex")) {
    mcu_start();
  }
  return 0;
}