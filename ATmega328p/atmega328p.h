#ifndef __ATMEGA328P_
#define __ATMEGA328P_

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t byte;
typedef uint8_t bit;
typedef uint16_t word;

#define WORD_SIZE sizeof(word)
#define REGISTER_COUNT 32
#define IO_REGISTER_COUNT 64
#define EXT_IO_REGISTER_COUNT 160
#define DATA_MEMORY_SIZE 0x0900
#define MEMORY_SIZE (32 * KB)
#define RAM_SIZE (2 * KB)
#define KB 1024

typedef union {
  // Status register flags
  struct {
    bit I : 1; // Interrupt enable
    bit T : 1; // Copy storage
    bit H : 1; // Half carry
    bit S : 1; // Sign
    bit V : 1; // Overflow
    bit N : 1; // Negative
    bit Z : 1; // Zero
    bit C : 1; // Carry
  } flags;
  byte value; // For easier clearing
} SREG_t;

typedef struct {
  SREG_t SREG;
  byte data_memory[DATA_MEMORY_SIZE]; // contains registers and RAM, allows various addressing modes
  byte ROM[KB];
  byte memory[MEMORY_SIZE];
  byte *R; // General purpose registers
  byte *IO; // IO registers
  byte *ext_IO; // External IO registers
  byte *RAM;
  uint16_t sp; // Stack pointer, 2 bytes needed to address the 2KB RAM space
  uint16_t pc; // Program counter
} ATmega328p_t;

typedef struct {
  char *name;
  void (*function)(uint32_t opcode);
  uint16_t mask1; // 1 for all fixed bits, 0 for variables
  uint16_t mask2; // 1 for all fixed 1s, 0 for all fixed 0s and variables
  uint8_t cycles;
} Instruction_t;


// API
void mcu_init(const char *filename);

static bool load_hex_to_flash(const char *filename);
static Instruction_t *find_opcode(uint16_t opcode);

static void stack_push(ATmega328p_t *mcu, uint16_t value);
static uint16_t stack_pop(ATmega328p_t *mcu);

static uint16_t X_reg_get(void);
static uint16_t Y_reg_get(void);
static uint16_t Z_reg_get(void);
static void X_reg_set(uint16_t value);
static void Y_reg_set(uint16_t value);
static void Z_reg_set(uint16_t value);

#endif // __ATMEGA328P_