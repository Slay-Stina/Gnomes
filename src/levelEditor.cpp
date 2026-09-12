#include "levelEditor.h"
#include "levels.h"

#include <imgui.h>

#include "command.h"
#include "rendering.h"

namespace {
    bool SpriteButton(const char* id, SpriteRenderInfo info, ImVec2 size) {
        Sprite* sprite = info.sprite;
        int count_x = sprite->sprite_count_x == NOT_SET ? 1 : sprite->sprite_count_x;
        int count_y = sprite->sprite_count_y == NOT_SET ? 1 : sprite->sprite_count_y;
        ImVec2 uv0 = ImVec2((float) (info.frame % count_x) / count_x,
                            (float) (info.frame / count_x) / count_y);
        ImVec2 uv1 = ImVec2(uv0.x + 1.0f / count_x, uv0.y + 1.0f / count_y);
        return ImGui::ImageButton(id, sprite->texture, size, uv0, uv1);
    }
}

namespace EDITOR {
    void DrawObjectPanel(Editor* editor, SpriteLibrary& sprites) {
        ImGui::Begin("objects");
        ImVec2 size = {32, 32};
        if (SpriteButton("Rock", sprites.Get(ENTITY_ID::ROCK), size)) {
            editor->object_to_place_id = ENTITY_ID::ROCK;
            editor->has_selection = true;
        }
        ImGui::SameLine();
        if (SpriteButton("Gnome", sprites.Get(ENTITY_ID::GNOME), size)) {
            editor->object_to_place_id = ENTITY_ID::GNOME;
            editor->has_selection = true;
        }
        ImGui::SameLine();
        if (SpriteButton("Golem", sprites.Get(ENTITY_ID::GOLEM), size)) {
            editor->object_to_place_id = ENTITY_ID::GOLEM;
            editor->has_selection = true;
        }
        ImGui::SameLine();
        if (SpriteButton("Medusa", sprites.Get(ENTITY_ID::MEDUSA), size)) {
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
        SpriteRenderInfo preview = sprites.Get(editor->object_to_place_id);
        if (preview.sprite == nullptr || !editor->has_selection) {
            return;
        }
        RenderSprite_OnTile(preview, level, renderer, camera, x, y, 1, 0.5);
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
