#version 410 core

// #extension GL_ARB_shader_storage_buffer_object : require

#defines

// This is necessary so the fragment shader will not be called for occluded faces
// layout(early_fragment_tests) in;

#if EMPTY_FRAGMENT_SHADER

void main(void) {}

#else

uniform bool showTriangles;
uniform vec3 eyePos;

// layout(binding = NUM_FRAGMENTS_BINDING) uniform atomic_uint num_fragments;


in vec3 orig_position;
in vec3 vert_color;
out vec4 frag_color;

highp float rand(float seed) {
    highp float c = 43758.5453;
    highp float sn = mod(seed, 3.14);
    return fract(sin(sn) * c);
}


void main(void) {
    if (showTriangles) {
        if (gl_FrontFacing) {
            frag_color = vec4(rand(gl_PrimitiveID), rand(gl_PrimitiveID + 0.3), rand(gl_PrimitiveID + 0.6), 0.0);
        } else {
            frag_color = vec4(1.0, 1.0, 1.0, 0.0);
        }
    } else {
        vec3 xTangent = dFdx(orig_position);
        vec3 yTangent = dFdy(orig_position);
        vec3 faceNormal = normalize(cross(xTangent, yTangent));

        vec3 lightPos = vec3(0.0, 0.0, 100.0);
        vec3 viewPos = eyePos;
        float spec = 0.0;
        bool blinn = true;

        vec3 color = vert_color;
        // ambient
        vec3 ambient = 0.05 * color;
        // diffuse
        vec3 lightDir = normalize(lightPos - orig_position);
        float diff = max(dot(lightDir, faceNormal), 0.0);
        vec3 diffuse = diff * color;
        // specular
        vec3 viewDir = normalize(viewPos - orig_position);
        vec3 reflectDir = reflect(-lightDir, faceNormal);
        if (blinn)
        {
            vec3 halfwayDir = normalize(lightDir + viewDir);
            spec = pow(max(dot(faceNormal, halfwayDir), 0.0), 32.0);
        }
        else
        {
            vec3 reflectDir = reflect(-lightDir, faceNormal);
            spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
        }
        vec3 specular = vec3(0.3) * spec; // assuming bright white light color
        frag_color = vec4(ambient + diffuse + specular, 1.0);
    }
}

#endif
