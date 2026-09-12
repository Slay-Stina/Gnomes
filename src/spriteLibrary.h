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
    int tileset_cell_count_x;
    int tileset_cell_count_y;
};

enum class SPRITE_ID {
    Fallback, Rock, Demon,
    Medusa_Idle_Side, Medusa_Idle_Front, Medusa_Idle_Back,
    Golem, Siren, Dropshadow, titlescreen_background, black_1x1,
    dungeon_tileset, COUNT // COUNT = antal, sista värdet
};

class SpriteLibrary {
public:
    void LoadAll(SDL_Renderer* renderer, Memory::Arena* arena);

    Sprite* Get(ENTITY_ID id) const;

    Sprite* GetFromEntity(const Entity* entity) const;

    Sprite* GetBySpriteID(SPRITE_ID id) const;

private:
    Sprite* sprites[(int) SPRITE_ID::COUNT] = {};
};
