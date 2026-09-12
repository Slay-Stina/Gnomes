#include "command.h"

#include <cassert>

#include "entity.h"
#include "levels.h"

void Execute(AnyCommand cmd, LevelData* level, CommandBuffer* buffer, bool from_redo = false) {
    switch (cmd.command.type) {
        case CMD_TYPE::NONE:
            break;
        case CMD_TYPE::MOVE: {
            MoveCommand* mv = &cmd.move;
            mv->entity->x_prev = mv->entity->x;
            mv->entity->y_prev = mv->entity->y;
            mv->entity->x += mv->xDir;
            mv->entity->y += mv->yDir;
            if (from_redo) {
                mv->entity->progress_01 = 1;
            }
            PostMove(mv->entity, level, buffer);
            break;
        }
        case CMD_TYPE::ROTATE: {
            RotateCommand* rotate = &cmd.rotate;
            if (!HasBehaviour(rotate->entity, CAN_ROTATE)) {
                break;
            }
            PreRotation(rotate->entity, level, buffer, rotate->from, rotate->to);
            rotate->entity->facing = rotate->to;
            PostRotation(rotate->entity, level, buffer, rotate->from, rotate->to);
            break;
        }
        case CMD_TYPE::MODIFY_BEHAVIOUR: {
            ModifyBehaviourCommand* modify = &cmd.modify;
            if (modify->mode == ModifyBehaviourCommand::ADD) {
                AddBehaviour(modify->entity, modify->flag);
            } else {
                RemoveBehaviour(modify->entity, modify->flag);
            }
            break;
        }
        case CMD_TYPE::ADD: {
            AddCommand* add = &cmd.add;
            AddEntity(add->id, add->x, add->y, level);
            break;
        }
        case CMD_TYPE::REMOVE: {
            RemoveCommand* remove = &cmd.remove;
            RemoveEntity(remove->x, remove->y, level);
            break;
        }
        case CMD_TYPE::EDIT: {
            EditCommand* edit = &cmd.edit;
            level->cells[edit->y * level->w + edit->x] = (int) edit->id;
            break;
        }
    }
}

void Push(CommandBuffer* buffer, AnyCommand cmd, LevelData* level) {
    assert(cmd.command.type != CMD_TYPE::NONE);
    buffer->allCommands[buffer->index] = cmd;
    buffer->allCommands[buffer->index].command.timestamp = buffer->timestamp;
    buffer->index++;
    buffer->head = buffer->index;
    Execute(cmd, level, buffer);
}

void Undo(CommandBuffer* buffer, LevelData* level) {
    if (buffer->index == 0) {
        return;
    }
    buffer->index--;
    AnyCommand cmd = buffer->allCommands[buffer->index];
    uint32_t timestamp = cmd.command.timestamp;
    switch (cmd.command.type) {
        case CMD_TYPE::NONE:
            break;
        case CMD_TYPE::MOVE: {
            MoveCommand mv = cmd.move;
            mv.entity->x -= mv.xDir;
            mv.entity->y -= mv.yDir;
            mv.entity->progress_01 = 1;
            break;
        }
        case CMD_TYPE::ROTATE: {
            RotateCommand rotate = cmd.rotate;
            if (!HasBehaviour(rotate.entity, CAN_ROTATE)) {
                break;
            }
            rotate.entity->facing = rotate.from;
            break;
        }
        case CMD_TYPE::MODIFY_BEHAVIOUR: {
            ModifyBehaviourCommand modify = cmd.modify;
            if (modify.mode == ModifyBehaviourCommand::ADD) {
                // note that this is flipped compared to Execute()
                RemoveBehaviour(modify.entity, modify.flag);
            } else {
                AddBehaviour(modify.entity, modify.flag);
            }
            break;
        }
        case CMD_TYPE::ADD: {
            AddCommand* add = &cmd.add;
            RemoveEntity(add->x, add->y, level);
            break;
        }
        case CMD_TYPE::REMOVE: {
            RemoveCommand* remove = &cmd.remove;
            AddEntity(remove->storedID, remove->x, remove->y, level);
            Entity* entity = GetEntity(level, remove->x, remove->y);
            if (entity == nullptr) {
                break;
            }
            SetBehaviour(entity, remove->storedBehaviour);
            break;
        }
        case CMD_TYPE::EDIT: {
            EditCommand* edit = &cmd.edit;
            level->cells[edit->y * level->w + edit->x] = edit->previous;
            break;
        }
    }
    if (buffer->index > 0) {
        if (buffer->allCommands[buffer->index - 1].command.timestamp == timestamp) {
            Undo(buffer, level);
        }
    }
}

void Redo(CommandBuffer* buffer, LevelData* level) {
    if (buffer->index == buffer->head) {
        return;
    }
    AnyCommand cmd = buffer->allCommands[buffer->index];
    Execute(cmd, level, buffer, true);
    buffer->index++;
    int timestamp = cmd.command.timestamp;
    if (buffer->index != buffer->head) {
        AnyCommand nextCommand = buffer->allCommands[buffer->index];
        if (nextCommand.command.timestamp == timestamp) {
            Redo(buffer, level);
        }
    }
}
