#include "arena.h"
#include "common.h"
#include "gameState.h"

#include <SDL3/SDL_log.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

SDL_Window* window;
SDL_Renderer* renderer;
Uint64 NOW;
Uint64 PREV;
constexpr const char* NAME_OF_LIB = "./libGnomes_game.so";
constexpr const char* NAME_OF_TEMP_LIB = "./libGnomes_game_temp.so";
static int load_counter = 0;

typedef void (*Function_Initialize)(GameData* data, SDL_Window* window, SDL_Renderer* renderer);

typedef bool (*Function_HandleEvents)(Arena* arena, SDL_Event event);

typedef void (*Function_Update)(GameData* data, float dt);

typedef void (*Function_Draw)(GameData* data, SDL_Renderer* renderer);

typedef void (*Function_OnQuit)(SDL_Renderer* renderer);

struct DLL_INFO {
    void* handle;
    time_t Timestamp;
    Function_Initialize Initialize;
    Function_HandleEvents HandleEvents;
    Function_Update Update;
    Function_Draw Draw;
    Function_OnQuit OnQuit;
};

time_t GetTimestamp() {
    struct stat s;
    if (stat(NAME_OF_LIB, &s) == 0) {
        return s.st_mtime;
    }
    return 0;
}

bool LoadDLL(DLL_INFO* info, int depth = 0) {
    if (depth > 20) {
        SDL_Log("failed to write temp library.");
        return false;
    }

    // Unikt temp-namn varje gång så vi aldrig skriver över en laddad fil
    char temp_name[256];
    snprintf(temp_name, sizeof(temp_name), "./libGnomes_game_temp_%d.so", load_counter++);

    char command[256];
    snprintf(command, sizeof(command), "cp %s %s", NAME_OF_LIB, temp_name);

    bool success = (system(command) == 0);
    if (!success) {
        usleep(50 * 1000);
        return LoadDLL(info, depth + 1);
    }

    info->handle = dlopen(temp_name, RTLD_NOW);

    if (!info->handle) {
        SDL_Log("could not load library: %s", dlerror());
        return false;
    }

    info->Initialize = (Function_Initialize) dlsym(info->handle, "Initialize");
    info->HandleEvents = (Function_HandleEvents) dlsym(info->handle, "HandleEvents");
    info->Update = (Function_Update) dlsym(info->handle, "Update");
    info->Draw = (Function_Draw) dlsym(info->handle, "Draw");
    info->OnQuit = (Function_OnQuit) dlsym(info->handle, "OnQuit");
    info->Timestamp = GetTimestamp();
    return true;
}

void UnloadDLL(DLL_INFO* info) {
    dlclose(info->handle);
    info->handle = nullptr;
}

void DLL_CheckStatus(DLL_INFO* dll) {
    time_t timestamp = GetTimestamp();
    bool is_timestamp_changed = (dll->Timestamp != timestamp);
    if (is_timestamp_changed) {
        usleep(100 * 1000);
        UnloadDLL(dll);
        LoadDLL(dll);
    }
}

void* AllocateGameMemory(size_t size) {
    void* blob = malloc(size);
    if (blob == nullptr) {
        SDL_Log("fatal error: could not allocate memory");
        return nullptr;
    }
    return blob;
}

void SDL_Setup() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("pilot", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
}

void CalculateDeltaTime(float& dt) {
    NOW = SDL_GetTicksNS();
    dt = NOW - PREV;
    dt = SDL_NS_TO_SECONDS(dt);
    PREV = NOW;
}

void CalculateRemainingFrameTime_MS(double* milliseconds) {
    Uint64 frame_end_time_ns = SDL_GetTicksNS();
    double frame_time_spent_ns = frame_end_time_ns - PREV;
    double frame_time_spent_ms = frame_time_spent_ns / 1e6;
    *milliseconds = FRAME_TIME_MS - frame_time_spent_ms;
}

void StoreGameState(Arena* arena) {
    std::ofstream file("temp_state.bin", std::ios::binary);
    file.write(reinterpret_cast<const char*>(arena->base), arena->size);
}

void RetrieveGameState(Arena* arena) {
    std::ifstream file("temp_state.bin", std::ios::binary);
    file.read(reinterpret_cast<char*>(arena->base), arena->size);
}

