#pragma once
#include "arena.h"
#include "entity.h"

#include <cstdint>
using namespace std;

using namespace Memory;

const int MAX_NUM_ENTITIES = 256;

struct LevelData {
    int w;
    int h;
    uint8_t* cells;
    const char* level_path;
    Entity* entityBuffer;
    int entityCount;
};

void CreateLevel(Arena* arena, LevelData* level, const char* level_name);

void CreateEntities(LevelData* lvl_data, Arena* arena);

Entity* GetNextAvailableEntity(LevelData* level);

void AddEntity(ID entity_id, int x, int y, LevelData* level);

void RemoveEntity(int x, int y, LevelData* level);

uint8_t GetCellID(LevelData* level, int x, int y);

Entity* GetEntity(LevelData* level, int x, int y);

Entity* RaycastFirstEntity(int x_origin, int y_origin, Direction direction, LevelData* level,
                           bool ignore_walls = false);
