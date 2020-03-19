#version 410 core

#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shader_storage_buffer_object : require

#defines

layout(local_size_x = LOCAL_SIZE_X, local_size_y = LOCAL_SIZE_Y, local_size_z = LOCAL_SIZE_Z) in;

/*
GlBuffer<NodeDrawable, GL_DYNAMIC_COPY, std::ratio<1, 1>> node_drawable_buffer;
GlBuffer<NodeReadable, GL_DYNAMIC_COPY, std::ratio<1, 1>> node_readable_buffer;

GlBuffer<EmitNode, GL_DYNAMIC_COPY, std::ratio<1, 1>> emit_node_buffer;
*/



uint trigger(uint data, uint if_success, uint if_fail)
{
    uint trigger_index = atomicCounterIncrement(current_trigger);
    if (trigger_index < max_triggers)
    {
        triggers[trigger_index] = data;
        return if_success;
    }
    else
    {
        return if_fail;
    }
}

void main(void)
{
    uint blob_id = (gl_WorkGroupID.x * gl_WorkGroupSize.y + gl_WorkGroupID.y) * (LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z) + gl_LocalInvocationIndex;
    uint child_0 = node_drawable[blob_id].children[0];
    uint child_1 = node_drawable[blob_id].children[1];



    uint count = face_counts[face_id];
    uint new_count;

    switch (count & PREFIX_MASK)
    {
    case NORMAL_STATE_PREFIX:
        if (count > (SUBDIV_THRESHOLD ^ NORMAL_STATE_PREFIX))
        {
            // Tell the CPU to subdivide this face
            // It should be subdivided by next frame
            new_count = trigger(SUBDIV_TRIGGER_PREFIX ^ face_id, NORMAL_STATE_PREFIX, NORMAL_STATE_PREFIX);
        }
        else if (count < (DESTROY_THRESHOLD ^ NORMAL_STATE_PREFIX))
        {
            // Tell the CPU that this face is availible for recycling
            // Won't necessarily respond within a frame, so we'll mark it as destroying
            new_count = trigger(DESTROY_TRIGGER_PREFIX ^ face_id, DESTROY_STATE_PREFIX, NORMAL_STATE_PREFIX);
        }
        else
        {
            new_count = NORMAL_STATE_PREFIX;
        }
        break;

    case DESTROY_STATE_PREFIX:
        if (count > ((DESTROY_THRESHOLD + DESTROY_THRASH_PADDING) ^ DESTROY_STATE_PREFIX))
        {
            new_count = trigger(NORMAL_TRIGGER_PREFIX ^ face_id, NORMAL_STATE_PREFIX, DESTROY_STATE_PREFIX);
        }
        else
        {
            new_count = DESTROY_STATE_PREFIX;
        }
        break;

    case GHOST_STATE_PREFIX:
        if (count > (GHOST_THRESHOLD ^ GHOST_STATE_PREFIX))
        {
            new_count = trigger(GHOST_TRIGGER_PREFIX ^ face_id, IGNORE_STATE_PREFIX, GHOST_STATE_PREFIX);
        }
        else
        {
            new_count = GHOST_STATE_PREFIX;
        }
        break;

    case IGNORE_STATE_PREFIX:
        new_count = IGNORE_STATE_PREFIX;
        break;

    default:
        // Should never get here
        new_count = 0;
    }

    face_counts[face_id] = new_count;
}
