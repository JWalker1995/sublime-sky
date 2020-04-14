#pragma once

#include "game/gamecontext.h"

#include "render/program/program.h"
#include "render/scenemanager.h"

namespace render {

class PixelMatchFetcherProgram : public Program {
public:
    PixelMatchFetcherProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

    virtual void draw();

private:
    class Vao : public graphics::GlVao {
    public:
        Vao(game::GameContext &context) {
            (void) context;

            bind();

            glGenBuffers(1, &transferBufferId);
            graphics::GL::catchErrors();

            glBindBuffer(GL_ARRAY_BUFFER, transferBufferId);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, transferBufferId);
            graphics::GL::catchErrors();

            GLuint valuesLocation = prepareProgramAttribute("VALUES_LOCATION", 1);
            glVertexAttribIPointer(valuesLocation, 4, GL_UNSIGNED_INT, sizeof(GLuint) * 4, 0);
            glEnableVertexAttribArray(valuesLocation);
            graphics::GL::catchErrors();

            unbind();
        }

        void ensureTransferBufferSize(unsigned int sizeBytes) {
            assertBound();

            if (sizeBytes > transferBufferSize) {
                glBufferData(GL_PIXEL_PACK_BUFFER, sizeBytes, 0, GL_DYNAMIC_COPY);
                graphics::GL::catchErrors();
                transferBufferSize = sizeBytes;
            }
        }

        ~Vao() {
            glDeleteBuffers(1, &transferBufferId);
        }

    private:
        GLuint transferBufferId;
        unsigned int transferBufferSize = 0;
    };
};

}
