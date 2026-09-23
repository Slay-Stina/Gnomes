#include "rendering.h"

#include "button.h"
#include "common.h"
#include "FontAtlas.h"

void RenderText( FontAtlas* atlas, const char* text, SDL_Renderer* renderer, Camera* camera, const float x,
                 const float y, Alignment mode ) {
    assert(atlas->AtlasTexture != nullptr);
    float draw_position_x = x;
    float draw_position_y = y;
    if (camera != nullptr) {
        draw_position_x -= camera->camera_x;
        draw_position_y -= camera->camera_y;
    }
    if (mode == Alignment::Centered) {
        float totalWidth = 0;
        for (int i = 0; text[i] != STOP_CHAR; i++) {
            totalWidth += atlas->GetGlyph(text[i]).atlasPosition.w;
        }
        draw_position_x -= totalWidth / 2.0;
    }
    for (int i = 0; text[i] != STOP_CHAR; i++) {
        Glyph glyph = atlas->GetGlyph(text[i]);
        SDL_FRect renderRectangle = {draw_position_x, draw_position_y, glyph.atlasPosition.w, glyph.atlasPosition.h};
        SDL_RenderTexture(renderer, atlas->AtlasTexture, &glyph.atlasPosition, &renderRectangle);
        draw_position_x += glyph.atlasPosition.w;
    }
}

void RenderButton( Button* button, bool is_selected, SDL_Renderer* renderer ) {
    uint8_t colorOverlay = is_selected ? 255 : 230;
    SDL_Texture* texture = button->sprite->texture;
    SDL_FRect rect = button->rect;

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture, colorOverlay, colorOverlay, colorOverlay);

    SDL_RenderTexture(renderer, button->sprite->texture, nullptr, &button->rect);
    if (!IsStringEmpty(button->text)) {
        float glyph_height = button->font->GetGlyph('H').atlasPosition.h / 2.0;
        RenderText(button->font, button->text, renderer, nullptr, rect.x + (rect.w / 2.0),
                   rect.y + (rect.h / 2.0) - glyph_height, Alignment::Centered);
    }
}

void RenderButton_Dynamic( Button* button, bool is_selected, SDL_Renderer* renderer ) {
    assert(button->sprite->sprite_count_x == 3);
    assert(button->sprite->sprite_count_y == 3);
    uint8_t colorOverlay = is_selected ? 255 : 230;
    SDL_Texture* texture = button->sprite->texture;
    SDL_FRect rect = button->rect;

    float part_w = texture->w / 3.0;
    float part_h = texture->h / 3.0;
    float vertical_center_height = rect.h - (part_h * 2);
    float horizontal_center_width = rect.w - (part_w * 2);
    float right_x = rect.x + rect.w - part_w;
    float bottom_y = rect.y + rect.h - part_h;
    float center_y = rect.y + part_h;
    float center_x = rect.x + part_w;

    SDL_FRect topLeftdst = {rect.x, rect.y, part_w, part_h};
    SDL_FRect topRightdst = {right_x, rect.y, part_w, part_h};
    SDL_FRect topCenterdst = {center_x, rect.y, horizontal_center_width, part_h};
    SDL_FRect bottomLeftdst = {rect.x, bottom_y, part_w, part_h};
    SDL_FRect bottomRightdst = {right_x, bottom_y, part_w, part_h};
    SDL_FRect bottomCenterdst = {center_x, bottom_y, horizontal_center_width, part_h};
    SDL_FRect centerLeftdst = {rect.x, center_y, part_w, vertical_center_height};
    SDL_FRect centerRightdst = {right_x, center_y, part_w, vertical_center_height};
    SDL_FRect centerdst = {center_x, center_y, horizontal_center_width, vertical_center_height};
    SDL_FRect topLeftsrc = {0, 0, part_w, part_h};
    SDL_FRect topRightsrc = {part_w * 2, 0, part_w, part_h};
    SDL_FRect topCentersrc = {part_w * 1, 0, part_w, part_h};
    SDL_FRect bottomLeftsrc = {0, part_h * 2, part_w, part_h};
    SDL_FRect bottomRightsrc = {part_w * 2, part_h * 2, part_w, part_h};
    SDL_FRect bottomCentersrc = {part_w * 1, part_h * 2, part_w, part_h};
    SDL_FRect centerLeftsrc = {0, part_h * 1, part_w, part_h};
    SDL_FRect centerRightsrc = {part_w * 2, part_h * 1, part_w, part_h};
    SDL_FRect centersrc = {part_w * 1, part_h * 1, part_w, part_h};

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture, colorOverlay, colorOverlay, colorOverlay);

    SDL_RenderTexture(renderer, texture, &topLeftsrc, &topLeftdst);
    SDL_RenderTexture(renderer, texture, &topCentersrc, &topCenterdst);
    SDL_RenderTexture(renderer, texture, &bottomCentersrc, &bottomCenterdst);
    SDL_RenderTexture(renderer, texture, &centerLeftsrc, &centerLeftdst);
    SDL_RenderTexture(renderer, texture, &centerRightsrc, &centerRightdst);
    SDL_RenderTexture(renderer, texture, &bottomLeftsrc, &bottomLeftdst);
    SDL_RenderTexture(renderer, texture, &topRightsrc, &topRightdst);
    SDL_RenderTexture(renderer, texture, &bottomRightsrc, &bottomRightdst);
    SDL_RenderTexture(renderer, texture, &centersrc, &centerdst);

    if (!IsStringEmpty(button->text)) {
        float glyph_height = button->font->GetGlyph('H').atlasPosition.h / 2.0;
        RenderText(button->font, button->text, renderer, nullptr, rect.x + (rect.w / 2.0),
                   rect.y + (rect.h / 2.0) - glyph_height, Alignment::Centered);
    }
}

