#include "levelRenderer.h"

#include <algorithm>
#include <cmath>

#include "rendering.h"

void RenderLevel(GameData* gameData, SDL_Renderer* renderer) {
    Gameplay* gameplay = &gameData->scenes.gameplay;
    LevelData* level = GetCurrentLevel(gameplay);

    Sprite* tileset;
    switch (level->tileset->type) {
        case TILESETS::Dungeon:
            tileset = gameData->sprites.GetBySpriteID(SPRITE_ID::dungeon_tileset);
            break;
        case TILESETS::NONE:
        case TILESETS::COUNT:
            assert(false);
            break;
    }

    for (int x = 0; x < level->w; x++) {
        for (int y = 0; y < level->h; y++) {
            uint16_t id = GetCellID(level, x, y);
            RenderTile(tileset, id, level, renderer, &gameData->camera, x, y, 1, 1);
        }
    }
}

bool IsEntityBelowOtherEntity(Entity* a, Entity* b) {
    return a->y < b->y;
}

void RenderEntities(GameData* data, SDL_Renderer* renderer) {
    Gameplay* gameplay = &data->scenes.gameplay;
    LevelData* lvl = GetCurrentLevel(gameplay);
    Entity** sortedEntities = ALLOC_ARRAY(data->arena_scratch, Entity*, lvl->entityCount);

    for (int i = 0; i < lvl->entityCount; i++) {
        sortedEntities[i] = &lvl->entityBuffer[i];
    }
    sort(sortedEntities, sortedEntities + lvl->entityCount, IsEntityBelowOtherEntity);

    Entity* activeEntity = gameplay->activePlayerBuffer[gameplay->activePlayerIndex];
    for (int i = 0; i < lvl->entityCount; i++) {
        Entity* entity = sortedEntities[i];
        if (!entity->active) {
            continue;
        }
        SpriteRenderInfo sprite = data->sprites.GetSprite_FromEntityState(entity);
        float x_animated = std::lerp(entity->x_prev, entity->x, entity->progress_01);
        float y_animated = std::lerp(entity->y_prev, entity->y, entity->progress_01);
        float ground_y = y_animated;
        if (entity->action == Actions::MOVING && HasBehaviour(entity, JUMPS) && !HasBehaviour(entity, IS_PUSHING)) {
            y_animated -= 0.5 * sinf(entity->progress_01 * M_PI);
        }
        Sprite* dropshadow = data->sprites.GetBySpriteID(SPRITE_ID::Dropshadow);
        RenderSprite_OnTile(dropshadow, lvl, renderer, &data->camera, x_animated, ground_y, 1, 0.4, false);
        if (entity == activeEntity) {
            SpriteRenderInfo marker = data->sprites.GetBySpriteID(SPRITE_ID::selection_marker);
            RenderSprite_OnTile(marker, lvl, renderer, &data->camera, x_animated, ground_y);
        }
        RenderSprite_OnTile(sprite, lvl, renderer, &data->camera, x_animated, y_animated, 1, 1, sprite.flipped);
    }
}
