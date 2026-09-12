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

enum class ENTITY_ID : uint8_t {
    MEDUSA = 0,
    GNOME = 1,
    ROCK = 2,
    SIREN = 3,
    GOLEM = 4,
};

enum class Direction {
    DOWN,
    RIGHT,
    LEFT,
    UP
};

inline Direction DirectionFromXY(int xDir, int yDir) {
    assert(xDir * yDir == 0);
    if (xDir == 1)
        return Direction::RIGHT;
    if (xDir == -1)
        return Direction::LEFT;
    if (yDir == -1)
        return Direction::UP;
    return Direction::DOWN;
}

struct Position {
    int x;
    int y;
};

enum class Actions {
    NONE = 0,
    MOVING = 1,
    ROTATING = 2
};


struct Entity {
    ENTITY_ID id;
    bool active;
    int x;
    int y;
    int x_prev;
    int y_prev;
    int strength;
    float progress_01;
    Actions action;
    Behaviour behaviour;
    Direction facing_current;
    Direction facing_previous;
};

bool IsActing(Entity* e);

bool HasBehaviour(const Entity* entity, Behaviour flags);

void InitializeBaseBehaviour(Entity* entity);

void SetBehaviour(Entity* entity, Behaviour flags);

void AddBehaviour(Entity* entity, Behaviour flags);

void RemoveBehaviour(Entity* entity, Behaviour flags);

void PostMove(Entity* entity, LevelData* level, CommandBuffer* commandBuffer);

void PostRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to);

void PreRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to);
