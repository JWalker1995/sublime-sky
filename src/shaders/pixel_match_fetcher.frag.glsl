#version 410 core

#defines

flat in float vertexId;

layout(location = 0) out vec4 fragColor;

void main(void) {
    fragColor.x = vertexId;
}
