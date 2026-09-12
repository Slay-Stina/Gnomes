#pragma once
#include <cstdint>

#include "entity.h"

struct LevelData;

enum class CMD_TYPE : uint8_t {
    NONE = 0,
    MOVE = 1,
    ROTATE = 2,
    MODIFY_BEHAVIOUR = 3,
    ADD = 4,
    REMOVE = 5,
    EDIT = 6,
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

struct AddCommand : Command {
    int x;
    int y;
    ID id;

    AddCommand(int _x, int _y, ID _id) {
        x = _x;
        y = _y;
        id = _id;
        type = CMD_TYPE::ADD;
    }
};

struct RemoveCommand : Command {
    int x;
    int y;
    Behaviour storedBehaviour;
    ID storedID;

    RemoveCommand(Entity* entity) {
        x = entity->x;
        y = entity->y;
        storedBehaviour = entity->behaviour;
        storedID = entity->id;
        type = CMD_TYPE::REMOVE;
    }
};

struct EditCommand : Command {
    int x;
    int y;
    ID id;
    uint8_t previous;

    EditCommand(int _x, int _y, ID _id, uint8_t prev) {
        x = _x;
        y = _y;
        id = _id;
        previous = prev;
        type = CMD_TYPE::EDIT;
    }
};

union AnyCommand {
    Command command;
    MoveCommand move;
    RotateCommand rotate;
    ModifyBehaviourCommand modify;
    AddCommand add;
    RemoveCommand remove;
    EditCommand edit;

    AnyCommand(MoveCommand mov) {
        move = mov;
    };

    AnyCommand(RotateCommand rot) {
        rotate = rot;
    };

    AnyCommand(ModifyBehaviourCommand mod) {
        modify = mod;
    }

    AnyCommand(AddCommand _add) {
        add = _add;
    }

    AnyCommand(RemoveCommand rem) {
        remove = rem;
    }

    AnyCommand(EditCommand edt) {
        edit = edt;
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

void Undo(CommandBuffer* buffer, LevelData* level);

void Redo(CommandBuffer* buffer, LevelData* level);
