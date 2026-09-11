#pragma once

#include "spriteLibrary.h"
#include "input.h"
#include "camera.h"
#include "command.h"
#include <imgui.h>

#include "levelEditor.h"

struct GameData {
    bool edit_level;
    int levelCount;
    int currentLevel;
    int input_buffer_capacity;
    int input_buffer_write_count;
    int input_buffer_read_count;
    const float* dt;

    Editor editorData;
    Sprite* spriteBuffer;
    Arena* arena_scratch;
    Arena* arena_entities;
    Arena* arena_commands;
    Arena* arena_levels;
    Arena* arena_images;
    Arena* arena_input;
    Input input;
    Camera camera;
    LevelData* levels;
    CommandBuffer* commandBuffer;
    ImGuiContext* imGui_context;
    Position* input_buffer;

    LevelData* GetCurrentLevel() {
        return &levels[currentLevel];
    };
};
