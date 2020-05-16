import asyncio
import websockets
import json
import http.server
import socketserver
import threading
import ctypes
import sys
import os
import wurlitzer
import mcu_types

ws_port = 3000
http_port = 3080
host = 'localhost'
handlers = {}
ws = None
mcu_t = mcu_types.ATmega328p_t
mcu_ptr = ctypes.POINTER(mcu_t)
mcu = mcu_t()
mcu_running = True

async def log(message):
  await ws.send(json.dumps({'event': 'log', 'data': message}))

async def web_console(message):
  await ws.send(json.dumps({'event': 'console', 'data': message}))

def start_http(arg):
  Handler = http.server.SimpleHTTPRequestHandler
  with socketserver.TCPServer(("", http_port), Handler) as httpd:
    httpd.serve_forever()

async def execute_c(function, check_state=False):
  global mcu_running
  with wurlitzer.pipes() as (out, err):
    if check_state:
      state = function()
      mcu_running = state == 1
      if not mcu_running:
        await emit('execute stop', None)
        await log('MCU has been stopped\n')
    else:
      function()
  data = out.read()
  if data:
    await web_console(data)

def on(event, callback):
  handlers[event] = callback

async def emit(event, data):
  await ws.send(json.dumps({'event': event, 'data': data}))

async def send_pong(data):
  await emit('pong', 'pong')

async def do_test(data):
  global mcu
  print('Test data =', data)
  string = 'This is a test'.encode('utf-8')
  with wurlitzer.pipes() as (out, err):
    mcu_fn.mcu_load_asm(string)
  data = out.read()
  if data:
    await web_console(data)

async def compile_asm(code):
  string = code.encode('utf-8')
  with wurlitzer.pipes() as (out, err):
    result = mcu_fn.mcu_load_asm(string)
  data = out.read()
  if data:
    await web_console(data)

async def compile_c(code):
  string = code.encode('utf-8')
  with wurlitzer.pipes() as (out, err):
    result = mcu_fn.mcu_load_c(string)
  data = out.read()
  if data:
    await web_console(data)

async def resume_mcu():
  global mcu_running
  mcu_fn.mcu_resume()
  mcu_running = True
  await emit('mcu resumed', None)
  await log('MCU has been resumed\n')

async def execute_cycle():
  global mcu_running
  if mcu_running:
   await execute_c(mcu_fn.mcu_execute_cycle, True)
   mcu_fn.mcu_get_copy(mcu)
   await emit('mcu state', mcu_types.to_string(mcu))

on('ping', send_pong)
on('test', do_test)
on('compile asm', compile_asm)
on('compile c', compile_c)
on('execute cycle', execute_cycle)
on('mcu resume', resume_mcu)

async def ws_server(websocket, path):
  global ws
  global mcu_running
  mcu_running = True
  ws = websocket
  await emit('ready', 'Hello')
  await log('Connected with socket server\n')
  await execute_c(mcu_fn.mcu_init)
  while True:
    try:
      message = json.loads(await ws.recv())
    except:
      break
    if message['event'] in handlers:
      if 'data' in message:
        await handlers[message['event']](message['data'])
      else:
        await handlers[message['event']]()

mcu_fn = ctypes.CDLL('../ATmega328p/mcu_shared.so')
# void mcu_get_copy(ATmega328p_t *mcu);
mcu_fn.mcu_get_copy.argtypes = [mcu_ptr]
mcu_fn.mcu_get_copy.restypes = []
# bool mcu_load_asm(const char *code);
mcu_fn.mcu_load_asm.argtypes = [ctypes.c_char_p]
mcu_fn.mcu_load_asm.restypes = [ctypes.c_bool]
# bool mcu_load_c(const char *code)
mcu_fn.mcu_load_c.argtypes = [ctypes.c_char_p]
mcu_fn.mcu_load_c.restypes = [ctypes.c_bool]
# bool mcu_execute_cycle(void);
mcu_fn.mcu_execute_cycle.argtypes = []
mcu_fn.mcu_execute_cycle.restypes = [ctypes.c_bool]

threading._start_new_thread(start_http, (None, ))

start_server = websockets.serve(ws_server, host, ws_port)

print(f"HTTP server:{http_port}, socket server:{ws_port}")

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()