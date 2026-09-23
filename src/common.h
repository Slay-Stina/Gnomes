#pragma once

#define KILOBYTES(n) ((size_t) n * 1024)
#define MEGABYTES(n) (KILOBYTES(n) * 1024)
#define GIGABYTES(n) (MEGABYTES(n) * 1024)

#define AS_KILOBYTES(b) ((double) (b) / KILOBYTES(1))
#define AS_MEGABYTES(b) (AS_KILOBYTES(b) / 1024.0)
#define AS_GIGABYTES(b) (AS_MEGABYTES(b) / 1024.0)

constexpr size_t GAME_MEMORY_ALLOWANCE = MEGABYTES(14);
constexpr size_t AUDIO_MEMORY_ALLOWANCE = MEGABYTES(5);

constexpr int TARGET_FPS = 60;
const double FRAME_TIME_MS = 1000.0 / TARGET_FPS;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 540;

const int UPSCALE_FACTOR = 4;
const int TILE_SIZE_PX_RAW = 16;
const int TILE_SIZE_PX_SCALED = TILE_SIZE_PX_RAW * UPSCALE_FACTOR;

const float MOVE_SPEED = 6.0;
const float UNDO_REPEAT_TIME = 0.15;
const float LEVEL_COMPLETE_DELAY = 0.3f;

const int MOUSE_BUTTON_COUNT = 3;

static const char STOP_CHAR = '\0';

inline bool IsStringEmpty( const char* str ) {
    return str == nullptr || str[0] == STOP_CHAR;
}

inline void Expand1DTo2D( int flatIndex, int width, int* x, int* y ) {
    *x = flatIndex % width;
    *y = flatIndex / width;
}

inline void Expand1DTo2D( int flatIndex, int width, float* x, float* y ) {
    *x = (float) (flatIndex % width);
    *y = (float) (flatIndex / width);
}
