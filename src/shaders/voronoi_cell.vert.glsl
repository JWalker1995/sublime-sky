#version 410 core

#defines

#define NUM_MESHES 64
#define NUM_MATERIALS 64

struct Mesh {
    mat4 transform;
};

struct Material {
    vec4 colorAmbient;
    vec4 colorDiffuse;
    vec4 colorSpecular;
    float shininess;
    uint renderModel;
};

layout (std140) uniform MeshesBlock
{
    Mesh meshes[NUM_MESHES];
};

layout (std140) uniform MaterialsBlock
{
    Material materials[NUM_MATERIALS];
};

layout(location = CELL_POSITION_LOCATION) in uvec4 cellPosition;
#repeat 0 3 layout(location = NEIGHBOR_CELL_LOCATION_%) in uvec4 neighborCell_attr_%;
layout(location = MESH_INDEX_LOCATION) in uint meshIndex;
layout(location = MATERIAL_INDEX_LOCATION) in uint materialIndex;

//in int gl_VertexID;

flat out vec4 neighborCellNormals[24];
//flat out uint face_offset;
//out vec4 position;
out vec3 modelPosition;
//flat out vec3 modelNormal;
flat out vec4 colorAmbient;
flat out vec4 colorDiffuse;
flat out vec4 colorSpecular;
flat out float shininess;
flat out uint renderModel;

float uintToFloat(uint coord) {
    return float(int(coord - UINT_COORD_OFFSET));
}

#define SET_NEIGHBOR_CELL_NORMALS(baseVec, inVar, i) \
    neighborCellNormals[i * 8 + 0] = decode(baseVec, inVar.x % 0x0000FFFFu); \
    neighborCellNormals[i * 8 + 1] = decode(baseVec, inVar.x >> 16); \
    neighborCellNormals[i * 8 + 2] = decode(baseVec, inVar.y % 0x0000FFFFu); \
    neighborCellNormals[i * 8 + 3] = decode(baseVec, inVar.y >> 16); \
    neighborCellNormals[i * 8 + 4] = decode(baseVec, inVar.z % 0x0000FFFFu); \
    neighborCellNormals[i * 8 + 5] = decode(baseVec, inVar.z >> 16); \
    neighborCellNormals[i * 8 + 6] = decode(baseVec, inVar.w % 0x0000FFFFu); \
    neighborCellNormals[i * 8 + 7] = decode(baseVec, inVar.w >> 16);

vec4 decode(in vec4 base, in uint code) {
    uint t = code;
    float z = float(t & 31u) * (5.0 / 32.0);
    t = t >> 5;
    float y = float(t & 31u) * (5.0 / 32.0);
    t = t >> 5;
    float x = float(t & 31u) * (5.0 / 32.0);
    t = t >> 5;

    return base + vec4(x, y, z, float(t));
}

/*
void processNeighborUshort(inout float frontDist, inout float backDist, inout vec3 normal, in vec3 base, in vec3 center, in uint neighborCode) {
    vec3 neighbor;
    bool isSolid;
    decode(neighbor, isSolid, base, neighborCode);

    vec3 viewDir = eyePos - modelPosition;
    vec3 n = neighbor - center;
    vec3 p_0 = (neighbor + center) / 2.0;
    float den = dot(viewDir, n);
    float d = dot(p_0 - eyePos, n) / den;

    if (den < 0.0) {
        frontDist = max(frontDist, d);
//        if (d > frontDist) {
//            frontDist = d;
//            normal = n;
//        }
    } else {
        backDist = min(backDist, d);
    }

//    if (frontDist > backDist) {
//        discard;
//    }
}

void processNeighborUint(inout float frontDist, inout float backDist, inout vec3 normal, in vec3 base, in vec3 center, in uint neighbors) {
    processNeighborUshort(frontDist, backDist, normal, base, center, neighbors & 0x0000FFFFu);
    processNeighborUshort(frontDist, backDist, normal, base, center, neighbors >> 16);
}

void processNeighborVec(inout float frontDist, inout float backDist, inout vec3 normal, in vec3 base, in vec3 center, in uvec4 neighbors) {
    processNeighborUint(frontDist, backDist, normal, base, center, neighbors.x);
    processNeighborUint(frontDist, backDist, normal, base, center, neighbors.y);
    processNeighborUint(frontDist, backDist, normal, base, center, neighbors.z);
    processNeighborUint(frontDist, backDist, normal, base, center, neighbors.w);
}
*/

highp float rand(float seed)
{
    highp float c = 43758.5453;
    highp float sn = mod(seed, 3.14);
    return fract(sin(sn) * c);
}

void main(void) {
    const float far_clip = 1e6f;
    const float clip_scale = 1.0f;

    modelPosition = vec3(uintToFloat(cellPosition.x), uintToFloat(cellPosition.y), uintToFloat(cellPosition.z));

    gl_Position = meshes[meshIndex].transform * vec4(modelPosition, 1.0);
    gl_PointSize = 1000.0 / gl_Position.z;
    //gl_Position = vec4(rand(gl_VertexID) / 16.0 + float(meshes.transforms[1][1][0] == 0.0) - 0.03125, rand(gl_VertexID + 100), 1.0, 1.0);
    //gl_Position = vec4(gl_VertexID / 200.0 - 0.8, float(position.w == 1.01) + cos(gl_VertexID) * 0.1, 1.0, 1.0);

    if (USE_LOG_DEPTH_BUFFER == 1)
    {
        gl_Position.z = gl_Position.w * log2(clip_scale * gl_Position.z + 1.0f) / log2(clip_scale * far_clip + 1.0f);
    }

    colorAmbient = materials[materialIndex].colorAmbient;
    colorDiffuse = materials[materialIndex].colorDiffuse;
    colorSpecular = materials[materialIndex].colorSpecular;
    shininess = materials[materialIndex].shininess;
    renderModel = materials[materialIndex].renderModel;

    vec4 base = vec4(modelPosition - vec3(2.0), 0.0);
    #repeat 0 3 SET_NEIGHBOR_CELL_NORMALS(base, neighborCell_attr_%, %);

    /*
    float red = rand(gl_VertexID + 0.0);
    float green = rand(gl_VertexID + 0.33);
    float blue = rand(gl_VertexID + 0.67);
    vert_color = vec4(red, green, blue, 0.0);
    */

    //face_offset = primitive_offset;
}
