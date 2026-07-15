#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

#define KB(n) ((uint64_t)(n) << 10)
#define MB(n) ((uint64_t)(n) << 20)
#define GB(n) ((uint64_t)(n) << 30)

#define ALIGN_UP_POW2(n, p) \
  (((uint64_t)(n) + ((uint64_t)(p) - 1)) & (~((uint64_t)(p) - 1)))

typedef struct Arena Arena;

typedef struct {
  Arena *arena;
  uintptr_t start_address;
  uint64_t size;
} ScratchArena;

/*
 * mmap the chunk and save it in an internal struct
 */
int arena_create();

/*
 * munmap the chunk and clean the internal struct
 */
void arena_destroy();

void *arena_push(ScratchArena *scratch, uint64_t size);

void arena_scratch_push(ScratchArena *scratch);
void arena_scratch_pop(ScratchArena *scratch);

#endif  // !ARENA_H
