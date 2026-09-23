#pragma once
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

enum class Alignment;
class SpriteLibrary;
struct GameData;

enum class ButtonType {
    NONE, START_GAME, QUIT
};

struct Button {
    ButtonType type;
    Alignment mode;
    SDL_FRect rect;
    SDL_Texture* texture;
    bool active;
};

void PressButton( GameData* data, Button* button );

int GetActiveButtonCount( Button* buttons, int count );

bool IsHoveredOver( Button* button, float x, float y );

void SetupButton( Button* button, SpriteLibrary* sprites, ButtonType type, Alignment mode, SDL_FRect rect );