int main() {
    void* game_memory = AllocateGameMemory(GAME_MEMORY_ALLOWANCE);
    if (!game_memory)
        return 1;

    // Main memory
    Arena* arena_main = new Arena();
    Initialize(arena_main, game_memory, GAME_MEMORY_ALLOWANCE);
    GameData* gameData = ALLOC(arena_main, GameData);
    gameData->arena_main = arena_main;
    gameData->arena_scratch = CreateSubArena(arena_main, KILOBYTES(256));

    Gameplay* gameplay = &gameData->scenes.gameplay;

    //Sprites memory
    size_t IMAGE_ARENA_SIZE = MEGABYTES(1);
    gameData->arena_images = CreateSubArena(arena_main, IMAGE_ARENA_SIZE);
    gameData->tilesetBuffer = ALLOC_ARRAY(gameData->arena_images, Tileset, (int) TILESETS::COUNT);

    //Levels memory
    gameData->arena_levels = CreateSubArena(arena_main, MEGABYTES(3));
    gameData->arena_entities = CreateSubArena(gameData->arena_levels, MEGABYTES(1));
    gameplay->levels = ALLOC_ARRAY(gameData->arena_levels, LevelData, 5);

    //Commands memory
    gameData->arena_commands = CreateSubArena(gameData->arena_levels, MEGABYTES(1));
    gameplay->commandBuffer = ALLOC(gameData->arena_commands, CommandBuffer);
    gameplay->commandBuffer->capacity = 2000;
    gameplay->commandBuffer->allCommands = ALLOC_ARRAY(gameData->arena_commands, AnyCommand,
                                                       gameplay->commandBuffer->capacity);

    //Input buffer memory
    gameplay->input_buffer_capacity = 50;
    gameplay->input_buffer = ALLOC_ARRAY(gameData->arena_levels, Position, gameplay->input_buffer_capacity);

    //Input management memory
    size_t INPUT_ARENA_SIZE = 0;
    INPUT_ARENA_SIZE += sizeof(bool) * SDL_SCANCODE_COUNT * 2;
    INPUT_ARENA_SIZE += sizeof(float) * SDL_SCANCODE_COUNT;
    INPUT_ARENA_SIZE += 128;
    gameData->arena_input = CreateSubArena(arena_main, INPUT_ARENA_SIZE);
    gameData->input.keys_current = ALLOC_ARRAY(gameData->arena_input, bool, SDL_SCANCODE_COUNT);
    gameData->input.keys_previous = ALLOC_ARRAY(gameData->arena_input, bool, SDL_SCANCODE_COUNT);
    gameData->input.keys_held_time = ALLOC_ARRAY(gameData->arena_input, float, SDL_SCANCODE_COUNT);
    gameData->input.mouse_held_time = ALLOC_ARRAY(gameData->arena_input, float, SDL_SCANCODE_COUNT);

    DLL_INFO dll;

    bool dllLoaded = LoadDLL(&dll);
    if (!dllLoaded) {
        return 2;
    }

    SDL_Setup();

    dll.Initialize(gameData, window, renderer);

    bool running = true;
    float dt;
    gameData->dt = &dt;

    while (running) {
        DLL_CheckStatus(&dll);
        Reset(gameData->arena_scratch);
        CalculateDeltaTime(dt);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            running = dll.HandleEvents(arena_main, event);
        }

        gameData->input.keys_current = SDL_GetKeyboardState(nullptr);
        gameData->input.mouse_current = SDL_GetMouseState(&gameData->input.mouse_x, &gameData->input.mouse_y);
        dll.Update(gameData, dt);
        UpdateKeys(&gameData->input, dt);
        UpdateMouse(&gameData->input, dt);
        dll.Draw(gameData, renderer);

        double time_to_sleep_ms;
        CalculateRemainingFrameTime_MS(&time_to_sleep_ms);
        if (time_to_sleep_ms > 0) {
            if (time_to_sleep_ms > 1) {
                SDL_Delay(time_to_sleep_ms - 1);
            }
            while (time_to_sleep_ms > 0) {
                CalculateRemainingFrameTime_MS(&time_to_sleep_ms);
            }
        } else {
            printf("missed frame \n");
        }
    }

    dll.OnQuit(renderer);
    SDL_Quit();
    return 0;
}
