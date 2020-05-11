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

ws_port = 3000
http_port = 3080
host = 'localhost'
handlers = {}
ws = None

async def log(message):
  await ws.send(json.dumps({'event': 'log', 'data': message}))

async def web_console(message):
  await ws.send(json.dumps({'event': 'console', 'data': message}))

def start_http(arg):
  Handler = http.server.SimpleHTTPRequestHandler
  with socketserver.TCPServer(("", http_port), Handler) as httpd:
    httpd.serve_forever()

async def execute_c(function):
  with wurlitzer.pipes() as (out, err):
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

on('ping', send_pong)

async def ws_server(websocket, path):
  global ws
  ws = websocket
  await emit('ready', 'Hello')
  await execute_c(mcu_fn.mcu_init)
  while True:
    try:
      message = await ws.recv()
    except:
      break
    msg = json.loads(message)
    if msg['event'] in handlers:
      await handlers[msg['event']](msg['data'])

mcu_fn = ctypes.CDLL('../ATmega328p/mcu_shared.so')

threading._start_new_thread(start_http, (None, ))

start_server = websockets.serve(ws_server, host, ws_port)

print(f"HTTP server:{http_port}, socket server:{ws_port}")

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()