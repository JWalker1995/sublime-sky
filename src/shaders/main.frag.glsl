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


const bool blinn = true;
const vec3 lightPos = vec3(0.0, 0.0, 10.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 40.0;
const vec3 ambientColor = vec3(0.0, 0.0, 0.0);
const vec3 diffuseColor = vec3(188/256.0,143/256.0,143/256.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 32.0;
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space


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

        vec3 lightDir = lightPos - orig_position;
        float distance = length(lightDir);
        distance = distance * distance;
        lightDir = normalize(lightDir);

        float lambertian = max(dot(lightDir, faceNormal), 0.0);
        float specular = 0.0;

        if (lambertian > 0.0) {

          vec3 viewDir = normalize(-orig_position);

          if (blinn) {
              vec3 halfDir = normalize(lightDir + viewDir);
              float specAngle = max(dot(halfDir, faceNormal), 0.0);
              specular = pow(specAngle, shininess);
          } else {
            vec3 reflectDir = reflect(-lightDir, faceNormal);
            float specAngle = max(dot(reflectDir, viewDir), 0.0);
            // note that the exponent is different here
            specular = pow(specAngle, shininess/4.0);
          }
        }
        vec3 colorLinear = ambientColor +
                           diffuseColor * lambertian * lightColor * lightPower / distance +
                           specColor * specular * lightColor * lightPower / distance;
        // apply gamma correction (assume ambientColor, diffuseColor and specColor
        // have been linearized, i.e. have no gamma correction in them)
        vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));
        // use the gamma corrected color in the fragment
        frag_color = vec4(colorGammaCorrected, 1.0);
    }
}

#endif
