#pragma once
#include "levels.h"

struct Camera {
    float camera_x;
    float camera_y;
    float camera_z;
};

namespace camera {
    void GridToWorld(float* x, float* y, const LevelData* lvl, float zoom);

    void WorldToGrid(float x_world, float y_world, int* x, int* y, const LevelData* lvl, float zoom);

    bool GetIsPointInsideGrid(float x, float y, const LevelData* lvl, float zoom);
}
