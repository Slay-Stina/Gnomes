#pragma once
#include "arena.h"
#include "entity.h"

#include <cstdint>

#include "tilesetLibrary.h"
using namespace std;

using namespace Memory;

const int MAX_NUM_ENTITIES = 256;

struct LevelData {
    int w;
    int h;
    int entityCount;
    uint16_t* cells;
    const char* level_path;
    Entity* entityBuffer;
    const Tileset* tileset;
};

void CreateLevel(Arena* arena, LevelData* level, const Tileset* tileset, const char* level_name);

void CreateEntities(LevelData* lvl_data, Arena* arena);

Entity* GetNextAvailableEntity(LevelData* level);

void AddEntity(ENTITY_ID entity_id, int x, int y, LevelData* level);

void RemoveEntity(int x, int y, LevelData* level);

uint16_t GetCellID(LevelData* level, int x, int y);

bool IsWalkable(int x, int y, LevelData* level);

Entity* GetEntity(LevelData* level, int x, int y);

Entity* RaycastFirstEntity(int x_origin, int y_origin, Direction direction, LevelData* level,
                           bool ignore_walls = false);
