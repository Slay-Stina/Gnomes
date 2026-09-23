#pragma once
#include <SDL3/SDL_rect.h>

#include "FontAtlas.h"

struct FontAtlas;
struct Sprite;
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
    Sprite* sprite;
    bool active;
    bool dynamic;
    FontAtlas* font;
    const char* text;
};

void PressButton( GameData* data, Button* button );

int GetActiveButtonCount( Button* buttons, int count );

bool IsHoveredOver( Button* button, float x, float y );

void SetupButton( Button* button, SpriteLibrary* sprites, ButtonType type, Alignment mode, SDL_FRect rect,
                  FontAtlas* font = nullptr,
                  const char* text = nullptr,
                  bool dynamic = false );

void FitButtonToText( Button* button, float padding );
