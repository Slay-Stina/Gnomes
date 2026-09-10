#pragma once
#include "arena.h"
#include "gameState.h"
#include <SDL3/SDL_render.h>

void StoreGameState(Arena* arena);

void RetrieveGameState(Arena* arena);

bool TryMove(Entity* mover, LevelData* level, CommandBuffer* cmd_buffer, int xDir, int yDir, int timestamp,
             int strength);

extern "C" {
void Initialize(GameData* data, SDL_Window* window, SDL_Renderer* renderer);

bool HandleEvents(Arena* arena, SDL_Event& event);

void Draw(GameData* data, SDL_Renderer* renderer);

void Update(GameData* data, float dt);

void OnQuit(SDL_Renderer* renderer);
}
