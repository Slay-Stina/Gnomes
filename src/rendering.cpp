#include "rendering.h"

#include "common.h"

void RenderSprite_World(Sprite* sprite, SDL_Renderer* renderer, const Camera* camera, float x, float y, float scale) {
    SDL_FRect rect;
    rect.x = x;
    rect.y = y;
    rect.h = sprite->height * UPSCALE_FACTOR * scale;
    rect.w = sprite->width * UPSCALE_FACTOR * scale;
    rect.x -= camera->camera_x;
    rect.y -= camera->camera_y;
    SDL_RenderTexture(renderer, sprite->texture, nullptr, &rect);
}

void RenderSprite_Grid(Sprite* sprite, LevelData* lvl, SDL_Renderer* renderer, const Camera* camera, float x, float y,
                       float scale) {
    camera::GridToWorld(&x, &y, lvl);
    RenderSprite_World(sprite, renderer, camera, x, y, scale);
}
