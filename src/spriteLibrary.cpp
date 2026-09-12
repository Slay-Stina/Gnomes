#include "spriteLibrary.h"
#include <SDL3_image/SDL_image.h>
#include <cassert>

namespace {
    const char* FALLBACK_PATH = "assets/sprites/fallback.png";

    struct SpriteDataEntry {
        SPRITE_ID id;
        const char* path;
        int pivot_x = NOT_SET;
        int pivot_y = NOT_SET;
        int tileset_cell_count_x = NOT_SET;
        int tileset_cell_count_y = NOT_SET;
    };

    const SpriteDataEntry ALL_SPRITE_DATA[] = {
            {SPRITE_ID::Fallback, FALLBACK_PATH, 0, 0},
            {SPRITE_ID::Demon, "assets/sprites/player.png"},
            {SPRITE_ID::Rock, "assets/sprites/rock.png"},
            {SPRITE_ID::Golem, "assets/sprites/golem.png"},
            {SPRITE_ID::Medusa_Idle_Side, "assets/sprites/medusa_idle_side.png", 12, 24},
            {SPRITE_ID::Medusa_Idle_Front, "assets/sprites/medusa_idle_front.png", 12, 24},
            {SPRITE_ID::Medusa_Idle_Back, "assets/sprites/medusa_idle_back.png", 12, 24},
            {SPRITE_ID::Dropshadow, "assets/sprites/dropshadow.png", 8, 3},
            {SPRITE_ID::black_1x1, "assets/sprites/1x1_black.png", 0, 0},
            {SPRITE_ID::titlescreen_background, "assets/sprites/titlescreen.png", 0, 0},
            {SPRITE_ID::dungeon_tileset, "assets/sprites/hell_of_a_time_dungeon_tileset.png", 0, 0, 9, 9}
    };

    void LoadOne(Sprite* sprite, const SpriteDataEntry& entry, SDL_Renderer* renderer) {
        SDL_Surface* surface = IMG_Load(entry.path);
        if (surface == nullptr)
            surface = IMG_Load(FALLBACK_PATH);
        assert(surface != nullptr);
        sprite->texture = SDL_CreateTextureFromSurface(renderer, surface);
        sprite->width = sprite->texture->w;
        sprite->height = sprite->texture->h;
        sprite->tileset_cell_count_x = entry.tileset_cell_count_x;
        sprite->tileset_cell_count_y = entry.tileset_cell_count_y;
        if (entry.pivot_x == NOT_SET || entry.pivot_y == NOT_SET) {
            sprite->pivot_x = sprite->width / 2;
            sprite->pivot_y = sprite->height / 2;
        } else {
            sprite->pivot_x = entry.pivot_x;
            sprite->pivot_y = entry.pivot_y;
        }
        SDL_DestroySurface(surface);
    }
}

void SpriteLibrary::LoadAll(SDL_Renderer* renderer, Memory::Arena* arena) {
    for (const SpriteDataEntry& entry: ALL_SPRITE_DATA) {
        Sprite* sprite = ALLOC(arena, Sprite);
        LoadOne(sprite, entry, renderer);
        sprites[(int) entry.id] = sprite;
    }
}

Sprite* SpriteLibrary::Get(ENTITY_ID id) const {
    Sprite* result = nullptr;
    switch (id) {
        case ENTITY_ID::ROCK:
            result = sprites[(int) SPRITE_ID::Rock];
            break;
        case ENTITY_ID::DEMON:
            result = sprites[(int) SPRITE_ID::Demon];
            break;
        case ENTITY_ID::GOLEM:
            result = sprites[(int) SPRITE_ID::Golem];
            break;
        case ENTITY_ID::MEDUSA:
            result = sprites[(int) SPRITE_ID::Medusa_Idle_Front];
            break;
        default:
            result = sprites[(int) SPRITE_ID::Fallback];
            break;
    }
    if (result == nullptr || result->texture == nullptr) {
        result = sprites[(int) SPRITE_ID::Fallback];
    }
    return result;
}

Sprite* SpriteLibrary::GetFromEntity(const Entity* entity) const {
    if (HasBehaviour(entity, IS_PETRIFIED)) {
        return sprites[(int) SPRITE_ID::Rock];
    }
    if (entity->id == ENTITY_ID::MEDUSA) {
        switch (entity->facing) {
            case Direction::RIGHT:
            case Direction::LEFT:
                return sprites[(int) SPRITE_ID::Medusa_Idle_Side];
            case Direction::DOWN:
                return sprites[(int) SPRITE_ID::Medusa_Idle_Back];
            case Direction::UP:
                return sprites[(int) SPRITE_ID::Medusa_Idle_Front];
        }
    }
    return Get(entity->id);
}

Sprite* SpriteLibrary::GetBySpriteID(SPRITE_ID id) const {
    Sprite* s = sprites[(int) id];
    return (s && s->texture) ? s : sprites[(int) SPRITE_ID::Fallback];
}
