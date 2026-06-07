#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

#define KB(n) ((uint64_t)(n) << 10)
#define MB(n) ((uint64_t)(n) << 20)
#define GB(n) ((uint64_t)(n) << 30)

#define ALIGN_UP_POW2(n, p) \
  (((uint64_t)(n) + ((uint64_t)(p) - 1)) & (~((uint64_t)(p) - 1)))

typedef struct {
  uint64_t capacity;
  uint64_t pos;
} Arena;

// Create the Arena
Arena *arena_create(uint64_t capacity);
void arena_destroy(Arena *arena);

void *arena_push(Arena *arena, uint64_t size);
void *arena_pop(Arena *arena, uint64_t size);
void *arena_pop_to(Arena *arena, uint64_t pos);
void *arena_clear(Arena *arena);

#endif  // !ARENA_H
