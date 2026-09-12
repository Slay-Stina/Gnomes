#include "spriteLibrary.h"
#include <SDL3_image/SDL_image.h>
#include <cassert>
#include <cmath>

namespace {
    const char* FALLBACK_PATH = "assets/sprites/fallback.png";

    struct SpriteDataEntry {
        SPRITE_ID id;
        const char* path;
        int pivot_x = NOT_SET;
        int pivot_y = NOT_SET;
        int sprite_count_x = NOT_SET;
        int sprite_count_y = NOT_SET;
    };

    const SpriteDataEntry ALL_SPRITE_DATA[] = {
            {SPRITE_ID::Fallback, FALLBACK_PATH, 8, 8},
            {SPRITE_ID::Gnome_Rotate, "assets/sprites/gnome-Sheet.png", 8, 8, 3, 1},
            {SPRITE_ID::Rock, "assets/sprites/rock.png", 10, 20},
            {SPRITE_ID::Medusa_Rotate, "assets/sprites/medusa_rotate.png", 12, 24, 8, 1},
            {SPRITE_ID::Golem, "assets/sprites/golem.png"},
            {SPRITE_ID::Dropshadow, "assets/sprites/dropshadow.png", 8, 8},
            {SPRITE_ID::black_1x1, "assets/sprites/1x1_black.png", 0, 0},
            {SPRITE_ID::titlescreen_background, "assets/sprites/titlescreen.png", 0, 0},
            {SPRITE_ID::selection_marker, "assets/sprites/selection_marker.png", 9, 9},
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
        sprite->sprite_count_x = entry.sprite_count_x;
        sprite->sprite_count_y = entry.sprite_count_y;
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

SpriteRenderInfo SpriteLibrary::Get(ENTITY_ID id) const {
    SPRITE_ID spriteId = SPRITE_ID::Fallback;
    switch (id) {
        case ENTITY_ID::ROCK:
            spriteId = SPRITE_ID::Rock;
            break;
        case ENTITY_ID::GNOME:
            spriteId = SPRITE_ID::Gnome_Rotate;
            break;
        case ENTITY_ID::GOLEM:
            spriteId = SPRITE_ID::Golem;
            break;
        case ENTITY_ID::MEDUSA:
            return {4, GetBySpriteID(SPRITE_ID::Medusa_Rotate)};
        case ENTITY_ID::SIREN:
            break;
    }
    return GetBySpriteID(spriteId);
}

SpriteRenderInfo SpriteLibrary::GetSprite_FromEntityState(const Entity* entity) const {
    if (HasBehaviour(entity, IS_PETRIFIED)) {
        return GetBySpriteID(SPRITE_ID::Rock);
    }
    if (entity->id == ENTITY_ID::MEDUSA && entity->action == Actions::ROTATING) {
        Sprite* spritesheet = GetBySpriteID(SPRITE_ID::Medusa_Rotate);
        int start = 0;
        int end = 0;
        switch (entity->facing_previous) {
            case Direction::RIGHT:
                start = 6;
                break;
            case Direction::LEFT:
                start = 2;
                break;
            case Direction::UP:
                start = 0;
                break;
            case Direction::DOWN:
                start = 4;
                break;
        }
        switch (entity->facing_current) {
            case Direction::RIGHT:
                end = 6;
                break;
            case Direction::LEFT:
                end = 2;
                break;
            case Direction::UP:
                end = 0;
                break;
            case Direction::DOWN:
                end = 4;
                break;
        }
        int sprite_count = GetSpriteCount(spritesheet);
        int forward = ((end - start) % sprite_count + sprite_count) % sprite_count;
        int backward = sprite_count - forward;
        end = forward <= backward ? start + forward : start - backward;
        int current_frame = (int) std::lerp(start, end, entity->progress_01) % sprite_count;
        return {current_frame, spritesheet};
    }
    switch (entity->id) {
        case ENTITY_ID::MEDUSA: {
            Sprite* sprite = GetBySpriteID(SPRITE_ID::Medusa_Rotate);
            switch (entity->facing_current) {
                case Direction::RIGHT:
                    return {6, sprite};
                case Direction::LEFT:
                    return {2, sprite};
                case Direction::DOWN:
                    return {4, sprite};
                case Direction::UP:
                    return {0, sprite};
            }
            return GetBySpriteID(SPRITE_ID::Fallback);
        }
        case ENTITY_ID::GNOME: {
            Sprite* sprite = GetBySpriteID(SPRITE_ID::Gnome_Rotate);
            switch (entity->facing_current) {
                case Direction::RIGHT:
                    return {1, sprite, true};
                case Direction::LEFT:
                    return {1, sprite, false};
                case Direction::DOWN:
                    return {0, sprite};
                case Direction::UP:
                    return {2, sprite};
            }
            return GetBySpriteID(SPRITE_ID::Fallback);
        }
        case ENTITY_ID::ROCK:
            return GetBySpriteID(SPRITE_ID::Rock);
        default:
            return GetBySpriteID(SPRITE_ID::Fallback);
    }
}

Sprite* SpriteLibrary::GetBySpriteID(SPRITE_ID id) const {
    Sprite* s = sprites[(int) id];
    return s && s->texture ? s : sprites[(int) SPRITE_ID::Fallback];
}
