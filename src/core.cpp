#include "core.h"

#include <fstream>

#include "common.h"
#include "dev_gui.h"
#include "levels.h"
#include "rendering.h"

using namespace std;

void StoreGameState( Arena* arena ) {
    std::ofstream file("temp_state.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(arena->base), arena->size);
}

void RetrieveGameState( Arena* arena ) {
    std::ifstream file("temp_state.bin", std::ios::binary);
    file.read(reinterpret_cast<char*>(arena->base), arena->size);
}

void ChangeScene( GameData* data, SCENE_TYPES new_scene ) {
    assert(new_scene != data->scene_current);
    data->scene_previous = data->scene_current;
    data->scene_current = new_scene;
    data->transition.state = data->scene_previous == SCENE_TYPES::NONE
                                 ? Transition::FadeFrom
                                 : Transition::FadeTo;
    data->transition.fade_time_elapsed = 0;
    switch (data->scene_current) {
        case SCENE_TYPES::TITLESCREEN:
            data->transition.fade_time_duration = 1;
            break;
        case SCENE_TYPES::MAINMENU:
            break;
        case SCENE_TYPES::GAME: {
            data->transition.fade_time_duration = 0.5f;
            Gameplay* gameplay = &data->scenes.gameplay;
            assert(gameplay->initialized);
            Game::StartLevel(gameplay, data->arena_commands, data->arena_entities);
            break;
        }
        case SCENE_TYPES::CREDITS:
            break;
        case SCENE_TYPES::NONE:
            assert(false);
            break;
    }
}

void DrawScene( GameData* data, SCENE_TYPES scene, SDL_Renderer* renderer ) {
    switch (scene) {
        case SCENE_TYPES::TITLESCREEN: {
            Sprite* background = data->sprites.GetBySpriteID(SPRITE_ID::titlescreen_background);
            Camera screen_camera = {0, 0, 1};
            RenderSprite_World(background, renderer, &screen_camera, 0, 0);
            break;
        }
        case SCENE_TYPES::MAINMENU:
            Menu::Draw(&data->scenes.mainMenu, renderer, &data->sprites, &data->input);
            break;
        case SCENE_TYPES::GAME:
            Game::Draw(data, renderer);
            break;
        case SCENE_TYPES::CREDITS:
            break;
        case SCENE_TYPES::NONE:
            assert(false);
            break;
    }
}

extern "C" {
void Initialize( GameData* data, SDL_Window* window, SDL_Renderer* renderer ) {
    *data->ticks_total = 0;
    data->camera.camera_z = 1.0f;
    DEV::Initialize(window, renderer);
    Audio::Initialize(&data->audio, data->arena_main);
    AssetManagement::LoadAllSFX(&data->audio);
    data->sprites.LoadAll(renderer, data->arena_images);
    AssetManagement::LoadAllTilesets(data->tilesetBuffer, data->arena_images);
    data->imGui_context = ImGui::GetCurrentContext();
    SDL_Texture* blackfade = data->sprites.GetBySpriteID(SPRITE_ID::black_1x1)->texture;
    SDL_SetTextureBlendMode(blackfade, SDL_BLENDMODE_BLEND);
    Game::Initialize(&data->scenes.gameplay, data->arena_levels, data->tilesetBuffer);
    Menu::Initialize(&data->scenes.mainMenu, &data->sprites, data->arena_main);
    PlaySong(SONG_ID::THEME);
    ChangeScene(data, SCENE_TYPES::MAINMENU);
}

bool HandleEvents( Arena* arena, SDL_Event& event ) {
    DEV::ProcessEvents(&event);
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_S) {
            StoreGameState(arena);
        }
        if (event.key.key == SDLK_L) {
            RetrieveGameState(arena);
        }
        if (event.key.key == SDLK_ESCAPE) {
            return false;
        }
    }
    return true;
}

void Update( GameData* data, float dt ) {
    *data->ticks_total += 1;
    Audio::Update(&data->audio);
    Gameplay* gameplay = &data->scenes.gameplay;
    EditorData* editorData = &data->editor_data;
    Transition* transition = &data->transition;

    // Editor
    if (KeyPressed(&data->input, SDL_SCANCODE_F2)) {
        editorData->edit_level = !editorData->edit_level;
    }
    if (editorData->edit_level) {
        EDITOR::Update(&editorData->editor, &data->input, GetCurrentLevel(gameplay), gameplay->commandBuffer,
                       &data->camera);
    }

    // Dev
    if (KeyPressed(&data->input, SDL_SCANCODE_F1)) {
        editorData->show_dev = !editorData->show_dev;
    }

    // Scene Transition
    if (transition->state != Transition::Inactive) {
        transition->fade_time_elapsed += dt;
        if (transition->fade_time_elapsed >= transition->fade_time_duration) {
            transition->fade_time_elapsed = 0;
            switch (transition->state) {
                case Transition::Inactive:
                    break;
                case Transition::FadeTo:
                    transition->state = Transition::FadeFrom;
                    break;
                case Transition::FadeFrom:
                    transition->state = Transition::Inactive;
                    break;
            }
        }
    }

    // Update the right scene
    switch (data->scene_current) {
        case SCENE_TYPES::TITLESCREEN:
            if (AnyKeyPressed(&data->input)) {
                if (transition->state == Transition::FadeTo || transition->state == Transition::Inactive) {
                    ChangeScene(data, SCENE_TYPES::GAME);
                }
            }
            break;
        case SCENE_TYPES::GAME:
            Game::Update(gameplay, &data->input, data->arena_scratch, data->arena_commands, data->arena_entities, dt);;
            break;
        case SCENE_TYPES::MAINMENU:
            Menu::Update(data);
            break;
        case SCENE_TYPES::CREDITS:
        case SCENE_TYPES::NONE:
            break;
    }
}

void Draw( GameData* data, SDL_Renderer* renderer ) {
    DEV::PreDraw(data->imGui_context);
    SDL_SetRenderDrawColor(renderer, 143, 86, 59, 255);
    SDL_RenderClear(renderer);

    Camera screen_camera = {0, 0, 1}; // ingen kamera-offset för overlayet
    switch (data->transition.state) {
        case Transition::Inactive:
            DrawScene(data, data->scene_current, renderer);
            break;
        case Transition::FadeTo: {
            DrawScene(data, data->scene_previous, renderer);
            float alpha = data->transition.fade_time_elapsed / data->transition.fade_time_duration;
            RenderSprite_World(data->sprites.GetBySpriteID(SPRITE_ID::black_1x1), renderer,
                               &screen_camera, 0, 0, SCREEN_WIDTH, alpha);
            break;
        }
        case Transition::FadeFrom: {
            DrawScene(data, data->scene_current, renderer);
            float alpha = 1 - data->transition.fade_time_elapsed / data->transition.fade_time_duration;
            RenderSprite_World(data->sprites.GetBySpriteID(SPRITE_ID::black_1x1), renderer,
                               &screen_camera, 0, 0, SCREEN_WIDTH, alpha);
            break;
        }
    }
    DEV::Draw(data, renderer);
    SDL_RenderPresent(renderer);
}

void OnQuit( SDL_Renderer* renderer ) {
    SDL_DestroyRenderer(renderer);
}
}
