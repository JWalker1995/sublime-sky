# automatically generated by the FlatBuffers compiler, do not modify

# namespace: SsProtocol

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Material(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsMaterial(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Material()
        x.Init(buf, n + offset)
        return x

    # Material
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Material
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Material
    def Phase(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Material
    def Mass(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # Material
    def RenderModel(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 2

    # Material
    def ColorAmbient(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from SsProtocol.Vec4_f import Vec4_f
            obj = Vec4_f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # Material
    def ColorDiffuse(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = o + self._tab.Pos
            from SsProtocol.Vec4_f import Vec4_f
            obj = Vec4_f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # Material
    def ColorSpecular(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            x = o + self._tab.Pos
            from SsProtocol.Vec4_f import Vec4_f
            obj = Vec4_f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # Material
    def Shininess(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def MaterialStart(builder): builder.StartObject(8)
def MaterialAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def MaterialAddPhase(builder, phase): builder.PrependUint8Slot(1, phase, 0)
def MaterialAddMass(builder, mass): builder.PrependFloat32Slot(2, mass, 0.0)
def MaterialAddRenderModel(builder, renderModel): builder.PrependUint32Slot(3, renderModel, 2)
def MaterialAddColorAmbient(builder, colorAmbient): builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(colorAmbient), 0)
def MaterialAddColorDiffuse(builder, colorDiffuse): builder.PrependStructSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(colorDiffuse), 0)
def MaterialAddColorSpecular(builder, colorSpecular): builder.PrependStructSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(colorSpecular), 0)
def MaterialAddShininess(builder, shininess): builder.PrependFloat32Slot(7, shininess, 0.0)
def MaterialEnd(builder): return builder.EndObject()
