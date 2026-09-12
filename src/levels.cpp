#include "levels.h"

#include "arena.h"
#include "entity.h"

#include <cstdint>
#include <fstream>
#include <json/json.h>

using namespace std;

const int LEVEL_INDEX = 0;
const int ENTITIES_INDEX = 1;

uint8_t GetCellID(LevelData* level, int x, int y) {
    assert(! (x < 0 || x >= level->w || y < 0 || y >= level->h));
    return level->cells[y * level->w + x];
}

Entity* GetEntity(LevelData* level, int x, int y) {
    for (int i = 0; i < level->entityCount; i++) {
        if (level->entityBuffer[i].x == x && level->entityBuffer[i].y == y) {
            return &level->entityBuffer[i];
        }
    }
    return nullptr;
}

void CreateLevel(Arena* arena, LevelData* level, const char* level_name) {
    ifstream stream(level_name);
    Json::CharReaderBuilder reader;
    Json::Value jsonResult;
    if (!Json::parseFromStream(reader, stream, &jsonResult, nullptr)) {
        return;
    }

    const Json::Value& dataField = jsonResult["layers"][LEVEL_INDEX]["data"];
    level->w = jsonResult["width"].asInt();
    level->h = jsonResult["height"].asInt();
    level->level_path = level_name;

    size_t size_of_cells = sizeof(uint8_t) * level->w * level->h;
    level->cells = (uint8_t*) Allocate(arena, size_of_cells);
    for (int i = 0; i < level->w * level->h; i++) {
        level->cells[i] = dataField[i].asUInt();
    }
}

void CreateEntities(LevelData* lvl_data, Arena* arena) {
    Reset(arena);
    lvl_data->entityCount = 0;

    ifstream stream(lvl_data->level_path);
    Json::CharReaderBuilder reader;
    Json::Value result;
    if (!Json::parseFromStream(reader, stream, &result, nullptr)) {
        return;
    }

    const Json::Value& entityData = result["layers"][ENTITIES_INDEX]["data"];
    for (int i = 0; i < lvl_data->w * lvl_data->h; i++) {
        unsigned char entity_id = entityData[i].asUInt();
        if (entity_id != 0) {
            lvl_data->entityCount++;
        }
    }

    lvl_data->entityBuffer = ALLOC_ARRAY(arena, Entity, MAX_NUM_ENTITIES);
    for (int i = 0; i < lvl_data->w * lvl_data->h; i++) {
        unsigned char entity_id = entityData[i].asUInt();
        if (entity_id != 0) {
            int x = i % lvl_data->w;
            int y = i / lvl_data->w;
            AddEntity((ID) entity_id, x, y, lvl_data);
        }
    }
}

Entity* GetNextAvailableEntity(LevelData* level) {
    for (int i = 0; i < level->entityCount; i++) {
        if (level->entityBuffer[i].id == ID::NONE) {
            return &level->entityBuffer[i];
        }
    }
    if (level->entityCount >= MAX_NUM_ENTITIES) {
        return nullptr;
    }
    return &level->entityBuffer[level->entityCount++];
}

void AddEntity(ID entity_id, int x, int y, LevelData* level) {
    Entity* entity = GetEntity(level, x, y);
    if (entity == nullptr) {
        entity = GetNextAvailableEntity(level);
        if (entity == nullptr) {
            return;
        }
    }
    entity->x = x;
    entity->y = y;
    entity->x_prev = x;
    entity->y_prev = y;
    entity->id = entity_id;
    InitializeBaseBehaviour(entity);
}

void RemoveEntity(int x, int y, LevelData* level) {
    Entity* entity = GetEntity(level, x, y);
    if (entity == nullptr) {
        return;
    }
    *entity = {};
    entity->x = -1;
    entity->y = -1;
}

Entity* RaycastFirstEntity(int x_origin, int y_origin, Direction direction, LevelData* level, bool ignore_walls) {
    Position facingVector{};
    switch (direction) {
        case Direction::RIGHT:
            facingVector = {1, 0};
            break;
        case Direction::LEFT:
            facingVector = {-1, 0};
            break;
        case Direction::UP:
            facingVector = {0, 1};
            break;
        case Direction::DOWN:
            facingVector = {0, -1};
            break;
    }
    int x_search = x_origin + facingVector.x;
    int y_search = y_origin + facingVector.y;
    while (x_search >= 0 && x_search < level->w && y_search >= 0 && y_search < level->h) {
        ID cellID = (ID) GetCellID(level, x_search, y_search);
        if (cellID == ID::WALL && !ignore_walls) {
            break;
        }
        Entity* entity_search = GetEntity(level, x_search, y_search);
        if (entity_search != nullptr) {
            return entity_search;
        }
        x_search += facingVector.x;
        y_search += facingVector.y;
    }
    return nullptr;
}
