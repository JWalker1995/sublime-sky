#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "flatbuffers/idl.h"

#include "game/gamecontext.h"

#include "schemas/config_client_generated.h"
#include "schemas/config_game_generated.h"
#include "config/configsupplier.h"
#include "game/mainloop.h"
#include "application/quitexception.h"


int main(int argc, char **argv)
{
    // https://picroma.com/cubeworld
    // http://cmusphinx.sourceforge.net/
    // http://www.openal-soft.org/

    // http://feedback.wildfiregames.com/report/opengl/

    /*
    flatbuffers::FlatBufferBuilder builder;
    flatbuffers::Offset<Monster> offset1;
    flatbuffers::Offset<Monster> offset2;
    flatbuffers::Offset<Monster> offset3;
    {
        flatbuffers::Offset<flatbuffers::String> name = builder.CreateString("Orc");

        MonsterBuilder monsterBuilder(builder);
        Vec3 pos(1.0f, 2.0f, 3.0f);
        monsterBuilder.add_pos(&pos);
        monsterBuilder.add_name(name);
        monsterBuilder.add_hp(150);
        offset1 = monsterBuilder.Finish();
        std::cout << offset1.o << std::endl;
    }
    {
        flatbuffers::Offset<flatbuffers::String> name = builder.CreateString("Orc");

        MonsterBuilder monsterBuilder(builder);
        Vec3 pos(1.0f, 2.0f, 3.0f);
        monsterBuilder.add_pos(&pos);
        monsterBuilder.add_name(name);
        monsterBuilder.add_hp(200);
        offset2 = monsterBuilder.Finish();
        std::cout << offset2.o << std::endl;
    }
    {
        flatbuffers::Offset<flatbuffers::String> name = builder.CreateString("Orc");

        MonsterBuilder monsterBuilder(builder);
        Vec3 pos(1.0f, 2.0f, 3.0f);
        monsterBuilder.add_pos(&pos);
        monsterBuilder.add_name(name);
        monsterBuilder.add_hp(250);
        offset3 = monsterBuilder.Finish();
        std::cout << offset3.o << std::endl;
    }

    std::cout << flatbuffers::GetTemporaryPointer<Monster>(builder, offset1)->hp() << std::endl;
    std::cout << flatbuffers::GetTemporaryPointer<Monster>(builder, offset2)->hp() << std::endl;
    std::cout << flatbuffers::GetTemporaryPointer<Monster>(builder, offset3)->hp() << std::endl;

    return 0;
    */

    // Setup context
    game::GameContext context;

    // Add logger to context
    context.provideInstance(spdlog::default_logger().get());

    // Print some debug info
#ifndef NDEBUG
    context.get<spdlog::logger>().warn("This is a debug build!");
    std::cout << "A pointer is " << (sizeof(void*) * CHAR_BIT) << " bits" << std::endl;
    std::cout << "An unsigned int is " << (sizeof(unsigned int) * CHAR_BIT) << " bits" << std::endl;
#endif

    context.get<spdlog::logger>().info("Starting...");

    // Parse command line options
    if (argc != 2 && argc != 3) {
        context.get<spdlog::logger>().critical("Invalid command line options");
        return 1;
    }

    const unsigned char clientSchemaStr[] = {
        #include "schemas/config_client.fbs.h"
    };
    config::ConfigSupplier<SsProtocol::Config::Client> clientConfigSupplier(context, reinterpret_cast<const char *>(clientSchemaStr));
    context.provideInstance(&clientConfigSupplier);
    clientConfigSupplier.loadFromJsonFile(argv[1]);

    const unsigned char gameSchemaStr[] = {
        #include "schemas/config_game.fbs.h"
    };
    config::ConfigSupplier<SsProtocol::Config::Game> gameConfigSupplier(context, reinterpret_cast<const char *>(gameSchemaStr));
    context.provideInstance(&gameConfigSupplier);
    if (argc > 2) {
        gameConfigSupplier.loadFromJsonFile(argv[2]);
    }

    SsProtocol::Config::LogLevel logLevel = context.get<const SsProtocol::Config::Client>().log_level();
    const char *logLevelStr = SsProtocol::Config::EnumNameLogLevel(logLevel);
    context.get<spdlog::logger>().info("Setting log level to \"{}\"", logLevelStr);

    switch (logLevel) {
        case SsProtocol::Config::LogLevel_trace: context.get<spdlog::logger>().set_level(spdlog::level::trace); break;
        case SsProtocol::Config::LogLevel_debug: context.get<spdlog::logger>().set_level(spdlog::level::debug); break;
        case SsProtocol::Config::LogLevel_info: context.get<spdlog::logger>().set_level(spdlog::level::info); break;
        case SsProtocol::Config::LogLevel_warning: context.get<spdlog::logger>().set_level(spdlog::level::warn); break;
        case SsProtocol::Config::LogLevel_error: context.get<spdlog::logger>().set_level(spdlog::level::err); break;
        case SsProtocol::Config::LogLevel_critical: context.get<spdlog::logger>().set_level(spdlog::level::critical); break;
        case SsProtocol::Config::LogLevel_off: context.get<spdlog::logger>().set_level(spdlog::level::off); break;
    }

    // Run the game!!!
    try {
        context.get<game::MainLoop>().run();
    } catch (const application::QuitException &) {
        // Normal exit
        context.get<spdlog::logger>().info("GameContext type counts: managed={}, total={}", context.getManagedTypeCount(), context.getTotalTypeCount());
        context.get<spdlog::logger>().info("Ending...");
        return 0;
    } catch (const std::exception &exception) {
        context.get<spdlog::logger>().critical("Uncaught exception: {}", exception.what());
        return 1;
    }
}
