#include "mainmenu.h"

#include <cassert>

#include "button.h"
#include "common.h"
#include "gameState.h"
#include "rendering.h"
#include "spriteLibrary.h"

void Menu::Initialize(MainMenu* mainmenu, SpriteLibrary* sprites, Arena* arena_main) {
    assert(mainmenu->initialized == false);
    mainmenu->buttons_count = 2;
    mainmenu->buttons = ALLOC_ARRAY(arena_main, Button, mainmenu->buttons_count);

    SetupButton(&mainmenu->buttons[0], sprites, ButtonType::START_GAME, ButtonMode::Centered,
                {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 200, 80});
    SetupButton(&mainmenu->buttons[1], sprites, ButtonType::QUIT, ButtonMode::Centered,
                {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + 100, 200, 80});

    mainmenu->activeButtonIndex = 0;
    mainmenu->initialized = true;
}

void Menu::Draw(MainMenu* mainmenu, SDL_Renderer* renderer, SpriteLibrary* sprites) {
    Sprite* background = sprites->GetBySpriteID(SPRITE_ID::titlescreen_background);
    float scale = SCREEN_HEIGHT / ((float) background->height * UPSCALE_FACTOR);

    RenderSprite_World(background, renderer, nullptr, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, scale);

    for (int i = 0; i < mainmenu->activeButtonCount; i++) {
        RenderButton(mainmenu->activeButtons[i], i == mainmenu->activeButtonIndex, renderer);
    }
}

void Menu::Update(GameData* data) {
    MainMenu* mainmenu = &data->scenes.mainMenu;
    Input* input = &data->input;
    mainmenu->activeButtonCount = GetActiveButtonCount(mainmenu->buttons, mainmenu->buttons_count);
    if (mainmenu->activeButtonCount == 0) {
        return;
    }
    mainmenu->activeButtons = ALLOC_ARRAY(data->arena_scratch, Button*, mainmenu->activeButtonCount);
    int index = 0;
    for (int i = 0; i < mainmenu->buttons_count; i++) {
        if (mainmenu->buttons[i].active) {
            mainmenu->activeButtons[index++] = &mainmenu->buttons[i];
        }
    }
    int* buttonIndex = &mainmenu->activeButtonIndex;
    bool mouseMoving = input->mouse_magnitude > 0.1;
    if (mouseMoving) {
        for (int i = 0; i < mainmenu->activeButtonCount; i++) {
            Button* button = mainmenu->activeButtons[i];
            if (IsHoveredOver(button, input->mouse_x, input->mouse_y)) {
                *buttonIndex = i;
                break;
            }
        }
    }
    bool up = KeyPressed(input, SDL_SCANCODE_UP);
    bool down = KeyPressed(input, SDL_SCANCODE_DOWN);
    if (up || down) {
        int direction = up ? -1 : 1;
        *buttonIndex += direction + mainmenu->activeButtonCount;
        *buttonIndex = *buttonIndex % mainmenu->activeButtonCount;
    }
    Button* selected = mainmenu->activeButtons[*buttonIndex];
    if (selected != nullptr) {
        if (KeyPressed(input, SDL_SCANCODE_RETURN)) {
            PressButton(data, mainmenu->activeButtons[*buttonIndex]);
            return;
        }
    }
    if (IsHoveredOver(selected, input->mouse_x, input->mouse_y)) {
        if (MousePressed(input, MouseButtons::LEFT)) {
            PressButton(data, selected);
        }
    }
}
