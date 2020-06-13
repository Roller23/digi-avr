import ctypes
import json

def getdict(struct):
  return dict((field, getattr(struct, field)) for field, _ in struct._fields_)

def bytes_to_int(bytes_array):
  output = []
  for byte in bytearray(bytes_array):
    output.append(byte)
  return output

def to_string(struct):
  dict_struct = getdict(struct)
  dict_struct['data_memory'] = bytes_to_int(dict_struct['data_memory'])
  dict_struct['ROM'] = bytes_to_int(dict_struct['ROM'])
  dict_struct['boot_section'] = (32 * 1024) - 512
  dict_struct['R'] = 0
  dict_struct['IO'] = 32 + dict_struct['R']
  dict_struct['ext_IO'] = 64 + dict_struct['IO']
  dict_struct['RAM'] = 160 + dict_struct['ext_IO']
  del dict_struct['program_memory']
  del dict_struct['exeption_handler']
  del dict_struct['instruction']
  return json.dumps(dict_struct)

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
    ("data_memory", ctypes.c_uint8 * 0x900),
    ("ROM", ctypes.c_uint8 * 1024),
    ("program_memory", ctypes.c_uint8 * 32 * 1024),
    ("boot_section", ctypes.POINTER(ctypes.c_uint8)),
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
    ("data_memory_change", ctypes.c_int16),
    ("cycles", ctypes.c_uint16),
    ("opcode", ctypes.c_uint32),
    ("instruction", ctypes.POINTER(Instruction_t)),
    ("exeption_handler", ctypes.POINTER(ctypes.c_int))
  ]