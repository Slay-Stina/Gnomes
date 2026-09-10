#pragma once
#include <cstdint>

struct LevelData;
enum Behaviour : std::uint32_t;
enum class Direction;
struct Entity;

enum class CMD_TYPE : uint8_t {
    NONE = 0,
    MOVE = 1,
    ROTATE = 2,
    MODIFY_BEHAVIOUR = 3
};

struct Command {
    CMD_TYPE type = CMD_TYPE::NONE;
    uint32_t timestamp;
};

struct MoveCommand : Command {
    Entity* entity;
    int xDir;
    int yDir;

    MoveCommand(Entity* _entity, int _xDir, int _yDir) {
        entity = _entity;
        xDir = _xDir;
        yDir = _yDir;
        type = CMD_TYPE::MOVE;
    }
};

struct RotateCommand : Command {
    Entity* entity;
    Direction from;
    Direction to;

    RotateCommand(Entity* _entity, Direction _from, Direction _to) {
        entity = _entity;
        from = _from;
        to = _to;
        type = CMD_TYPE::ROTATE;
    }
};

struct ModifyBehaviourCommand : Command {
    enum Mode {
        ADD,
        REMOVE
    };

    Entity* entity;
    Behaviour flag;
    Mode mode;

    ModifyBehaviourCommand(Entity* _entity, Behaviour _flag, Mode _mode) {
        entity = _entity;
        flag = _flag;
        mode = _mode;
        type = CMD_TYPE::MODIFY_BEHAVIOUR;
    }
};

union AnyCommand {
    Command command;
    MoveCommand move;
    RotateCommand rotate;
    ModifyBehaviourCommand modify;

    AnyCommand(MoveCommand mv) {
        move = mv;
    };

    AnyCommand(RotateCommand rc) {
        rotate = rc;
    };

    AnyCommand(ModifyBehaviourCommand mod) {
        modify = mod;
    }
};

struct CommandBuffer {
    AnyCommand* allCommands;
    uint32_t timestamp;
    int capacity;
    int index;
    int head;
};

void Push(CommandBuffer* buffer, AnyCommand cmd, LevelData* level);

void Undo(CommandBuffer* buffer);

void Redo(CommandBuffer* buffer, LevelData* level);
