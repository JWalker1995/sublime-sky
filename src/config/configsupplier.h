#pragma once

#include <fstream>
#include <string>

#include "flatbuffers/idl.h"

#include "jw_util/baseexception.h"

#include "game/gamecontext.h"

namespace config {

template <typename ConfigType>
class ConfigSupplier {
public:
    class Exception : public jw_util::BaseException {
        friend class ConfigSupplier;

    private:
        Exception(const std::string &msg)
            : BaseException(msg)
        {}
    };

    ConfigSupplier(game::GameContext &context, const char *schemaCode)
        : context(context)
        , schemaCode(schemaCode)
    {}

    void loadFromJsonFile(const char *filename) {
        assert(filename);

        // Parse command line options
        std::ifstream t(filename);
        t.seekg(0, std::ios::end);
        std::size_t size = t.tellg();
        char *json = new char[size + 1];
        t.seekg(0);
        t.read(json, size);
        json[size] = '\0';

        flatbuffers::Parser parser;

        // Parse config schema file into flatbuffers schema
        if (!parser.Parse(schemaCode)) {
            throw Exception("Cannot load config schema: " + parser.error_);
        }

        // Parse config json into config buffer
        if (!parser.Parse(json)) {
            throw Exception("Cannot load config json: " + parser.error_);
        }

        loadFromBuffer(parser.builder_.GetBufferPointer(), parser.builder_.GetSize());

        delete[] json;
    }

    void loadFromBuffer(const std::uint8_t *buffer, std::size_t size) {
        assert(configBuffer.empty());
        assert(buffer);

        configBuffer.resize(size);
        assert(reinterpret_cast<std::uintptr_t>(configBuffer.data()) % 8 == 0);
        std::copy_n(buffer, size, configBuffer.data());
        const ConfigType *config = flatbuffers::GetRoot<ConfigType>(configBuffer.data());

        // Add config to context
        context.provideInstance(config);
    }

    const std::uint8_t *getConfigBufferData() const {
        return configBuffer.data();
    }

    std::size_t getConfigBufferSize() const {
        return configBuffer.size();
    }

private:
    game::GameContext &context;
    const char *schemaCode;
    std::vector<std::uint8_t> configBuffer;
};

}
