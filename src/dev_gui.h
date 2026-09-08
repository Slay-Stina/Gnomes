#pragma once
#include <SDL3/SDL.h>
#include <imgui.h>
#include "gameState.h"

namespace DEV {
    void Initialize(SDL_Window * window, SDL_Renderer * renderer);
    void ProcessEvents(SDL_Event * event);
    void PreDraw(ImGuiContext * saved_context);
    void Draw(GameData * data, SDL_Renderer * renderer);
}