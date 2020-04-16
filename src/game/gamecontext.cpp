#include "gamecontext.h"

#include "spdlog/spdlog.h"

namespace game {

void GameContext::log(LogLevel level, const std::string &msg) {
    switch (level) {
        case LogLevel::Trace: spdlog::trace(msg); break;
        case LogLevel::Info: spdlog::info(msg); break;
        case LogLevel::Warning: spdlog::warn(msg); break;
        case LogLevel::Error: spdlog::error(msg); break;
    }
}

}
