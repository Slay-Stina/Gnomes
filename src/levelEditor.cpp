#include "levelEditor.h"
#include "levels.h"

#include <imgui.h>

#include "command.h"
#include "rendering.h"

namespace EDITOR {
    void DrawObjectPanel(Editor* editor, SpriteLibrary& sprites) {
        ImGui::Begin("objects");
        ImVec2 size = {32, 32};
        if (ImGui::ImageButton("Rock", sprites.Get(ENTITY_ID::ROCK)->texture, size)) {
            editor->object_to_place_id = ENTITY_ID::ROCK;
            editor->has_selection = true;
        }
        ImGui::SameLine();
        if (ImGui::ImageButton("Demon", sprites.Get(ENTITY_ID::DEMON)->texture, size)) {
            editor->object_to_place_id = ENTITY_ID::DEMON;
            editor->has_selection = true;
        }
        ImGui::SameLine();
        if (ImGui::ImageButton("Golem", sprites.Get(ENTITY_ID::GOLEM)->texture, size)) {
            editor->object_to_place_id = ENTITY_ID::GOLEM;
            editor->has_selection = true;
        }
        ImGui::SameLine();
        if (ImGui::ImageButton("Medusa", sprites.Get(ENTITY_ID::MEDUSA)->texture, size)) {
            editor->object_to_place_id = ENTITY_ID::MEDUSA;
            editor->has_selection = true;
        }
        ImGui::End();
    }

    void PlaceObject(const int x, const int y, Editor* editor, LevelData* level, CommandBuffer* buffer) {
        if (!editor->has_selection) {
            return;
        }
        AddCommand add(x, y, editor->object_to_place_id);
        Push(buffer, add, level);
    }

    void DrawPreview(Editor* editor, Input* input, SDL_Renderer* renderer, LevelData* level, Camera* camera,
                     SpriteLibrary& sprites) {
        int x;
        int y;
        camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
        Sprite* preview = sprites.Get(editor->object_to_place_id);
        if (preview == nullptr || !editor->has_selection) {
            return;
        }
        RenderEntity_OnTile(preview, level, renderer, camera, x, y, 1, 0.5);
    }

    void Update(Editor* editor, Input* input, LevelData* level, CommandBuffer* buffer) {
        if (ImGui::GetIO().WantCaptureMouse) {
            return;
        }
        if (MousePressed(input, MouseButtons::LEFT)) {
            if (camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level)) {
                int x;
                int y;
                camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
                PlaceObject(x, y, editor, level, buffer);
            }
        } else if (MousePressed(input, MouseButtons::RIGHT)) {
            if (camera::GetIsPointInsideGrid(input->mouse_x, input->mouse_y, level)) {
                int x;
                int y;
                camera::WorldToGrid(input->mouse_x, input->mouse_y, &x, &y, level);
                Entity* entity = GetEntity(level, x, y);
                if (entity == nullptr) {
                    return;
                }
                RemoveCommand remove(entity);
                Push(buffer, remove, level);
            }
        }
    }
}
