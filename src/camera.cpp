#include "camera.h"
#include "common.h"

bool camera::GetIsPointInsideGrid( float x, float y, const LevelData* lvl, float zoom ) {
    int x_grid;
    int y_grid;
    WorldToGrid(x, y, &x_grid, &y_grid, lvl, zoom);
    return x_grid >= 0 && y_grid >= 0 && x_grid < lvl->w && y_grid < lvl->h;
}

void camera::GridToWorld( float* x, float* y, const LevelData* lvl, float zoom ) {
    float tile = TILE_SIZE_PX_SCALED * zoom;
    *x = *x * tile + SCREEN_WIDTH / 2.0f - lvl->w * tile / 2.0f;
    *y = *y * tile + SCREEN_HEIGHT / 2.0f - lvl->h * tile / 2.0f;
}

void camera::WorldToGrid( float x_world, float y_world, int* x, int* y, const LevelData* lvl, float zoom ) {
    float tile = TILE_SIZE_PX_SCALED * zoom;
    *x = (int) ((x_world - SCREEN_WIDTH / 2.0f) / tile + lvl->w / 2.0f);
    *y = (int) ((y_world - SCREEN_HEIGHT / 2.0f) / tile + lvl->h / 2.0f);
}
