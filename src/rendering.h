#pragma once
#include "spriteLibrary.h"
#include "camera.h"
#include "levels.h"

void RenderSprite_World(Sprite* sprite, SDL_Renderer* renderer, const Camera* camera, float x, float y,
                        float scale = 1);

void RenderSprite_Grid(Sprite* sprite, LevelData* lvl, SDL_Renderer* renderer, const Camera* camera, float x, float y,
                       float scale = 1);
