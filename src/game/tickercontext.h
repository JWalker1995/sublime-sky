#pragma once

#include "jw_util/context.h"
#include "jw_util/contextbuilder.h"

#include "game/gamecontext.h"

namespace game {

class GameContext;

class TickerContext : public jw_util::Context<TickerContext> {
public:
    template <typename ClassType>
    class TickCaller {
    public:
        TickCaller(TickerContext &tickerContext) {
            tickerContext.getGameContext().template get<ClassType>().tick(tickerContext);
        }
    };

    template <typename ClassType>
    class ScopedCaller {
    public:
        ScopedCaller(TickerContext &tickerContext)
            : tickerContext(tickerContext)
        {
            tickerContext.getGameContext().template get<ClassType>().tickOpen(tickerContext);
        }

        ~ScopedCaller() {
            tickerContext.getGameContext().template get<ClassType>().tickClose(tickerContext);
        }

    private:
        TickerContext &tickerContext;
    };

    template <typename DerivedType, typename TickerType = TickCaller<DerivedType>>
    class TickableBase {
    public:
        typedef TickerType Ticker;

        TickableBase(game::GameContext &context)
            : context(context)
        {
            context.get<TickerContext>().template insert<TickerType>();
        }

        ~TickableBase() {
            context.get<TickerContext>().template remove<TickerType>();
        }

    protected:
        game::GameContext &context;
    };


    TickerContext(game::GameContext &gameContext)
        : Context(0.1f)
        , gameContext(gameContext)
    {}

    ~TickerContext() {
        assert(builder.getSize() == 0);
    }

    GameContext &getGameContext() {
        return gameContext;
    }

    template <typename TickerType>
    void insert() {
        builder.registerConstructor<TickerType>();
    }

    template <typename TickerType>
    void remove() {
        builder.removeConstructor<TickerType>();
    }

    void tick();

    void log(LogLevel level, const std::string &msg);

private:
    game::GameContext &gameContext;
    jw_util::ContextBuilder<TickerContext> builder;
};

}
