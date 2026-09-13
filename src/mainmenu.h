#pragma once

namespace Memory {
    struct Arena;
}

struct GameData;
struct SDL_Renderer;
struct Button;
class SpriteLibrary;

struct MainMenu {
    Button* buttons;
    int buttons_count;
    int activeButtonIndex;
    Button** activeButtons;
    int activeButtonCount;
    bool initialized;
};

namespace Menu {
    void Initialize(MainMenu* mainmenu, SpriteLibrary* sprites, Memory::Arena* arena_main);

    void Update(GameData* data);

    void Draw(MainMenu* mainmenu, SDL_Renderer* renderer, SpriteLibrary* sprites);
}
