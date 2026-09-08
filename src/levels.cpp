#include "levels.h"

#include "arena.h"
#include "entity.h"

#include <cstdint>
#include <fstream>
#include <json/json.h>

using namespace std;

const int LEVEL_INDEX = 0;
const int ENTITIES_INDEX = 1;

void CreateLevel(Arena *arena, LevelData *level, const char *level_name) {
    ifstream stream(level_name);
    Json::CharReaderBuilder reader;
    Json::Value jsonResult;
    if (!Json::parseFromStream(reader, stream, &jsonResult, nullptr)) {
        return;
    }

    const Json::Value &dataField = jsonResult["layers"][LEVEL_INDEX]["data"];
    level->w = jsonResult["width"].asInt();
    level->h = jsonResult["height"].asInt();
    level->level_path = level_name;

    size_t size_of_cells = sizeof(uint8_t) * level->w * level->h;
    level->cells = (uint8_t *) Memory::Allocate(arena, size_of_cells);
    for (int i = 0; i < level->w * level->h; i++) {
        level->cells[i] = dataField[i].asUInt();
    }
}

void CreateEntities(LevelData *lvl_data, Arena *arena) {
    Memory::Reset(arena);
    lvl_data->entityCount = 0;

    ifstream stream(lvl_data->level_path);
    Json::CharReaderBuilder reader;
    Json::Value result;
    if (!Json::parseFromStream(reader, stream, &result, nullptr)) {
        return;
    }

    const Json::Value &entityData = result["layers"][ENTITIES_INDEX]["data"];
    for (int i = 0; i < lvl_data->w * lvl_data->h; i++) {
        unsigned char entity_id = entityData[i].asUInt();
        if (entity_id != 0) {
            lvl_data->entityCount++;
        }
    }

    lvl_data->entityBuffer = (Entity *) Memory::Allocate(arena, sizeof(Entity) * lvl_data->entityCount);
    int index = 0;
    for (int i = 0; i < lvl_data->w * lvl_data->h; i++) {
        unsigned char entity_id = entityData[i].asUInt();
        if (entity_id != 0) {
            int x = i % lvl_data->w;
            int y = i / lvl_data->w;

            lvl_data->entityBuffer[index].id = (ID) entity_id;
            lvl_data->entityBuffer[index].InitializeBaseBehaviour();
            lvl_data->entityBuffer[index].x = x;
            lvl_data->entityBuffer[index].y = y;
            lvl_data->entityBuffer[index].x_prev = x;
            lvl_data->entityBuffer[index].y_prev = y;
            index += 1;
        }
    }
}
