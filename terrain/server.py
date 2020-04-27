#!/usr/bin/env python

import asyncio
import websockets
import flatbuffers
import numpy as np

import SsProtocol.Message as SsMessage
import SsProtocol.MessageUnion as SsMessageUnion
import SsProtocol.InitRequest as SsInitRequest
import SsProtocol.InitResponse as SsInitResponse
import SsProtocol.Capabilities as SsCapabilities
import SsProtocol.Material as SsMaterial
import SsProtocol.MaterialPhase as SsMaterialPhase
import SsProtocol.MaterialRenderModel as SsMaterialRenderModel
import SsProtocol.TerrainChunk as SsTerrainChunk
import SsProtocol.Vec3_u32 as SsVec3_u32
import SsProtocol.Vec4_f as SsVec4_f

from generate import generate_materials, generate_terrain

def serialize_material(builder, material):
    name = builder.CreateString(material['name'])

    SsMaterial.MaterialStart(builder)
    SsMaterial.MaterialAddName(builder, name)
    SsMaterial.MaterialAddPhase(builder, {
        'solid': SsMaterialPhase.MaterialPhase.Solid,
        'liquid': SsMaterialPhase.MaterialPhase.Liquid,
        'gas': SsMaterialPhase.MaterialPhase.Gas,
    }[material['phase'].lower()])
    SsMaterial.MaterialAddMass(builder, material['mass'])
    SsMaterial.MaterialAddRenderModel(builder, {
        'invisible': SsMaterialRenderModel.MaterialRenderModel.Invisible,
        'phong': SsMaterialRenderModel.MaterialRenderModel.Phong,
        'blinn': SsMaterialRenderModel.MaterialRenderModel.Blinn,
    }[material['render_model'].lower()])
    SsMaterial.MaterialAddColorAmbient(builder, SsVec4_f.CreateVec4_f(builder, *material['color_ambient']))
    SsMaterial.MaterialAddColorDiffuse(builder, SsVec4_f.CreateVec4_f(builder, *material['color_diffuse']))
    SsMaterial.MaterialAddColorSpecular(builder, SsVec4_f.CreateVec4_f(builder, *material['color_specular']))
    SsMaterial.MaterialAddShininess(builder, material['shininess'])
    return SsMaterial.MaterialEnd(builder)

async def connection_loop(conn, path):
    print('Received connection from {}'.format(conn.remote_address))
    try:
        while True:
            buf = await conn.recv()
            msg = SsMessage.Message.GetRootAsMessage(buf, 0)
            msg_type = msg.MessageType()
            if msg_type == SsMessageUnion.MessageUnion.InitRequest:
                builder = flatbuffers.Builder(1024)

                req = SsInitRequest.InitRequest()
                req.Init(msg.Message().Bytes, msg.Message().Pos)
                seed = req.Seed()

                materials = generate_materials(seed)
                material_offsets = [serialize_material(builder, m) for m in materials]

                SsInitResponse.InitResponseStartMaterialsVector(builder, len(material_offsets))
                [builder.PrependUOffsetTRelative(mo) for mo in reversed(material_offsets)]
                materials_vec = builder.EndVector(len(material_offsets))

                SsInitResponse.InitResponseStart(builder)
                SsInitResponse.InitResponseAddCapabilities(builder, SsCapabilities.Capabilities.GenerateTerrainChunk)
                SsInitResponse.InitResponseAddMaterials(builder, materials_vec)
                init_response = SsInitResponse.InitResponseEnd(builder)

                SsMessage.MessageStart(builder)
                SsMessage.MessageAddMessageType(builder, SsMessageUnion.MessageUnion.InitResponse)
                SsMessage.MessageAddMessage(builder, init_response)
                message = SsMessage.MessageEnd(builder)

                builder.Finish(message)
                await conn.send(builder.Output())
            elif msg_type == SsMessageUnion.MessageUnion.TerrainMessage:
                builder = flatbuffers.Builder(1024)

                message = SsTerrainMessage.TerrainMessage()
                message.Init(msg.Message().Bytes, msg.Message().Pos)

                

                cell_types = generate_terrain(seed, cell_positions)
                assert len(cell_types.shape) == 1
                assert cell_types.dtype == np.uint32

                SsTerrainChunk.TerrainChunkStartCellMaterialsVector(builder, cell_types.shape[0])
                builder.Prep(4, cell_types.shape[0] * 4 - 4)
                new_head = builder.head - cell_types.shape[0] * 4
                builder.Bytes[new_head : builder.head] = cell_types.tobytes()
                builder.head = new_head
                cell_types_vec = builder.EndVector(cell_types.shape[0])

                SsTerrainChunk.TerrainChunkStart(builder)
                SsTerrainChunk.TerrainChunkAddCellSizeLog2(builder, chunk.CellSizeLog2())
                coord = chunk.CellCoord()
                SsTerrainChunk.TerrainChunkAddCellCoord(builder, SsVec3_u32.CreateVec3_u32(builder, coord.X(), coord.Y(), coord.Z()))
                SsTerrainChunk.TerrainChunkAddCellMaterials(builder, cell_types_vec)
                chunk = SsTerrainChunk.TerrainChunkEnd(builder)

                SsMessage.MessageStart(builder)
                SsMessage.MessageAddMessageType(builder, SsMessageUnion.MessageUnion.TerrainChunk)
                SsMessage.MessageAddMessage(builder, chunk)
                message = SsMessage.MessageEnd(builder)

                builder.Finish(message)
                await conn.send(builder.Output())
            else:
                raise Exception('Unexpected message type: {}'.format(msg_type))
    except websockets.exceptions.ConnectionClosedOK:
        print('Clean disconnect from {}'.format(conn.remote_address))
        pass

start_server = websockets.serve(connection_loop, 'localhost', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
