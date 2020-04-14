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

layout(location = POSITION_LOCATION) in vec4 position;
layout(location = NORMAL_LOCATION) in vec4 normal;
layout(location = MESH_INDEX_LOCATION) in uint meshIndex;
layout(location = MATERIAL_INDEX_LOCATION) in uint materialIndex;

//in int gl_VertexID;

flat out vec3 modelPosition;
flat out vec3 modelNormal;
flat out vec4 colorAmbient;
flat out vec4 colorDiffuse;
flat out vec4 colorSpecular;
flat out float shininess;
flat out uint renderModel;


void main(void) {
    const float far_clip = 1e6f;
    const float clip_scale = 1.0f;

    modelPosition = position.xyz;
    modelNormal = normal.xyz;

    gl_Position = meshes[meshIndex].transform * vec4(modelPosition, 1.0);
    gl_PointSize = 1.0;
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

    /*
    float red = rand(gl_VertexID + 0.0);
    float green = rand(gl_VertexID + 0.33);
    float blue = rand(gl_VertexID + 0.67);
    vert_color = vec4(red, green, blue, 0.0);
    */

    //face_offset = primitive_offset;
}
