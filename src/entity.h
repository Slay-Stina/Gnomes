#pragma once
#include <cassert>
#include <cstdint>


struct CommandBuffer;
struct LevelData;

enum Behaviour : uint32_t {
    NONE = 0,
    CAN_MOVE = 1 << 0,
    IS_PLAYER = 1 << 1,
    RESPOND_TO_INPUT = 1 << 2,
    IS_PETRIFIED = 1 << 3,
    CAN_ROTATE = 1 << 4,
    UNPUSHABLE = 1 << 5,
    JUMPS = 1 << 6,
    IS_PUSHING = 1 << 7
};

enum class ID {
    NONE = 0,
    GROUND = 1,
    WALL = 2,
    DEMON = 3,
    ROCK = 4,
    GOLEM = 5,
    MEDUSA = 6,
    SIREN = 7
};

enum class Direction {
    RIGHT,
    LEFT,
    UP,
    DOWN
};

inline Direction DirectionFromXY(int xDir, int yDir) {
    assert(xDir * yDir == 0);
    if (xDir == 1) {
        return Direction::RIGHT;
    }
    if (xDir == -1) {
        return Direction::LEFT;
    }
    if (yDir == 1) {
        return Direction::UP;
    }
    return Direction::DOWN;
}

struct Position {
    int x;
    int y;
};

struct Entity {
    ID id;
    int x;
    int y;
    int x_prev;
    int y_prev;
    int strength;
    float progress_01;
    Behaviour behaviour;
    Direction facing;
};

bool HasBehaviour(const Entity* entity, Behaviour flags);

void InitializeBaseBehaviour(Entity* entity);

void SetBehaviour(Entity* entity, Behaviour flags);

void AddBehaviour(Entity* entity, Behaviour flags);

void RemoveBehaviour(Entity* entity, Behaviour flags);

bool IsMoving(Entity* e);

void PostMove(Entity* entity, LevelData* level, CommandBuffer* commandBuffer);

void PostRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to);

void PreRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to);
