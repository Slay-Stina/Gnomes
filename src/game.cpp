#include "game.h"

#include <fstream>

#include "common.h"
#include "dev_gui.h"
#include "levelRenderer.h"
#include "levels.h"
#include "rendering.h"

using namespace std;

void StoreGameState(Arena* arena) {
    ofstream file("temp_state.bin", ios::binary);
    file.write(reinterpret_cast<const char*>(arena->base), arena->size);
}

void RetrieveGameState(Arena* arena) {
    ifstream file("temp_state.bin", ios::binary);
    file.read(reinterpret_cast<char*>(arena->base), arena->size);
}

bool TryMove(Entity* mover, LevelData* level, CommandBuffer* cmd_buffer, int xDir, int yDir, int strength) {
    if (strength < 0) {
        return false;
    }
    if (HasBehaviour(mover, CAN_MOVE) == false) {
        return false;
    }
    int test_x = mover->x + xDir;
    int test_y = mover->y + yDir;
    Entity* stepInto_entity = GetEntity(level, test_x, test_y);

    if (stepInto_entity == nullptr) {
        if (IsWalkable(test_x, test_y, level)) {
            MoveCommand mv(mover, xDir, yDir);
            Push(cmd_buffer, mv, level);
            return true;
        }
        return false;
    }
    if (HasBehaviour(stepInto_entity, CAN_MOVE) && !HasBehaviour(stepInto_entity, UNPUSHABLE)) {
        if (TryMove(stepInto_entity, level, cmd_buffer, xDir, yDir, --strength)) {
            MoveCommand mv(mover, xDir, yDir);
            AddBehaviour(mover, IS_PUSHING);
            Push(cmd_buffer, mv, level);
            return true;
        }
    }
    return false;
}

void StartLevel(Gameplay* gameplay, Arena* arena_commands, Arena* arena_entities) {
    Reset(arena_commands);
    CreateEntities(&gameplay->levels[gameplay->currentLevelIndex], arena_entities);
}

void ChangeScene(GameData* data, SCENE_TYPES new_scene) {
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
            StartLevel(gameplay, data->arena_commands, data->arena_entities);
            break;
        }
        case SCENE_TYPES::CREDITS:
            break;
        case SCENE_TYPES::NONE:
            assert(false);
            break;
    }
}

void InitializeGame(Gameplay* gameplay, Arena* arena_levels, Tileset* tilesetBuffer) {
    assert(gameplay->initialized == false);
    gameplay->currentLevelIndex = 0;
    gameplay->activePlayerIndex = 0;
    CreateLevel(arena_levels, &gameplay->levels[0], &tilesetBuffer[(int) TILESETS::Dungeon],
                "assets/levels/testing.tmj");
    gameplay->initialized = true;
}

