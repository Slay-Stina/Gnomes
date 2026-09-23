#include "button.h"

#include <cassert>

#include "collision.h"
#include "common.h"
#include "core.h"
#include "gameState.h"
#include "rendering.h"

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

void SetupButton( Button* button, SpriteLibrary* sprites, ButtonType type, Alignment mode, SDL_FRect rect,
                  FontAtlas* font, const char* text, bool dynamic ) {
    assert(type != ButtonType::NONE);
    button->mode = mode;
    button->dynamic = dynamic;
    button->type = type;
    button->rect = rect;

    bool hasText = !IsStringEmpty(text);
    if (font == nullptr) {
        assert(!hasText);
    }
    if (hasText) {
        assert(font != nullptr);
        button->font = font;
        button->text = text;
    }

    if (button->dynamic) {
        FitButtonToText(button, 25);
    }
    if (button->mode == Alignment::Centered) {
        button->rect.x -= button->rect.w / 2;
        button->rect.y -= button->rect.h / 2;
    }
    button->active = true;
    switch (button->type) {
        case ButtonType::QUIT:
        case ButtonType::START_GAME:
            button->sprite = sprites->GetSprite(SPRITE_ID::Button_Basic);
            break;
        default:
            button->sprite = sprites->GetSprite(SPRITE_ID::Fallback);
            break;
    }
}

void FitButtonToText( Button* button, float padding ) {
    if (IsStringEmpty(button->text)) return;
    int w = 0, h = 0;
    for (int i = 0; i < (int) SDL_strlen(button->text); i++) {
        Glyph glyph = button->font->GetGlyph(button->text[i]);
        w += glyph.atlasPosition.w;
        h = h > glyph.atlasPosition.h ? h : glyph.atlasPosition.h;
    }
    button->rect.w = w + padding * 2;
    button->rect.h = h + padding * 2;
}
