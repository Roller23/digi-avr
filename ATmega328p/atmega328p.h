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
#define LOOKUP_SIZE 0xFFFF

typedef union {
  // Status register flags
  struct {
    bit C : 1; // Carry
    bit Z : 1; // Zero
    bit N : 1; // Negative
    bit V : 1; // Overflow
    bit S : 1; // Sign
    bit H : 1; // Half carry
    bit T : 1; // Copy storage
    bit I : 1; // Interrupt enable
  } flags;
  byte value;
} SREG_t;

typedef union {
  // MCU status register flags
  struct {
    bit reserved4 : 1;
    bit reserved3 : 1;
    bit reserved2 : 1;
    bit reserved1 : 1;
    // reset flags
    bit WDRF : 1; // Watchdog
    bit BORF : 1; // Brown-out
    bit EXTRF : 1; // External
    bit PORF : 1; // Power-on
  } flags;
  byte value;
} MCUSR_t;

typedef struct {
  char *name;
  void (*function)(uint32_t opcode);
  uint16_t mask1; // 1 for all fixed bits, 0 for variables
  uint16_t mask2; // 1 for all fixed 1s, 0 for all fixed 0s and variables
  uint16_t cycles;
  uint16_t length; // in WORDs
} Instruction_t;

typedef struct {
  SREG_t SREG;
  MCUSR_t SR; // MCU status register
  byte data_memory[DATA_MEMORY_SIZE]; // contains registers and RAM, allows various addressing modes
  byte ROM[KB];
  byte memory[MEMORY_SIZE];
  byte *R; // General purpose registers
  byte *IO; // IO registers
  byte *ext_IO; // External IO registers
  byte *RAM;
  uint16_t sp; // Stack pointer, 2 bytes needed to address the 2KB RAM space
  uint16_t pc; // Program counter
  bool skip_next;
  bool sleeping;
  bool stopped;
  uint16_t cycles;
  Instruction_t *opcode_lookup[LOOKUP_SIZE];
} ATmega328p_t;

// API
void mcu_init(void);
bool mcu_load_file(const char *filename);
bool mcu_load_code(const char *code);
void mcu_run(void);
bool mcu_execute_cycle(void);
void mcu_resume(void);
ATmega328p_t mcu_get_copy(void);

static void create_lookup_table(void);
static Instruction_t *find_instruction(uint16_t opcode);
static uint16_t get_opcode16(void);
static uint32_t get_opcode32(void);

static void stack_push16(uint16_t value);
static void stack_push8(uint8_t value);
static uint16_t stack_pop16(void);
static uint8_t stack_pop8(void);

static uint16_t word_reg_get(uint8_t d);
static uint16_t word_reg_set(uint8_t d, uint16_t value);
static uint16_t X_reg_get(void);
static uint16_t Y_reg_get(void);
static uint16_t Z_reg_get(void);
static void X_reg_set(uint16_t value);
static void Y_reg_set(uint16_t value);
static void Z_reg_set(uint16_t value);

#endif // __ATMEGA328P_