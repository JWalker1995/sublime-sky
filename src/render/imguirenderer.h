#pragma once

#include "graphics/gl.h"

#include "game/gamecontext.h"
#include "game/tickercontext.h"

namespace render {

class ImguiRenderer : public game::TickerContext::TickableBase<ImguiRenderer, game::TickerContext::ScopedCaller<ImguiRenderer>> {
public:
    ImguiRenderer(game::GameContext &context);
    ~ImguiRenderer();

    void tickOpen(game::TickerContext &tickerContext);
    void tickClose(game::TickerContext &tickerContext);

private:
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void charCallback(GLFWwindow *window, unsigned int c);

    static GLFWmousebuttonfun prevMouseButtonCallback;
    static GLFWscrollfun prevScrollCallback;
    static GLFWkeyfun prevKeyCallback;
    static GLFWcharfun prevCharCallback;
};

}
