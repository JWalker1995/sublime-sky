# automatically generated by the FlatBuffers compiler, do not modify

# namespace: SsProtocol

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class TerrainMessage(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsTerrainMessage(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TerrainMessage()
        x.Init(buf, n + offset)
        return x

    # TerrainMessage
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TerrainMessage
    def Chunks(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from SsProtocol.TerrainChunk import TerrainChunk
            obj = TerrainChunk()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # TerrainMessage
    def ChunksLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # TerrainMessage
    def ChunksIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def TerrainMessageStart(builder): builder.StartObject(1)
def TerrainMessageAddChunks(builder, chunks): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(chunks), 0)
def TerrainMessageStartChunksVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def TerrainMessageEnd(builder): return builder.EndObject()
