#include "mainmenu.h"

#include <cassert>

#include "button.h"
#include "common.h"
#include "gameState.h"
#include "rendering.h"
#include "spriteLibrary.h"

void Menu::Initialize( MainMenu* mainmenu, SpriteLibrary* sprites, Arena* arena_main ) {
    assert(mainmenu->initialized == false);
    mainmenu->buttons_count = 2;
    mainmenu->buttons = ALLOC_ARRAY(arena_main, Button, mainmenu->buttons_count);

    SetupButton(&mainmenu->buttons[0], sprites, ButtonType::START_GAME, ButtonMode::Centered,
                {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 200, 80});
    SetupButton(&mainmenu->buttons[1], sprites, ButtonType::QUIT, ButtonMode::Centered,
                {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + 100, 200, 80});

    mainmenu->activeButtonIndex = 0;
    mainmenu->background_horizon = sprites->GetBySpriteID(SPRITE_ID::Menu_Horizon);
    mainmenu->background_cloud_back = sprites->GetBySpriteID(SPRITE_ID::Menu_Cloud_Back);
    mainmenu->background_cloud_front = sprites->GetBySpriteID(SPRITE_ID::Menu_Cloud_Front);
    mainmenu->background_middle = sprites->GetBySpriteID(SPRITE_ID::Menu_Middle);
    mainmenu->background_front = sprites->GetBySpriteID(SPRITE_ID::Menu_Front);
    mainmenu->initialized = true;
}

void Menu::Draw( MainMenu* mainmenu, SDL_Renderer* renderer, SpriteLibrary* sprites, Input* input ) {
    float scale = (SCREEN_HEIGHT / ((float) mainmenu->background_horizon->height * UPSCALE_FACTOR));
    scale *= 1.2;
    float mouse_x = input->mouse_x;
    float mouse_y = input->mouse_y;
    float center_x = SCREEN_WIDTH / 2.0;
    float center_y = SCREEN_HEIGHT / 2.0;
    float offset_x = center_x - mouse_x;
    float offset_y = center_y - mouse_y;
    RenderSprite_World(sprites->GetBySpriteID(SPRITE_ID::Menu_Horizon), renderer, nullptr, center_x, center_y, scale);
    RenderSprite_World(sprites->GetBySpriteID(SPRITE_ID::Menu_Cloud_Back), renderer, nullptr,
                       center_x + (offset_x / 11), center_y + (offset_y / 11), scale);
    RenderSprite_World(sprites->GetBySpriteID(SPRITE_ID::Menu_Cloud_Front), renderer, nullptr,
                       center_x + (offset_x / 9), center_y + (offset_y / 9), scale);
    RenderSprite_World(sprites->GetBySpriteID(SPRITE_ID::Menu_Middle), renderer, nullptr, center_x + (offset_x / 7),
                       center_y + (offset_y / 7), scale);
    RenderSprite_World(sprites->GetBySpriteID(SPRITE_ID::Menu_Front), renderer, nullptr, center_x + (offset_x / 5),
                       center_y + (offset_y / 5), scale);
    for (int i = 0; i < mainmenu->activeButtonCount; i++) {
        Button* button = mainmenu->activeButtons[i];
        RenderButton(mainmenu->activeButtons[i], i == mainmenu->activeButtonIndex, renderer);
    }
}

void Menu::Update( GameData* data ) {
    MainMenu* mainmenu = &data->scenes.mainMenu;
    Input* input = &data->input;
    mainmenu->activeButtonCount = GetActiveButtonCount(mainmenu->buttons, mainmenu->buttons_count);
    if (mainmenu->activeButtonCount == 0) {
        return;
    }
    mainmenu->activeButtons = ALLOC_ARRAY(data->arena_scratch, Button *, mainmenu->activeButtonCount);
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
