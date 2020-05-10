import asyncio
import websockets
import json

port = 3000
host = 'localhost'
handlers = {}
ws = None

def on(event, callback):
  handlers[event] = callback

async def emit(event, data):
  await ws.send(json.dumps({'event': event, 'data': data}))

async def send_pong(data):
  await emit('pong', 'pong')

on('ping', send_pong)

async def server(websocket, path):
  global ws
  ws = websocket
  ready_event = {'event': 'ready', 'data': 'Hello'}
  await ws.send(json.dumps(ready_event))
  while True:
    try:
      message = await ws.recv()
    except:
      break
    print(f"Got a message\n{message}")
    msg = json.loads(message)
    if msg['event'] in handlers:
      await handlers[msg['event']](msg['data'])

start_server = websockets.serve(server, host, 3000)
print(f"Socket server started on {host}:{port}")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()