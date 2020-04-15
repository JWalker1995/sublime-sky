#pragma once

#include "game/gamecontext.h"

#include "render/program/program.h"
#include "render/scenemanager.h"

namespace render {

class PixelMatchFetcherProgram : public Program {
public:
    PixelMatchFetcherProgram(game::GameContext &context);
    virtual ~PixelMatchFetcherProgram();

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

    virtual void draw();

private:
    class TransferVao : public graphics::GlVao {
    public:
        TransferVao(game::GameContext &context) {
            (void) context;

            bind();

            glGenBuffers(1, &vboId);
            graphics::GL::catchErrors();

            glBindBuffer(GL_ARRAY_BUFFER, vboId);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, vboId);
            graphics::GL::catchErrors();

            GLuint valuesLocation = prepareProgramAttribute("VALUES_LOCATION", 1);
            glVertexAttribIPointer(valuesLocation, 4, GL_UNSIGNED_INT, sizeof(GLuint) * 4, 0);
            glEnableVertexAttribArray(valuesLocation);
            graphics::GL::catchErrors();

            unbind();
        }

        void ensureTransferBufferSize(unsigned int sizeBytes) {
            assertBound();

            if (sizeBytes > size) {
                glBufferData(GL_PIXEL_PACK_BUFFER, sizeBytes, 0, GL_DYNAMIC_COPY);
                graphics::GL::catchErrors();
                size = sizeBytes;
            }
        }

        ~TransferVao() {
            glDeleteBuffers(1, &vboId);
        }

    private:
        GLuint vboId;
        unsigned int size = 0;
    };

    static constexpr unsigned int maxDownloadSize = 1024 * 8;
    static constexpr unsigned int numDownloadVbos = 4;
    GLuint downloadVbos[numDownloadVbos];
    unsigned int nextDownloadVbo = 0;
};

}
