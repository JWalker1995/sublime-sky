#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "jw_util/baseexception.h"
#include "game/gamecontext.h"

#include "graphics/gl.h"

namespace logger { class Logger; }

namespace graphics {

class GpuProgram {
public:
    class Exception : public jw_util::BaseException {
        friend class GpuProgram;

    private:
        Exception(const std::string &msg);
    };

    class Defines {
    public:
        template <typename DataType>
        void set(const std::string &key, const DataType val) {
            define_map[key] = std::to_string(val);
        }

        void writeInto(std::string &str) const;

    private:
        std::unordered_map<std::string, std::string> define_map;
    };

    GpuProgram(game::GameContext &context);
    ~GpuProgram();

    GLuint getProgramId() const {
        return program_id;
    }

    void attachShader(GLenum type, std::string &&source, const Defines &defines);

    void link();
    void assertLinked() const;

    void bind();
    void assertBound() const;

    void bindUniformBlock(const char *name, GLuint bindingPointIndex) const;

    static void printExtensions(game::GameContext &context);

protected:
    game::GameContext &context;

private:
    GLuint program_id;
    bool isLinked = false;
    std::vector<GLuint> shaders;
};

}
