#include "game.h"

#include <fstream>

#include "common.h"
#include "dev_gui.h"
#include "levelRenderer.h"

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
    ID stepInto_tile_id = (ID) GetCellID(level, test_x, test_y);

    if (stepInto_entity == nullptr) {
        if (stepInto_tile_id == ID::GROUND) {
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

extern "C" {
void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer) {
    DEV::Initialize(window, renderer);
    data->imGui_context = ImGui::GetCurrentContext();
    AssetManagement::LoadAllSprites(data->spriteBuffer, renderer);
    data->currentLevel = 1;
    CreateLevel(data->arena_levels, &data->levels[0], "assets/levels/testLevel.tmj");
    CreateLevel(data->arena_levels, &data->levels[1], "assets/levels/testLevel_box.tmj");
    CreateEntities(&data->levels[data->currentLevel], data->arena_entities);
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
    }
    if (event.key.key == SDLK_ESCAPE) {
        return false;
    }
    if (event.type != SDL_EVENT_KEY_DOWN) {
        return true;
    }
    return true;
}

void Update(GameData* data, float dt) {
    //Check input to enter edit mode
    if (KeyPressed(&data->input, SDL_SCANCODE_F2)) {
        data->edit_level = !data->edit_level;
    }

    //Update level editor
    if (data->edit_level) {
        EDITOR::Update(&data->editorData, &data->input, data->GetCurrentLevel());
    }

    //Check input to undo/redo
    if (KeyPressed(&data->input, SDL_SCANCODE_Z) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_Z, UNDO_REPEAT_TIME)) {
        ResetKeyHeldTime(&data->input, SDL_SCANCODE_Z);
        if (KeyHeld(&data->input, SDL_SCANCODE_LSHIFT)) {
            Redo(data->commandBuffer, data->GetCurrentLevel());
        } else {
            Undo(data->commandBuffer);
        }
    }

    //Check input for movement
    if (KeyPressed(&data->input, SDL_SCANCODE_RIGHT) || KeyHeld_ForTime(&data->input, SDL_SCANCODE_RIGHT,
                                                                        (1 / MOVE_SPEED) * 1.15)) {
        ResetKeyHeldTime(&data->input, SDL_SCANCODE_RIGHT);
        data->input_buffer[data->input_buffer_write_count++ % data->input_buffer_capacity] = {1, 0};
    } else if (KeyPressed(&data->input, SDL_SCANCODE_LEFT) || KeyHeld_ForTime(
                       &data->input, SDL_SCANCODE_LEFT, (1 / MOVE_SPEED) * 1.15)) {
        ResetKeyHeldTime(&data->input, SDL_SCANCODE_LEFT);
        data->input_buffer[data->input_buffer_write_count++ % data->input_buffer_capacity] = {-1, 0};
    } else if (KeyPressed(&data->input, SDL_SCANCODE_UP) || KeyHeld_ForTime(
                       &data->input, SDL_SCANCODE_UP, (1 / MOVE_SPEED) * 1.15)) {
        ResetKeyHeldTime(&data->input, SDL_SCANCODE_UP);
        data->input_buffer[data->input_buffer_write_count++ % data->input_buffer_capacity] = {0, -1};
    } else if (KeyPressed(&data->input, SDL_SCANCODE_DOWN) || KeyHeld_ForTime(
                       &data->input, SDL_SCANCODE_DOWN, (1 / MOVE_SPEED) * 1.15)) {
        ResetKeyHeldTime(&data->input, SDL_SCANCODE_DOWN);
        data->input_buffer[data->input_buffer_write_count++ % data->input_buffer_capacity] = {0, 1};
    }

    //Update movable entities
    bool are_entities_moving = false;
    for (int i = 0; i < data->GetCurrentLevel()->entityCount; i++) {
        Entity* entity = &data->GetCurrentLevel()->entityBuffer[i];
        if (HasBehaviour(entity, CAN_MOVE) && IsMoving(entity)) {
            entity->progress_01 += MOVE_SPEED * dt;
            if (entity->progress_01 >= 1) {
                entity->progress_01 = 0;
                entity->x_prev = entity->x;
                entity->y_prev = entity->y;
            }
            if (IsMoving(entity)) {
                are_entities_moving = true;
            }
        }
    }

    //Update player movement
    if (are_entities_moving == false && data->input_buffer_read_count < data->input_buffer_write_count) {

        for (int i = 0; i < data->GetCurrentLevel()->entityCount; i++) {
            Entity* entity = &data->GetCurrentLevel()->entityBuffer[i];
            if (HasBehaviour(entity, IS_PUSHING)) {
                RemoveBehaviour(entity, IS_PUSHING);
            }
            
            if (HasBehaviour(entity, (Behaviour) (RESPOND_TO_INPUT | CAN_MOVE))) {
                if (HasBehaviour(entity, IS_PETRIFIED)) {
                    continue;
                }
                int xDir = data->input_buffer[data->input_buffer_read_count % data->input_buffer_capacity].x;
                int yDir = data->input_buffer[data->input_buffer_read_count % data->input_buffer_capacity].y;
                Direction new_facing = DirectionFromXY(xDir, yDir);
                if (new_facing != entity->facing) {
                    RotateCommand rotate(entity, entity->facing, new_facing);
                    Push(data->commandBuffer, rotate, data->GetCurrentLevel());
                }
                TryMove(entity, data->GetCurrentLevel(), data->commandBuffer, xDir, yDir, entity->strength);
            }
        }
        data->input_buffer_read_count++;
    }
    data->commandBuffer->timestamp++;
}

void Draw(GameData* data, SDL_Renderer* renderer) {
    DEV::PreDraw(data->imGui_context);

    SDL_SetRenderDrawColor(renderer, 143, 86, 59, 255);
    SDL_RenderClear(renderer);

    RenderLevel(data, renderer);
    RenderEntities(data, renderer);
    DEV::Draw(data, renderer);

    SDL_RenderPresent(renderer);
}

void OnQuit(SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
}
}
