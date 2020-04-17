#version 410 core

#defines

uniform sampler2D color1Map;
uniform sampler2D color2Map;

uniform vec2 pixelSize;             // Inverse of the size of the screen
uniform float stepWidth;
in vec2 v_st;
layout(location = 0) out vec4 color1;  // Paint ids
layout(location = 1) out vec4 color2;  // Paint color
const vec4 nullId = vec4 (0.0);

vec2 pack2(float value) {
    int ivalue = int(value * 256.0 * 256.0);
    int ix = ivalue % 256;
    int iy = ivalue / 256;
    return vec2(float(ix) / 255.0, float(iy) / 255.0);
}

float unpack2(vec2 v) {
    int ix = int(round(v.x * 255.0));
    int iy = int(round(v.y * 255.0));
    int ivalue = ix + iy * 256;
    return float(ivalue) / 256.0 / 256.0;
}

void main() {
#if SOURCE_FORMAT_CENTERS

#endif

    // position vec3;
    //

    vec4 sampleId = texture (color1Map, v_st);
    if (sampleId.w == 1.0) {  // A seed pixel
        color1 = sampleId;
        color2 = texture(color2Map, v_st);
        return;
    }

    // Background pixel
    vec4 bestId = nullId;
    vec2 bestCoord = vec2 (0.0);
    float bestDist = 99999.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 sampleCoord = v_st + vec2(x,y) * (pixelSize * stepWidth);
            vec4 sampleId = texture(color1Map, sampleCoord);
            if (sampleId == nullId) continue; // Empty background neighbor pixel
            if (sampleId.w == 0.0) { // Neighbor pixel
                vec4 tmp = texture(color2Map, sampleCoord);
                // Coordinate of actual seed stored in color2Map
                sampleCoord = vec2(unpack2(tmp.xy), unpack2(tmp.zw));
            }

            float dist = distance(sampleCoord, v_st);
            if (dist < bestDist) {
                bestDist = dist;
                bestCoord = sampleCoord;
                bestId = vec4(texture(color1Map, sampleCoord).xy, 0.0, 0.0);
            }
        }
    }

    color1 = bestId;
    color2 = vec4(pack2(bestCoord.x), pack2(bestCoord.y));
}
