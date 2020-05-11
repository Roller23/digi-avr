import ctypes

# https://stackoverflow.com/questions/4351721/python-ctypes-passing-a-struct-to-a-function-as-a-pointer-to-get-back-data

class ATmega328p_t(ctypes.Structure):
  _fields_ = [
    ("SREG", ctypes.c_uint8),
    ("SR", ctypes.c_uint8),
    ("data_memory", ctypes.c_uint8 * 0x0900),
    ("ROM", ctypes.c_uint8 * 1024)
  ]

# typedef struct {
#   SREG_t SREG;
#   MCUSR_t SR; // MCU status register
#   byte data_memory[DATA_MEMORY_SIZE]; // contains registers and RAM, allows various addressing modes
#   byte ROM[KB];
#   byte program_memory[PROGRAM_MEMORY_SIZE];
#   byte *boot_section; // Last 512 bytes of program memory
#   byte *R; // General purpose registers
#   byte *IO; // IO registers
#   byte *ext_IO; // External IO registers
#   byte *RAM;
#   uint16_t sp; // Stack pointer, 2 bytes needed to address the 2KB RAM space
#   uint16_t pc; // Program counter
#   bool skip_next;
#   bool sleeping;
#   bool stopped;
#   bool handle_interrupt;
#   bool auto_execute;
#   uint16_t interrupt_address;
#   uint16_t cycles;
#   uint32_t opcode;
#   Instruction_t *instruction;
#   void (*exception_handler)(void);
# } ATmega328p_t;

# typedef struct {
#   char *name;
#   void (*execute)(uint32_t opcode);
#   uint16_t mask1; // 1 for all fixed bits, 0 for variables
#   uint16_t mask2; // 1 for all fixed 1s, 0 for all fixed 0s and variables
#   uint16_t cycles;
#   uint16_t length; // in WORDs
# } Instruction_t;

#define WORD_SIZE sizeof(word)
#define REGISTER_COUNT 32
#define IO_REGISTER_COUNT 64
#define EXT_IO_REGISTER_COUNT 160
#define DATA_MEMORY_SIZE 0x0900
#define PROGRAM_MEMORY_SIZE (32 * KB)
#define BOOTLOADER_SIZE (KB / 2)
#define RAM_SIZE (2 * KB)
#define KB 1024
#define LOOKUP_SIZE 0xFFFF