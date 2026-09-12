#include <cassert>

#include "command.h"
#include "entity.h"
#include "levels.h"

bool IsActing(Entity* e) {
    if (e->active == false)
        return false;
    return e->action != Actions::NONE;
}

bool HasBehaviour(const Entity* entity, Behaviour flags) {
    return (entity->behaviour & flags) == flags;
}

void InitializeBaseBehaviour(Entity* entity) {
    assert(entity->active);
    switch (entity->id) {
        default:
            SetBehaviour(entity, NONE);
            break;
        case ENTITY_ID::GNOME:
            SetBehaviour(entity, (Behaviour) (CAN_ROTATE | CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
            AddBehaviour(entity, JUMPS);
            entity->strength = 1;
            break;
        case ENTITY_ID::GOLEM:
            SetBehaviour(entity, (Behaviour) (CAN_ROTATE | CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
            AddBehaviour(entity, UNPUSHABLE);
            entity->strength = 999;
            break;

        case ENTITY_ID::MEDUSA:
            SetBehaviour(entity, (Behaviour) (CAN_ROTATE | CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
            AddBehaviour(entity, JUMPS);
            entity->strength = 1;
            break;
        case ENTITY_ID::SIREN:
            SetBehaviour(entity, (Behaviour) (CAN_ROTATE | CAN_MOVE | IS_PLAYER | RESPOND_TO_INPUT));
            entity->strength = 0;
            break;
        case ENTITY_ID::ROCK:
            SetBehaviour(entity, CAN_MOVE);
            break;
    }
}

void SetBehaviour(Entity* entity, Behaviour flags) {
    entity->behaviour = flags;
}

void AddBehaviour(Entity* entity, Behaviour flags) {
    entity->behaviour = (Behaviour) (entity->behaviour | flags);
}

void RemoveBehaviour(Entity* entity, Behaviour flags) {
    entity->behaviour = (Behaviour) (entity->behaviour & ~flags);
}

void PostMove(Entity* entity, LevelData* level, CommandBuffer* buffer) {
    if (entity->id == ENTITY_ID::MEDUSA) {
        Entity* entity_looked_at = RaycastFirstEntity(entity->x, entity->y, entity->facing_current, level);
        if (entity_looked_at != nullptr) {
            if (!HasBehaviour(entity_looked_at, IS_PETRIFIED)) {
                ModifyBehaviourCommand modify(entity_looked_at, IS_PETRIFIED, ModifyBehaviourCommand::ADD);
                Push(buffer, modify, level);
            }
        }
    }
}

void PostRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to) {
    if (from == to) {
        return;
    }
    if (entity->id == ENTITY_ID::MEDUSA) {
        Entity* entity_looked_at = RaycastFirstEntity(entity->x, entity->y, to, level);
        if (entity_looked_at != nullptr) {
            if (!HasBehaviour(entity_looked_at, IS_PETRIFIED)) {
                ModifyBehaviourCommand modify(entity_looked_at, IS_PETRIFIED, ModifyBehaviourCommand::ADD);
                Push(commandBuffer, modify, level);
            }
        }
    }
}

void PreRotation(Entity* entity, LevelData* level, CommandBuffer* commandBuffer, Direction from, Direction to) {
    if (from == to) {
        return;
    }
    if (entity->id == ENTITY_ID::MEDUSA) {
        Entity* entity_previously_looked_at = RaycastFirstEntity(entity->x, entity->y, from, level);
        if (entity_previously_looked_at != nullptr) {
            if (HasBehaviour(entity_previously_looked_at, IS_PETRIFIED)) {
                ModifyBehaviourCommand modify(entity_previously_looked_at, IS_PETRIFIED,
                                              ModifyBehaviourCommand::REMOVE);
                Push(commandBuffer, modify, level);
            }
        }
    }
}
