#include "tilesetLibrary.h"

#include "arena.h"

#include <cassert>
#include <fstream>

using namespace std;

uint16_t Get_Tileset_ID_Offset_From_Tilemap(int id_limit, const Json::Value& tmj_result) {
    int highest_tilemap_start_id = 0;
    for (const Json::Value& tileset: tmj_result["tilesets"]) {
        int first_id = tileset["firstgid"].asInt();
        if (first_id <= id_limit && first_id > highest_tilemap_start_id) {
            highest_tilemap_start_id = first_id;
        }
    }
    return highest_tilemap_start_id;
}

uint16_t GetLocalTileID(uint16_t id_global, const Json::Value& tmj_result) {
    return id_global - Get_Tileset_ID_Offset_From_Tilemap(id_global, tmj_result);
}

namespace {
    const TilesetDataEntry ALL_TILESETS_DATA[] = {
            {TILESETS::Dungeon, "assets/tilesets/dungeon_tileset.tsj"}
    };
}

namespace AssetManagement {
    void LoadAllTilesets(Tileset* tilesetBuffer, Memory::Arena* arena_images) {
        for (const TilesetDataEntry& entry: ALL_TILESETS_DATA) {
            LoadTileset(&entry, tilesetBuffer, arena_images);
        }
    }

    void LoadTileset(const TilesetDataEntry* entry, Tileset* tilesetBuffer, Memory::Arena* arena_images) {
        assert(entry->type != TILESETS::COUNT);
        assert(entry->type != TILESETS::NONE);

        Tileset* tileset = &tilesetBuffer[(int) entry->type];
        tileset->type = entry->type;

        ifstream stream(entry->path);
        Json::CharReaderBuilder reader;
        Json::Value jsonResult;
        if (!Json::parseFromStream(reader, stream, &jsonResult, nullptr)) {
            return;
        }

        int tile_count = jsonResult["tilecount"].asInt();
        tileset->walkableBuffer = ALLOC_ARRAY(arena_images, bool, tile_count);

        const Json::Value& tiles = jsonResult["tiles"];
        for (const Json::Value& tile: tiles) {
            int tile_id = tile["id"].asInt();
            const Json::Value& properties = tile["properties"];
            for (const Json::Value& property: properties) {
                if (property["name"].asString() == "walkable") {
                    tileset->walkableBuffer[tile_id] = property["value"].asBool();
                }
            }
        }
    }
}
