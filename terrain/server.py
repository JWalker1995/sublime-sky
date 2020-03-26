#!/usr/bin/env python

import asyncio
import websockets
import flatbuffers

import generated.SublimeSky.Message as SsMessage

async def hello(websocket, path):
    msg = await websocket.recv()
    print(msg)

	# buf = open('monster.dat', 'rb').read()
	# buf = bytearray(buf)
	# message = SsMessage.GetRootAsMessage(buf, 0)

    await websocket.send('hello')

start_server = websockets.serve(hello, 'localhost', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
