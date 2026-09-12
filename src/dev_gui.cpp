#include "dev_gui.h"
#include "gameState.h"
#include "command.h"
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <string>

using namespace std;

void Draw_Imgui_Arena_Usage(Arena* arena, std::string name_of_arena) {
    float fraction = (float) arena->used / (float) arena->size;
    string barText = name_of_arena;
    barText += " " + to_string(arena->used);
    barText += " / " + to_string(arena->size);
    ImGui::ProgressBar(fraction, ImVec2(-1, 0), barText.c_str());
}

void Draw_History(CommandBuffer* buffer, LevelData* level) {
    int sliderPos = buffer->index;
    if (ImGui::SliderInt("history", &sliderPos, 0, buffer->head)) {
        while (buffer->index > sliderPos) {
            Undo(buffer, level);
        }
        while (buffer->index < sliderPos) {
            Redo(buffer, level);
        }
    }
}

void DrawFPS(float dt) {
    ImGui::Text("FPS: %0.f", 1 / dt);
}

void DEV::Initialize(SDL_Window* window, SDL_Renderer* renderer) {
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    ImGuiIO& io = ImGui::GetIO();
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    io.DisplaySize = ImVec2((float) w, (float) h);
}

void DEV::ProcessEvents(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void DEV::PreDraw(ImGuiContext* saved_context) {
    if (ImGui::GetCurrentContext() == nullptr) {
        ImGui::SetCurrentContext(saved_context);
    }
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();
}

void DEV::Draw(GameData* data, SDL_Renderer* renderer) {
    ImGui::Begin("Dev Tools");
    ImGui::Text("memory arena usage");
    Draw_Imgui_Arena_Usage(data->arena_images, "images");
    Draw_Imgui_Arena_Usage(data->arena_levels, "levels");
    Draw_Imgui_Arena_Usage(data->arena_commands, "commands");
    Draw_Imgui_Arena_Usage(data->arena_entities, "entities");
    Draw_History(data->commandBuffer, data->GetCurrentLevel());
    DrawFPS(*data->dt);
    ImGui::End();
    if (data->edit_level) {
        EDITOR::DrawObjectPanel(&data->editorData, data->spriteBuffer);
        EDITOR::DrawPreview(&data->editorData, &data->input, renderer, data->GetCurrentLevel(), &data->camera,
                            data->spriteBuffer);
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
