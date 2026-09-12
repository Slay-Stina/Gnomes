#pragma once

#include "spriteLibrary.h"
#include "input.h"
#include "camera.h"
#include "command.h"
#include <imgui.h>

#include "levelEditor.h"

struct Gameplay {
    CommandBuffer* commandBuffer;
    LevelData* levels;
    int levelCount;
    int currentLevelIndex;
    Position* input_buffer;
    int input_buffer_capacity;
    int input_buffer_write_count;
    int input_buffer_read_count;
    bool initialized;
};

struct MainMenu {
};

struct TitleScreen {
};

struct Credits {
};

struct EditorData {
    Editor editor;
    bool edit_level;
};

struct Transition {
    enum States {
        Inactive,
        FadeTo,
        FadeFrom
    };

    States state = Inactive;
    float fade_time_elapsed = 0;
    float fade_time_duration = 1;
};

struct Scenes {
    Gameplay gameplay;
    MainMenu mainMenu;
    TitleScreen titlescreen;
    Credits credts;
};

enum class SCENE_TYPES : uint8_t {
    NONE,
    TITLESCREEN,
    MAINMENU,
    GAME,
    CREDITS,
};

struct GameData {
    const float* dt;
    SCENE_TYPES scene_current;
    SCENE_TYPES scene_previous;
    Scenes scenes;
    Transition transition;
    EditorData editor_data;
    Input input;
    SpriteLibrary sprites;
    Tileset* tilesetBuffer;
    Arena* arena_main;
    Arena* arena_levels;
    Arena* arena_entities;
    Arena* arena_images;
    Arena* arena_commands;
    Arena* arena_input;
    Arena* arena_scratch;
    Camera camera;
    ImGuiContext* imGui_context;
};

inline LevelData* GetCurrentLevel(Gameplay* game) {
    return &game->levels[game->currentLevelIndex];
}
