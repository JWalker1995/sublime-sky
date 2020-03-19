#pragma once

#include "defs/GLBUFFER_ASSERT_BINDINGS.h"

#include "graphics/gl.h"

namespace graphics {

class GlBufferBase {
public:
    GlBufferBase(GLenum target, GLenum buffer_hint)
        : target(target)
        , buffer_hint(buffer_hint)
    {
        glGenBuffers(1, &vbo_id);
        graphics::GL::catchErrors();
    }

    ~GlBufferBase()
    {
        glDeleteBuffers(1, &vbo_id);
        graphics::GL::catchErrors();
    }

    void bind() const
    {
        glBindBuffer(target, vbo_id);
        graphics::GL::catchErrors();
    }

    void assert_bound() const
    {
#if GLBUFFER_ASSERT_BINDINGS
        GLint binding;

        switch (target)
        {
        case GL_ARRAY_BUFFER:
            glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &binding);
            break;

        case GL_ATOMIC_COUNTER_BUFFER:
            glGetIntegerv(GL_ATOMIC_COUNTER_BUFFER_BINDING, &binding);
            break;

        case GL_DISPATCH_INDIRECT_BUFFER:
            glGetIntegerv(GL_DISPATCH_INDIRECT_BUFFER_BINDING, &binding);
            break;

        case GL_DRAW_FRAMEBUFFER:
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &binding);
            break;

        case GL_DRAW_INDIRECT_BUFFER:
            glGetIntegerv(GL_DRAW_INDIRECT_BUFFER_BINDING, &binding);
            break;

        case GL_ELEMENT_ARRAY_BUFFER:
            glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &binding);
            break;

        case GL_FRAMEBUFFER:
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &binding);
            break;

        case GL_PIXEL_PACK_BUFFER:
            glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &binding);
            break;

        case GL_PIXEL_UNPACK_BUFFER:
            glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &binding);
            break;

        case GL_PROGRAM_PIPELINE:
            glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, &binding);
            break;

        case GL_QUERY_BUFFER:
            glGetIntegerv(GL_QUERY_BUFFER_BINDING, &binding);
            break;

        case GL_READ_FRAMEBUFFER:
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &binding);
            break;

        case GL_RENDERBUFFER:
            glGetIntegerv(GL_RENDERBUFFER_BINDING, &binding);
            break;

        case GL_SAMPLER:
            glGetIntegerv(GL_SAMPLER_BINDING, &binding);
            break;

        case GL_SHADER_STORAGE_BUFFER:
            glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &binding);
            break;

        case GL_TRANSFORM_FEEDBACK_BUFFER:
            glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &binding);
            break;

        case GL_UNIFORM_BUFFER:
            glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &binding);
            break;

        case GL_VERTEX_ARRAY:
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &binding);
            break;

        default:
            assert(false);
        }

        assert(static_cast<GLuint>(binding) == vbo_id);
#endif
    }

    GLuint get_base_binding() {
        static GLuint nextIndex = 0;
        if (base_binding == static_cast<GLuint>(-1)) {
            base_binding = nextIndex++;
        }
        return base_binding;
    }
    void bind_base()
    {
        glBindBufferBase(target, get_base_binding(), vbo_id);
        graphics::GL::catchErrors();
    }

    void unbind() const
    {
        glBindBuffer(target, 0);
        graphics::GL::catchErrors();
    }

protected:
    const GLenum target;
    const GLenum buffer_hint;
    GLuint vbo_id;
    GLuint base_binding = static_cast<GLuint>(-1);
};

}