void UpdateGame(Gameplay* gameplay, Input* input, Arena* arena_scratch, const float dt) {
    LevelData* level = GetCurrentLevel(gameplay);
    Entity* entityBuffer = level->entityBuffer;

    // Check player count and put them in buffer
    int player_count = 0;
    for (int i = 0; i < level->entityCount; i++) {
        if (entityBuffer[i].active == false) {
            continue;
        }
        if (HasBehaviour(&level->entityBuffer[i], IS_PLAYER)) {
            player_count++;
        }
    }
    int index = 0;
    gameplay->activePlayerBuffer = ALLOC_ARRAY(arena_scratch, Entity*, player_count);
    if (player_count == 0) {
        return;
    }
    for (int i = 0; i < level->entityCount; i++) {
        if (entityBuffer[i].active == false) {
            continue;
        }
        if (HasBehaviour(&level->entityBuffer[i], IS_PLAYER)) {
            gameplay->activePlayerBuffer[index++] = &level->entityBuffer[i];
        }
    }
    
    //Check input to undo/redo
    if (KeyPressed(input, SDL_SCANCODE_Z) || KeyHeld_ForTime(input, SDL_SCANCODE_Z, UNDO_REPEAT_TIME)) {
        ResetKeyHeldTime(input, SDL_SCANCODE_Z);
        if (KeyHeld(input, SDL_SCANCODE_LSHIFT)) {
            Redo(gameplay->commandBuffer, GetCurrentLevel(gameplay));
        } else {
            Undo(gameplay->commandBuffer, GetCurrentLevel(gameplay));
        }
    }
    bool are_entities_acting = false;

    for (int i = 0; i < level->entityCount; i++) {
        if (IsActing(&entityBuffer[i])) {
            are_entities_acting = true;
            break;
        }
    }

    if (!are_entities_acting && KeyPressed(input, SDL_SCANCODE_X) && player_count > 0) {
        SwapActiveEntityCommand swap(&gameplay->activePlayerIndex, player_count);
        Push(gameplay->commandBuffer, swap, GetCurrentLevel(gameplay));
        gameplay->commandBuffer->timestamp += 1;
    }

    Entity* entity = GetActiveEntity(gameplay);
    if (!HasBehaviour(entity, (Behaviour) (RESPOND_TO_INPUT | CAN_MOVE))) {
        return;
    }
    if (HasBehaviour(entity, IS_PETRIFIED)) {
        return;
    }

    //Check input for movement
    if (KeyPressed(input, SDL_SCANCODE_RIGHT) || KeyHeld_ForTime(input, SDL_SCANCODE_RIGHT,
                                                                 1 / MOVE_SPEED * 1.15)) {
        ResetKeyHeldTime(input, SDL_SCANCODE_RIGHT);
        gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {1, 0};
    } else if (KeyPressed(input, SDL_SCANCODE_LEFT) || KeyHeld_ForTime(
                       input, SDL_SCANCODE_LEFT, 1 / MOVE_SPEED * 1.15)) {
        ResetKeyHeldTime(input, SDL_SCANCODE_LEFT);
        gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {-1, 0};
    } else if (KeyPressed(input, SDL_SCANCODE_UP) || KeyHeld_ForTime(
                       input, SDL_SCANCODE_UP, 1 / MOVE_SPEED * 1.15)) {
        ResetKeyHeldTime(input, SDL_SCANCODE_UP);
        gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {0, -1};
    } else if (KeyPressed(input, SDL_SCANCODE_DOWN) || KeyHeld_ForTime(
                       input, SDL_SCANCODE_DOWN, 1 / MOVE_SPEED * 1.15)) {
        ResetKeyHeldTime(input, SDL_SCANCODE_DOWN);
        gameplay->input_buffer[gameplay->input_buffer_write_count++ % gameplay->input_buffer_capacity] = {0, 1};
    }

    // Is there an entity moving?
    for (int i = 0; i < level->entityCount; i++) {
        Entity* entity = &entityBuffer[i];
        if (!entity->active)
            continue;
        switch (entity->action) {
            case Actions::NONE:
                continue;
            case Actions::MOVING:
                entity->progress_01 += MOVE_SPEED * dt;
                break;
            case Actions::ROTATING:
                entity->progress_01 += 8 * dt;
                break;
        }
    }
    for (int i = 0; i < level->entityCount; i++) {
        Entity* entity = &entityBuffer[i];
        if (entity->progress_01 >= 1) {
            entity->x_prev = entity->x;
            entity->y_prev = entity->y;
            entity->facing_previous = entity->facing_current;
            entity->action = Actions::NONE;
            entity->progress_01 = 0;
            if (HasBehaviour(entity, IS_PUSHING)) {
                RemoveBehaviour(entity, IS_PUSHING);
            }
        }
    }


    if (are_entities_acting) {
        return;
    }
    if (gameplay->input_buffer_read_count == gameplay->input_buffer_write_count) {
        return;
    }

    int xDir = gameplay->input_buffer[gameplay->input_buffer_read_count % gameplay->input_buffer_capacity].x;
    int yDir = gameplay->input_buffer[gameplay->input_buffer_read_count % gameplay->input_buffer_capacity].y;
    Direction new_facing = DirectionFromXY(xDir, yDir);
    if (new_facing != entity->facing_current) {
        RotateCommand rotate(entity, entity->facing_current, new_facing);
        Push(gameplay->commandBuffer, rotate, level);
        return;
    }
    if (!IsActing(entity)) {
        TryMove(entity, level, gameplay->commandBuffer, xDir, yDir, entity->strength);
        gameplay->commandBuffer->timestamp += 1;
        gameplay->input_buffer_read_count++;
    }
}

void DrawScene(GameData* data, SCENE_TYPES scene, SDL_Renderer* renderer) {
    switch (scene) {
        case SCENE_TYPES::TITLESCREEN: {
            Sprite* background = data->sprites.GetBySpriteID(SPRITE_ID::titlescreen_background);
            Camera screen_camera = {0, 0};
            RenderSprite_World(background, renderer, &screen_camera, 0, 0);
            break;
        }
        case SCENE_TYPES::MAINMENU:
        case SCENE_TYPES::GAME:
            RenderLevel(data, renderer);
            RenderEntities(data, renderer);
            break;
        case SCENE_TYPES::CREDITS:
            break;
        case SCENE_TYPES::NONE:
            assert(false);
            break;
    }
}

extern "C" {
void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer) {
    DEV::Initialize(window, renderer);
    data->sprites.LoadAll(renderer, data->arena_images);
    AssetManagement::LoadAllTilesets(data->tilesetBuffer, data->arena_images);
    data->imGui_context = ImGui::GetCurrentContext();
    SDL_Texture* blackfade = data->sprites.GetBySpriteID(SPRITE_ID::black_1x1)->texture;
    SDL_SetTextureBlendMode(blackfade, SDL_BLENDMODE_BLEND);
    InitializeGame(&data->scenes.gameplay, data->arena_levels, data->tilesetBuffer);
    ChangeScene(data, SCENE_TYPES::TITLESCREEN);
}

bool HandleEvents(Arena* arena, SDL_Event& event) {
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

void Update(GameData* data, float dt) {
    Gameplay* gameplay = &data->scenes.gameplay;
    EditorData* editorData = &data->editor_data;
    Transition* transition = &data->transition;

    // Editor
    if (KeyPressed(&data->input, SDL_SCANCODE_F2)) {
        editorData->edit_level = !editorData->edit_level;
    }
    if (editorData->edit_level) {
        EDITOR::Update(&editorData->editor, &data->input, GetCurrentLevel(gameplay), gameplay->commandBuffer);
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
            UpdateGame(gameplay, &data->input, data->arena_scratch, dt);
            break;
        case SCENE_TYPES::MAINMENU:
        case SCENE_TYPES::CREDITS:
        case SCENE_TYPES::NONE:
            break;
    }
}

void Draw(GameData* data, SDL_Renderer* renderer) {
    DEV::PreDraw(data->imGui_context);
    SDL_SetRenderDrawColor(renderer, 143, 86, 59, 255);
    SDL_RenderClear(renderer);

    Camera screen_camera = {0, 0}; // ingen kamera-offset för overlayet
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

void OnQuit(SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
}
}
