#version 410 core

#defines

// MESH_TRANSFORMS_BINDING
layout (std140) uniform meshes
{
    mat4 transforms[1024];
};

// in uint primitive_offset;
layout(location = POSITION_LOCATION) in vec4 position;
layout(location = NORMAL_LOCATION) in vec4 normal;
layout(location = MESH_INDEX_LOCATION) in uint meshIndex;
layout(location = RENDER_FLAGS_LOCATION) in uint renderFlags;
layout(location = COLOR_LOCATION) in vec4 in_color;

//in int gl_VertexID;

//flat out uint face_offset;
//out vec4 position;
out vec3 orig_position;
out vec3 vert_color;

highp float rand(float seed)
{
    highp float c = 43758.5453;
    highp float sn = mod(seed, 3.14);
    return fract(sin(sn) * c);
}

void main(void)
{
    const float far_clip = 1e6f;
    const float clip_scale = 1.0f;

    orig_position = position.xyz;

    gl_Position = transforms[meshIndex] * vec4(position.xyz, 1.0);
    //gl_Position = vec4(rand(gl_VertexID) / 16.0 + float(meshes.transforms[1][1][0] == 0.0) - 0.03125, rand(gl_VertexID + 100), 1.0, 1.0);
    //gl_Position = vec4(gl_VertexID / 200.0 - 0.8, float(position.w == 1.01) + cos(gl_VertexID) * 0.1, 1.0, 1.0);

    if (USE_LOG_DEPTH_BUFFER == 1)
    {
        gl_Position.z = gl_Position.w * log2(clip_scale * gl_Position.z + 1.0f) / log2(clip_scale * far_clip + 1.0f);
    }

    if ((renderFlags & (1u << RENDER_FLAGS_SELECTED_BIT)) != 0) {
        vert_color = vec3(1.0, 0.0, 0.0);
    } else {
        vert_color = in_color.xyz;
    }

    /*
    float red = rand(gl_VertexID + 0.0);
    float green = rand(gl_VertexID + 0.33);
    float blue = rand(gl_VertexID + 0.67);
    vert_color = vec4(red, green, blue, 0.0);
    */

    //face_offset = primitive_offset;
}
