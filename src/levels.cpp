#include "levels.h"

#include "arena.h"
#include "entity.h"

#include <cstdint>
#include <fstream>
#include <json/json.h>

#include "common.h"

using namespace std;

const int LEVEL_INDEX = 0;
const int ENTITIES_INDEX = 1;

uint16_t GetCellID( LevelData* level, int x, int y ) {
    assert(!(x < 0 || x >= level->w || y < 0 || y >= level->h));
    return level->cells[y * level->w + x];
}

bool IsWalkable( int x, int y, LevelData* level ) {
    uint16_t id = GetCellID(level, x, y);
    return level->tileset->walkableBuffer[id];
}

Entity* GetEntity( LevelData* level, int x, int y ) {
    for (int i = 0; i < level->entityCount; i++) {
        if (level->entityBuffer[i].active && level->entityBuffer[i].x == x && level->entityBuffer[i].y == y) {
            return &level->entityBuffer[i];
        }
    }
    return nullptr;
}

void CreateLevel( Arena* arena, LevelData* level, const Tileset* tileset, const char* level_name ) {
    ifstream stream(level_name);
    Json::CharReaderBuilder reader;
    Json::Value result;
    if (!Json::parseFromStream(reader, stream, &result, nullptr)) {
        return;
    }
    bool found = false;
    vector<uint16_t> levelData = AssetManagement::GetCellDataFromJsonLayer(result, "level", &found);
    assert(found);

    // Hitta första icke-noll-id för att bestämma tileset-offset
    int first_non_zero_id = AssetManagement::GetFirstNonZeroCell(&levelData);
    int id_offset = Get_Tileset_ID_Offset_From_Tilemap(first_non_zero_id, result);

    level->w = result["width"].asInt();
    level->h = result["height"].asInt();
    level->level_path = level_name;
    level->tileset = tileset;
    level->cells = ALLOC_ARRAY(arena, uint16_t, level->w * level->h);

    for (int i = 0; i < level->w * level->h; i++) {
        int local_id = levelData[i] - id_offset;
        if (local_id < 0) {
            local_id = 0;
        }
        level->cells[i] = local_id;
    }

    level->goals = nullptr;
    level->goalCount = 0;
    bool foundGoals = false;
    vector<uint16_t> onLevel = AssetManagement::GetCellDataFromJsonLayer(result, "on_level", &foundGoals);
    if (foundGoals) {
        int firstNonZero = AssetManagement::GetFirstNonZeroCell(&onLevel);
        int goal_offset = Get_Tileset_ID_Offset_From_Tilemap(firstNonZero, result);

        for (int i = 0; i < level->w * level->h; i++) {
            int local_id = onLevel[i] - goal_offset + 1;
            if (local_id > 0) {
                level->goalCount++;
            }
        }

        level->goals = ALLOC_ARRAY(arena, Goal, level->goalCount);
        int index = 0;
        for (int i = 0; i < level->w * level->h; i++) {
            int local_id = onLevel[i] - goal_offset + 1;
            if (local_id < 0) {
                local_id = 0;
            }
            if (local_id != 0) {
                int x;
                int y;
                Expand1DTo2D(i, level->w, &x, &y);
                level->goals[index].x = x;
                level->goals[index].y = y;
                index++;
            }
        }
    }
}

void CreateEntities( LevelData* lvl_data, Arena* arena ) {
    Reset(arena);
    lvl_data->entityCount = 0;
    lvl_data->entityBuffer = ALLOC_ARRAY(arena, Entity, MAX_NUM_ENTITIES);

    ifstream stream(lvl_data->level_path);
    Json::CharReaderBuilder reader;
    Json::Value result;
    if (!Json::parseFromStream(reader, stream, &result, nullptr)) {
        return;
    }
    bool found = false;
    vector<uint16_t> entities = AssetManagement::GetCellDataFromJsonLayer(result, "entities", &found);
    if (!found)
        return;

    for (int i = 0; i < lvl_data->w * lvl_data->h; i++) {
        int raw = entities[i];
        if (raw == 0) {
            continue;
        }
        uint16_t entity_id = GetLocalTileID(raw, result);
        int x;
        int y;
        Expand1DTo2D(i, lvl_data->w, &x, &y);
        AddEntity((ENTITY_ID) entity_id, x, y, lvl_data);
    }
}

Entity* GetNextAvailableEntity( LevelData* level ) {
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

void AddEntity( ENTITY_ID entity_id, int x, int y, LevelData* level ) {
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
    entity->action = Actions::NONE;
    InitializeBaseBehaviour(entity);
}

void RemoveEntity( int x, int y, LevelData* level ) {
    Entity* entity = GetEntity(level, x, y);
    if (entity == nullptr) {
        return;
    }
    entity->active = false;
}

Entity* RaycastFirstEntity( int x_origin, int y_origin, Direction direction, LevelData* level, bool ignore_walls ) {
    Position facingVector{};
    switch (direction) {
        case Direction::RIGHT:
            facingVector = {1, 0};
            break;
        case Direction::LEFT:
            facingVector = {-1, 0};
            break;
        case Direction::UP:
            facingVector = {0, -1};
            break;
        case Direction::DOWN:
            facingVector = {0, 1};
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

namespace AssetManagement {
    std::vector<uint16_t>
    GetCellDataFromJsonLayer( const Json::Value& parsedJson, const char* layerName, bool* wasFound ) {
        std::vector<uint16_t> result;
        *wasFound = false;
        for (const Json::Value& layer: parsedJson["layers"]) {
            if (layer["name"].asString() == layerName) {
                for (const Json::Value& value: layer["data"]) {
                    result.push_back((uint16_t) value.asUInt());
                }
                *wasFound = true;
                break;
            }
        }
        return result;
    }

    int GetFirstNonZeroCell( std::vector<uint16_t>* list ) {
        for (uint16_t id: *list) {
            if (id != 0) {
                return id;
            }
        }
        assert(false);
        return -1;
    }
}
