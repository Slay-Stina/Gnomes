#include "arena.h"

#include <cstring>

void Memory::Initialize(Arena* arena, void* mem_start, size_t size) {
    arena->base = (unsigned char*) mem_start;
    arena->size = size;
    arena->used = 0;
}

void* Memory::Allocate(Arena* arena, size_t size) {
    void* front = arena->base + arena->used;
    arena->used += size;
    memset(front, 0, size);
    return front;
}

void Memory::Reset(Arena* arena) {
    arena->used = 0;
}

Memory::Arena* Memory::CreateSubArena(Arena* parent_arena, size_t size) {
    Arena* sub_arena = ALLOC(parent_arena, Arena);
    void* memory_start = Allocate(parent_arena, size);
    Initialize(sub_arena, memory_start, size);
    return sub_arena;
}
