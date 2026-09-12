#include "levels.h"

#include "arena.h"
#include "entity.h"

#include <cstdint>
#include <fstream>
#include <json/json.h>

using namespace std;

const int LEVEL_INDEX = 0;
const int ENTITIES_INDEX = 1;

uint16_t GetCellID(LevelData* level, int x, int y) {
    assert(! (x < 0 || x >= level->w || y < 0 || y >= level->h));
    return level->cells[y * level->w + x];
}

bool IsWalkable(int x, int y, LevelData* level) {
    uint16_t id = GetCellID(level, x, y);
    return level->tileset->walkableBuffer[id];
}

Entity* GetEntity(LevelData* level, int x, int y) {
    for (int i = 0; i < level->entityCount; i++) {
        if (level->entityBuffer[i].active && level->entityBuffer[i].x == x && level->entityBuffer[i].y == y) {
            return &level->entityBuffer[i];
        }
    }
    return nullptr;
}

void CreateLevel(Arena* arena, LevelData* level, const Tileset* tileset, const char* level_name) {
    ifstream stream(level_name);
    Json::CharReaderBuilder reader;
    Json::Value result;
    if (!Json::parseFromStream(reader, stream, &result, nullptr)) {
        return;
    }

    // Hitta lagret som heter "level"
    const Json::Value* levelLayer = nullptr;
    for (const Json::Value& layer: result["layers"]) {
        if (layer["name"].asString() == "level") {
            levelLayer = &layer;
            break;
        }
    }
    assert(levelLayer != nullptr);
    const Json::Value& levelData = (*levelLayer)["data"];

    // Hitta första icke-noll-id för att bestämma tileset-offset
    int first_non_zero_id = 0;
    for (const Json::Value& value: levelData) {
        if (value.asInt() != 0) {
            first_non_zero_id = value.asInt();
            break;
        }
    }
    int id_offset = Get_Tileset_ID_Offset_From_Tilemap(first_non_zero_id, result);

    level->w = result["width"].asInt();
    level->h = result["height"].asInt();
    level->level_path = level_name;
    level->tileset = tileset;
    level->cells = ALLOC_ARRAY(arena, uint16_t, level->w * level->h);

    for (int i = 0; i < level->w * level->h; i++) {
        int local_id = levelData[i].asInt() - id_offset;
        if (local_id < 0) {
            local_id = 0;
        }
        level->cells[i] = local_id;
    }
}

void CreateEntities(LevelData* lvl_data, Arena* arena) {
    Reset(arena);
    lvl_data->entityCount = 0;
    lvl_data->entityBuffer = ALLOC_ARRAY(arena, Entity, MAX_NUM_ENTITIES);

    ifstream stream(lvl_data->level_path);
    Json::CharReaderBuilder reader;
    Json::Value result;
    if (!Json::parseFromStream(reader, stream, &result, nullptr)) {
        return;
    }

    const Json::Value* entityLayer = nullptr;
    for (const Json::Value& layer: result["layers"]) {
        if (layer["name"].asString() == "entities") {
            entityLayer = &layer;
            break;
        }
    }
    if (entityLayer == nullptr) {
        return;
    }
    const Json::Value& entities = (*entityLayer)["data"];

    for (int i = 0; i < lvl_data->w * lvl_data->h; i++) {
        int raw = entities[i].asInt();
        if (raw == 0) {
            continue;
        }
        uint16_t entity_id = GetLocalTileID(raw, result);
        int x = i % lvl_data->w;
        int y = i / lvl_data->w;
        AddEntity((ENTITY_ID) entity_id, x, y, lvl_data);
    }
}

Entity* GetNextAvailableEntity(LevelData* level) {
    for (int i = 0; i < level->entityCount; i++) {
        if (!level->entityBuffer[i].active) {
            return &level->entityBuffer[i];
        }
    }
    if (level->entityCount >= MAX_NUM_ENTITIES) {
        return nullptr;
    }
    return &level->entityBuffer[level->entityCount++];
}

void AddEntity(ENTITY_ID entity_id, int x, int y, LevelData* level) {
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
    entity->active = true;
    InitializeBaseBehaviour(entity);
}

void RemoveEntity(int x, int y, LevelData* level) {
    Entity* entity = GetEntity(level, x, y);
    if (entity == nullptr) {
        return;
    }
    entity->active = false;
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
        if (!IsWalkable(x_search, y_search, level) && !ignore_walls) {
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
