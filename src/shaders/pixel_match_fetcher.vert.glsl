#version 410 core

#defines
#define M_PI 3.1415926535897932384626433832795

layout(location = VALUES_LOCATION) in uvec4 values;

flat out float vertexId;

highp float rand(float seed)
{
    highp float c = 43758.5453;
    highp float sn = mod(seed, M_PI);
    return fract(sin(sn) * c);
}

void main(void) {
    vertexId = float(gl_VertexID);

    // Assign a random x-coordinate
    float x = rand(vertexId) * 2.0 - 1.0;

    uvec4 t0 = max(values, values << 8);
    uvec4 t1 = max(values << 16, values << 24);
    uvec4 t2 = max(t0, t1);
    uint t3 = max(max(t2.x, t2.y), max(t2.z, t2.w));
    // Make values equal to 255 become -1.0, and all other values be clipped
    float y = (float(t3 >= 0xFF000000u)) - 2.0;

    gl_Position = vec4(x, y, 0.0, 1.0);
}
