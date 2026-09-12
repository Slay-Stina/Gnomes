#pragma once
#include <SDL3/SDL_render.h>
#include "arena.h"
#include "entity.h"

const int NOT_SET = -1;

struct Sprite {
    SDL_Texture* texture;
    int width;
    int height;
    int pivot_x;
    int pivot_y;
    int sprite_count_x;
    int sprite_count_y;
};

inline int GetSpriteCount(Sprite* sprite) {
    if (sprite->sprite_count_x == NOT_SET)
        return 1;
    if (sprite->sprite_count_y == NOT_SET)
        return 1;
    return sprite->sprite_count_x * sprite->sprite_count_y;
}

struct SpriteRenderInfo {
    Sprite* sprite;
    int frame;
    bool flipped;

    SpriteRenderInfo() {
        sprite = nullptr;
        frame = 0;
        flipped = false;
    }

    SpriteRenderInfo(int frame, Sprite* sprite) {
        this->frame = frame;
        this->sprite = sprite;
        this->flipped = false;
    }

    SpriteRenderInfo(int frame, Sprite* sprite, bool flipped) {
        this->frame = frame;
        this->sprite = sprite;
        this->flipped = flipped;
    }

    SpriteRenderInfo(Sprite* sprite) {
        this->sprite = sprite;
        this->frame = 0;
        this->flipped = false;
    }
};


enum class SPRITE_ID {
    Fallback, Rock, Gnome_Rotate, Medusa_Rotate, Golem, Siren,
    Dropshadow, titlescreen_background, black_1x1,
    dungeon_tileset, selection_marker, COUNT // COUNT = antal, sista värdet
};

class SpriteLibrary {
public:
    void LoadAll(SDL_Renderer* renderer, Memory::Arena* arena);

    SpriteRenderInfo Get(ENTITY_ID id) const;

    SpriteRenderInfo GetSprite_FromEntityState(const Entity* entity) const;

    Sprite* GetBySpriteID(SPRITE_ID id) const;

private:
    Sprite* sprites[(int) SPRITE_ID::COUNT] = {};
};