void RenderTile( Sprite* tileset, int cell_id, LevelData* level, SDL_Renderer* renderer,
                 const Camera* camera, float x, float y, float scale, float alpha ) {
    camera::GridToWorld(&x, &y, level, camera->camera_z);
    RenderSprite_World({cell_id, tileset}, renderer, camera, x, y, scale, alpha, false);
}

void RenderSprite_OnTile( SpriteRenderInfo spriteInfo, LevelData* lvl, SDL_Renderer* renderer,
                          const Camera* camera, float x, float y, float scale, float alpha, bool flipped ) {
    float zoom = camera->camera_z;
    camera::GridToWorld(&x, &y, lvl, zoom);
    x += TILE_SIZE_PX_SCALED * zoom / 2.0f;
    y += TILE_SIZE_PX_SCALED * zoom / 2.0f;
    RenderSprite_World(spriteInfo, renderer, camera, x, y, scale, alpha, flipped);
}

void RenderSprite_World( SpriteRenderInfo spriteRenderInfo, SDL_Renderer* renderer, const Camera* camera,
                         float x, float y, float scale, float alpha, bool flipped ) {
    int frame = spriteRenderInfo.frame;
    Sprite* sprite = spriteRenderInfo.sprite;

    SDL_FRect tilesetRect;
    if (GetSpriteCount(sprite) > 1) {
        int width = sprite->width / sprite->sprite_count_x;
        int height = sprite->height / sprite->sprite_count_y;
        tilesetRect.w = width;
        tilesetRect.h = height;
        tilesetRect.x = frame % sprite->sprite_count_x * width;
        tilesetRect.y = frame / sprite->sprite_count_x * height;
    } else {
        tilesetRect.w = sprite->width;
        tilesetRect.h = sprite->height;
        tilesetRect.x = 0;
        tilesetRect.y = 0;
    }

    SDL_FRect rect;
    rect.x = x;
    rect.y = y;
    float zoom = (camera != nullptr) ? camera->camera_z : 1.0f;
    float final_scale = UPSCALE_FACTOR * scale * zoom;
    rect.h = tilesetRect.h * final_scale;
    rect.w = tilesetRect.w * final_scale;
    rect.x -= sprite->pivot_x * final_scale;
    rect.y -= sprite->pivot_y * final_scale;
    if (camera != nullptr) {
        rect.x -= camera->camera_x;
        rect.y -= camera->camera_y;
    }

    SDL_SetTextureScaleMode(sprite->texture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureAlphaModFloat(sprite->texture, alpha);
    SDL_FlipMode flip = flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, sprite->texture, &tilesetRect, &rect, 0, nullptr, flip);
}
