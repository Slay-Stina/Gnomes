#pragma once
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

class SpriteLibrary;
struct GameData;

enum class ButtonMode {
    Centered, Raw
};

enum class ButtonType {
    NONE, START_GAME, QUIT
};

struct Button {
    ButtonType type;
    ButtonMode mode;
    SDL_FRect rect;
    SDL_Texture* texture;
    bool active;
};

void PressButton(GameData* data, Button* button);

int GetActiveButtonCount(Button* buttons, int count);

bool IsHoveredOver(Button* button, float x, float y);

void SetupButton(Button* button, SpriteLibrary* sprites, ButtonType type, ButtonMode mode, SDL_FRect rect);
