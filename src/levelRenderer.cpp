#include "levelRenderer.h"

#include <algorithm>
#include <cmath>

#include "rendering.h"

void RenderLevel(GameData* gameData, SDL_Renderer* renderer) {
    LevelData* lvl = gameData->GetCurrentLevel();
    for (int x = 0; x < lvl->w; x++) {
        for (int y = 0; y < lvl->h; y++) {
            uint8_t cellType = GetCellID(lvl, x, y);
            Sprite* sprite;
            if (ID(cellType) == ID::GROUND) {
                sprite = &gameData->spriteBuffer[(x + y) % 2 == 0
                                                     ? (int) SPRITE_ID::Ground
                                                     : (int) SPRITE_ID::Ground_alt];
            } else {
                sprite = GetSpriteFromID((ID) cellType, gameData->spriteBuffer);
            }
            RenderSprite_Grid(sprite, lvl, renderer, &gameData->camera, x, y);
        }
    }
}

bool IsEntityBelowOtherEntity(Entity* a, Entity* b) {
    return a->y < b->y;
}

void RenderEntities(GameData* data, SDL_Renderer* renderer) {
    LevelData* lvl = data->GetCurrentLevel();
    Entity** sortedEntities = ALLOC_ARRAY(data->arena_scratch, Entity*, lvl->entityCount);

    for (int i = 0; i < lvl->entityCount; i++) {
        sortedEntities[i] = &lvl->entityBuffer[i];
    }
    sort(sortedEntities, sortedEntities + lvl->entityCount, IsEntityBelowOtherEntity);
    for (int i = 0; i < lvl->entityCount; i++) {
        Entity* entity = sortedEntities[i];
        if (entity->id == ID::NONE) {
            continue;
        }
        Sprite* sprite = GetSprite_FromEntityState(entity, data->spriteBuffer);
        if (HasBehaviour(entity, IS_PETRIFIED)) {
            sprite = GetSpriteFromID(ID::ROCK, data->spriteBuffer);
        }
        float x_animated = std::lerp(entity->x_prev, entity->x, entity->progress_01);
        float y_animated = std::lerp(entity->y_prev, entity->y, entity->progress_01);
        float dropshadow_y = y_animated;
        if (HasBehaviour(entity, JUMPS) && !HasBehaviour(entity, IS_PUSHING)) {
            y_animated -= 0.5 * sinf(entity->progress_01 * M_PI);
        }
        Sprite* dropshadow = &data->spriteBuffer[(int) SPRITE_ID::Dropshadow];
        RenderEntity_OnTile(dropshadow, lvl, renderer, &data->camera, x_animated, dropshadow_y, 1, 0.4, false);
        RenderEntity_OnTile(sprite, lvl, renderer, &data->camera, x_animated, y_animated, 1, 1,
                            entity->facing == Direction::RIGHT);
    }
}
