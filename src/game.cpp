#include "game.h"

#include <SDL3/SDL_scancode.h>

#include "command.h"
#include "common.h"
#include "levelRenderer.h"
#include "levels.h"

namespace {
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
}

void Game::Initialize(Gameplay* gameplay, Arena* arena_levels, Tileset* tilesetBuffer) {
    assert(gameplay->initialized == false);
    gameplay->currentLevelIndex = 0;
    gameplay->activePlayerIndex = 0;
    CreateLevel(arena_levels, &gameplay->levels[0], &tilesetBuffer[(int) TILESETS::Dungeon],
                "assets/levels/Level_01.tmj");
    CreateLevel(arena_levels, &gameplay->levels[1], &tilesetBuffer[(int) TILESETS::Dungeon],
                "assets/levels/Level_02.tmj");
    gameplay->initialized = true;
}

void Game::Update(Gameplay* gameplay, Input* input, Arena* arena_scratch, Arena* arena_commands, Arena* arena_entities,
                  float dt) {
    if (KeyPressed(input, SDL_SCANCODE_R)) {
        StartLevel(gameplay, arena_commands, arena_entities);
        return;
    }
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
    gameplay->activePlayerBuffer = ALLOC_ARRAY(arena_scratch, Entity *, player_count);
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

    for (int i = 0; i < level->goalCount; i++) {
        Entity* entity = GetEntity(level, level->goals[i].x, level->goals[i].y);
        if (entity != nullptr && !IsActing(entity)) {
            level->goals[i].blink_timer += dt;
        } else {
            level->goals[i].blink_timer = 0;
        }
    }

    if (level->goalCount > 0) {
        int goals_reached = 0;
        for (int i = 0; i < level->goalCount; i++) {
            Goal goal = level->goals[i];
            Entity* entity = GetEntity(level, goal.x, goal.y);
            if (entity == nullptr) {
                break;
            }
            if (!IsActing(entity) && HasBehaviour(entity, IS_PLAYER)) {
                goals_reached++;
            }
        }

        if (goals_reached == level->goalCount) {
            gameplay->level_complete_timer += dt;
            if (gameplay->level_complete_timer >= LEVEL_COMPLETE_DELAY) {
                gameplay->currentLevelIndex++;
                StartLevel(gameplay, arena_commands, arena_entities);
                return;
            }
        } else {
            gameplay->level_complete_timer = 0;
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
        bool moved = TryMove(entity, level, gameplay->commandBuffer, xDir, yDir, entity->strength);
        if (moved) {
            PlaySFX(SFX_ID::JUMP);
        }
        gameplay->commandBuffer->timestamp += 1;
        gameplay->input_buffer_read_count++;
    }
}

void Game::Draw(GameData* data, SDL_Renderer* renderer) {
    RenderLevel(data, renderer);
    RenderEntities(data, renderer);
}

void Game::StartLevel(Gameplay* gameplay, Arena* arena_commands, Arena* arena_entities) {
    ResetCommandBuffer(gameplay->commandBuffer);
    Reset(arena_commands);
    CreateEntities(&gameplay->levels[gameplay->currentLevelIndex], arena_entities);
    gameplay->activePlayerIndex = 0;
    gameplay->level_complete_timer = 0;
}
