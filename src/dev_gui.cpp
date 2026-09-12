#include "dev_gui.h"
#include "gameState.h"
#include "command.h"
#include "common.h"
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <cstdio>
#include <string>

using namespace std;

namespace {
    string FormatBytes(size_t bytes) {
        char buf[32];
        if (bytes >= GIGABYTES(1)) {
            snprintf(buf, sizeof(buf), "%.1f GB", AS_GIGABYTES(bytes));
        } else if (bytes >= MEGABYTES(1)) {
            snprintf(buf, sizeof(buf), "%.1f MB", AS_MEGABYTES(bytes));
        } else if (bytes >= KILOBYTES(1)) {
            snprintf(buf, sizeof(buf), "%.1f KB", AS_KILOBYTES(bytes));
        } else {
            snprintf(buf, sizeof(buf), "%zu B", bytes);
        }
        return buf;
    }
}

void Draw_Imgui_Arena_Usage(Arena* arena, const std::string& name_of_arena) {
    float fraction = (float) arena->used / (float) arena->size;
    string barText = name_of_arena + " " + FormatBytes(arena->used) + " / " + FormatBytes(arena->size);
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

void DrawFPS(GameData* data) {
    EditorData* editor = &data->editor_data;
    float fps = 1.0 / *data->dt * *data->dt_scaler;
    editor->fps_buffer[editor->fps_buffer_index++] = fps;
    editor->fps_buffer_index %= FPS_BUFFER_COUNT;
    ImVec4 color = fps >= 55
                       ? ImVec4(0.3f, 1, 0.3f, 1)
                       : fps >= 30
                       ? ImVec4(1, 0.8f, 0.2f, 1)
                       : ImVec4(1, 0.3f, 0.3f, 1);
    ImGui::TextColored(color, "FPS: %.0f", fps);
    ImGui::PlotLines("##fps", editor->fps_buffer, FPS_BUFFER_COUNT, 0, nullptr, 0, TARGET_FPS, ImVec2(-1, 35));
    float ms = 1000.0f / fps;
    ImGui::Text("%.1f ms (%.0f fps)", ms, fps);
    ImGui::ProgressBar(ms / 33.3f, ImVec2(-1, 0)); // 0=bra, 1=33ms=30fps
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
    Gameplay* gameplay = &data->scenes.gameplay;

    if (data->editor_data.show_dev) {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.8f);
        ImGui::Begin("Dev Tools");

        if (ImGui::BeginTabBar("dev_tabs")) {
            if (ImGui::BeginTabItem("perf")) {
                DrawFPS(data);
                ImGui::SliderFloat("deltaTimeScaler", data->dt_scaler, 0.1, 3);
                Draw_History(gameplay->commandBuffer, GetCurrentLevel(gameplay));
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("arenas")) {
                ImGui::Text("memory arena usage");
                Draw_Imgui_Arena_Usage(data->arena_main, "total");
                Draw_Imgui_Arena_Usage(data->arena_images, "images");
                Draw_Imgui_Arena_Usage(data->arena_levels, "levels");
                Draw_Imgui_Arena_Usage(data->arena_commands, "commands");
                Draw_Imgui_Arena_Usage(data->arena_entities, "entities");
                Draw_Imgui_Arena_Usage(data->arena_input, "input");
                Draw_Imgui_Arena_Usage(data->arena_scratch, "scratch");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    if (data->editor_data.edit_level) {
        EDITOR::DrawObjectPanel(&data->editor_data.editor, data->sprites);
        EDITOR::DrawPreview(&data->editor_data.editor, &data->input, renderer,
                            GetCurrentLevel(gameplay), &data->camera, data->sprites);
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
