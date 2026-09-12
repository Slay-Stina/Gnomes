#pragma once
#include "spriteLibrary.h"
#include "camera.h"
#include "levels.h"

void RenderTile(Sprite* tileset, int cell_id, LevelData* level, SDL_Renderer* renderer,
                const Camera* camera, float x, float y, float scale, float alpha);

void RenderSprite_World(SpriteRenderInfo spriteInfo, SDL_Renderer* renderer, const Camera* camera,
                        float x, float y, float scale = 1, float alpha = 1, bool flipped = false);

void RenderSprite_OnTile(SpriteRenderInfo spriteInfo, LevelData* lvl, SDL_Renderer* renderer,
                         const Camera* camera, float x, float y, float scale = 1, float alpha = 1,
                         bool flipped = false);
