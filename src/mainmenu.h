#pragma once

struct Input;

namespace Memory {
    struct Arena;
}

struct GameData;
struct SDL_Renderer;
struct Button;
struct Sprite;
class SpriteLibrary;

struct MainMenu {
    Button* buttons;
    int buttons_count;
    int activeButtonIndex;
    Button** activeButtons;
    int activeButtonCount;
    bool initialized;
    Sprite* background_horizon;
    Sprite* background_cloud_back;
    Sprite* background_cloud_front;
    Sprite* background_middle;
    Sprite* background_front;
};

namespace Menu {
    void Initialize(MainMenu* mainmenu, SpriteLibrary* sprites, Memory::Arena* arena_main);

    void Update(GameData* data);

    void Draw(MainMenu* mainmenu, SDL_Renderer* renderer, SpriteLibrary* sprites, Input* input);
}
