#include "button.h"

#include <cassert>

#include "collision.h"
#include "core.h"
#include "gameState.h"

void PressButton( GameData* data, Button* button ) {
    if (button == nullptr)
        return;
    assert(button->active);
    switch (button->type) {
        case ButtonType::START_GAME:
            ChangeScene(data, SCENE_TYPES::GAME);
            break;
        case ButtonType::QUIT:
            data->running = false;
            break;
        case ButtonType::NONE:
            assert(false);
            break;
    }
}

int GetActiveButtonCount( Button* buttons, int count ) {
    if (count == 0) {
        return 0;
    }
    int activeButtons = 0;
    for (int i = 0; i < count; i++) {
        if (buttons[i].active)
            activeButtons++;
    }
    return activeButtons;
}

bool IsHoveredOver( Button* button, float x, float y ) {
    if (button == nullptr || !button->active)
        return false;
    assert(button->type != ButtonType::NONE);
    return CheckCollisionInsideBounds(button->rect, x, y);
}

void SetupButton( Button* button, SpriteLibrary* sprites, ButtonType type, ButtonMode mode, SDL_FRect rect ) {
    assert(type != ButtonType::NONE);
    button->type = type;
    button->mode = mode;
    button->rect = rect;
    if (button->mode == ButtonMode::Centered) {
        button->rect.x -= rect.w / 2;
        button->rect.y -= rect.h / 2;
    }
    switch (button->type) {
        case ButtonType::START_GAME:
            button->texture = sprites->GetBySpriteID(SPRITE_ID::Fallback)->texture;
            break;
        case ButtonType::QUIT:
            button->texture = sprites->GetBySpriteID(SPRITE_ID::Fallback)->texture;
            break;
        default:
            button->texture = sprites->GetBySpriteID(SPRITE_ID::Fallback)->texture;
            break;
    }
    button->active = true;
}
