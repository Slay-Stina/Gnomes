#include "rendering.h"

#include "common.h"

void RenderSprite_World(Sprite* sprite, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale,
                        float alpha, bool flipped) {
    SDL_FRect rect;
    rect.x = x;
    rect.y = y;
    float final_scale = UPSCALE_FACTOR * scale;
    rect.h = sprite->height * final_scale;
    rect.w = sprite->width * final_scale;
    rect.x -= sprite->pivot_x * final_scale;
    rect.y -= sprite->pivot_y * final_scale;
    rect.x -= camera->camera_x;
    rect.y -= camera->camera_y;
    SDL_SetTextureScaleMode(sprite->texture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureAlphaModFloat(sprite->texture, alpha);
    SDL_RenderTextureRotated(renderer, sprite->texture, nullptr, &rect, 0.0, nullptr,
                             flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void RenderSprite_Grid(Sprite* sprite, LevelData* lvl, SDL_Renderer* renderer, const Camera* camera, float x, float y,
                       float scale, float alpha, bool flipped) {
    camera::GridToWorld(&x, &y, lvl);
    RenderSprite_World(sprite, renderer, camera, x, y, scale, alpha, flipped);
}

void RenderEntity_OnTile(Sprite* sprite, LevelData* lvl, SDL_Renderer* renderer, const Camera* camera, float x, float y,
                         float scale, float alpha, bool flipped) {
    camera::GridToWorld(&x, &y, lvl);
    x += CELL_SIZE_PX / 2.0;
    y += CELL_SIZE_PX / 2.0;
    RenderSprite_World(sprite, renderer, camera, x, y, scale, alpha, flipped);
}
