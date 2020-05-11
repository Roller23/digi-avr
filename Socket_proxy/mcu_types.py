import ctypes

# https://stackoverflow.com/questions/4351721/python-ctypes-passing-a-struct-to-a-function-as-a-pointer-to-get-back-data

# https://stackoverflow.com/questions/50043861/python-ctypes-passing-pointer-for-data

class Instruction_t(ctypes.Structure):
  _fields_ = [
    ("name", ctypes.POINTER(ctypes.c_char)),
    ("execute", ctypes.POINTER(ctypes.c_int)),
    ("mask1", ctypes.c_uint16),
    ("mask2", ctypes.c_uint16),
    ("cycles", ctypes.c_uint16),
    ("length", ctypes.c_uint16)
  ]

class ATmega328p_t(ctypes.Structure):
  _fields_ = [
    ("SREG", ctypes.c_uint8),
    ("SR", ctypes.c_uint8),
    ("data_memory", ctypes.c_uint8 * 0x0900),
    ("ROM", ctypes.c_uint8 * 1024),
    ("program_memory", ctypes.c_uint8 * 32 * 1024),
    ("boot section", ctypes.POINTER(ctypes.c_uint8)),
    ("R", ctypes.POINTER(ctypes.c_uint8)),
    ("IO", ctypes.POINTER(ctypes.c_uint8)),
    ("ext_IO", ctypes.POINTER(ctypes.c_uint8)),
    ("RAM", ctypes.POINTER(ctypes.c_uint8)),
    ("sp", ctypes.c_uint16),
    ("pc", ctypes.c_uint16),
    ("skip_next", ctypes.c_bool),
    ("sleeping", ctypes.c_bool),
    ("stopped", ctypes.c_bool),
    ("handle_interrupt", ctypes.c_bool),
    ("auto_execute", ctypes.c_bool),
    ("interrupt_address", ctypes.c_uint16),
    ("cycles", ctypes.c_uint16),
    ("opcode", ctypes.c_uint32),
    ("instruction", ctypes.POINTER(Instruction_t)),
    ("exeption_handler", ctypes.POINTER(ctypes.c_int)) # ??
  ]