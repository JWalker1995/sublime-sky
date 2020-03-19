#version 410 core

#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shader_storage_buffer_object : require

#defines

layout(std140, binding = ALPHA_BOUNDARIES_BINDING) buffer AlphaBoundaries
{
    // Extends beyond the capacity by [number of pixels] elements
    // The extension should be zeroed at the beginning
    uint alpha_boundary_ptrs[];

    uint alpha_boundary_blob_ids[];

    float alpha_boundary_z[];
};

layout(binding = ALPHA_BOUNDARIES_SIZE_BINDING) uniform atomic_uint alpha_boundaries_size;
layout(location = ALPHA_BOUNDARIES_CAPACITY_LOCATION) uniform uint alpha_boundaries_capacity;


in vec4 gl_FragCoord;
flat in vec2 blob_coord;
flat in float blob_rad;
flat in uint blob_id;

void main(void) {
    if (!fragment collides with blob) {
        return;
    }

    uint new_id = atomicCounterIncrement(alpha_boundaries_size);
    if (new_id >= alpha_boundaries_capacity) {
        return;
    }

    alpha_boundary_ptrs[new_id] = 0;
    alpha_boundary_[new_id] = 0;
    alpha_boundary_ptrs[new_id] = 0;

    uint pixel = ...;
    uint ptr = alpha_boundaries_capacity + pixel;

    if (ptr == 0) {
        ptr = new_id;
    }

    do {
        if (alpha_boundary_ptrs[pixel_id])
    } while (pixel_id != 0);

    alpha_boundaries[];

    atomicExchange()
}
