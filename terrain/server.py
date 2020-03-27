#!/usr/bin/env python

import asyncio
import websockets
import flatbuffers

import SsProtocol.Message as SsMessage
import SsProtocol.MessageUnion as SsMessageUnion
import SsProtocol.InitRequest as SsInitRequest
import SsProtocol.InitResponse as SsInitResponse
import SsProtocol.Capabilities as SsCapabilities
import SsProtocol.Chunk as SsChunk

# Kinda hacky but it works
def CellPositionsAsNumpy(chunk):
    cellPositionsIndex = 8
    o = flatbuffers.number_types.UOffsetTFlags.py_type(chunk._tab.Offset(cellPositionsIndex))
    if o != 0:
        offset = chunk._tab.Vector(o)
        length = chunk._tab.VectorLen(o) * 3
        numpy_dtype = flatbuffers.number_types.to_numpy_type(flatbuffers.number_types.Float32Flags)
        arr = flatbuffers.encode.GetVectorAsNumpy(numpy_dtype, chunk._tab.Bytes, length, offset)
        return arr.reshape((-1, 3))
    return 0

async def connection_loop(conn, path):
    print('Received connection from {}'.format(conn.remote_address))
    builder = flatbuffers.Builder(1024)
    try:
        while True:
            buf = await conn.recv()
            msg = SsMessage.Message.GetRootAsMessage(buf, 0)
            msg_type = msg.MessageType()
            print('Received message of type {}'.format(msg_type))
            if msg_type == SsMessageUnion.MessageUnion.InitRequest:
                SsInitResponse.InitResponseStart(builder)
                SsInitResponse.InitResponseAddCapabilities(builder, SsCapabilities.Capabilities.GenerateChunk)
                init_response = SsInitResponse.InitResponseEnd(builder)

                SsMessage.MessageStart(builder)
                SsMessage.MessageAddMessageType(builder, SsMessageUnion.MessageUnion.InitResponse)
                SsMessage.MessageAddMessage(builder, init_response)
                message = SsMessage.MessageEnd(builder)

                builder.Finish(message)
                await conn.send(builder.Output())
            elif msg_type == SsMessageUnion.MessageUnion.Chunk:
                chunk = SsChunk.Chunk()
                chunk.Init(msg.Message().Bytes, msg.Message().Pos)
                cell_positions = CellPositionsAsNumpy(chunk)
                print(chunk.CellSizeLog2(), chunk.CellCoord(), chunk.CellPositionsLength())
            else:
                raise Exception('Unexpected message type: {}'.format(msg_type))
    except websockets.exceptions.ConnectionClosedOK:
        print('Clean disconnect from {}'.format(conn.remote_address))
        pass

start_server = websockets.serve(connection_loop, 'localhost', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
