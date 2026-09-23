#include "rendering.h"

#include "button.h"
#include "common.h"

void RenderButton( Button* button, bool is_selected, SDL_Renderer* renderer ) {
    SDL_Texture* texture = button->texture;
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
    uint8_t colorOverlay = is_selected ? 255 : 230;
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture, colorOverlay, colorOverlay, colorOverlay);
    SDL_RenderTexture(renderer, button->texture, nullptr, &button->rect);
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
