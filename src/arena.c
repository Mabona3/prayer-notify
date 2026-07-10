#include "arena.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "logger.h"

struct Arena {
  void *data;
  uintptr_t pos;
};

static Arena arena;

int arena_create() {
  arena.data = mmap(NULL, MB(512), PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
  if (arena.data == NULL) {
    log_msg(LOGLEVEL_ERROR, "mmap() failed\n");
    return EXIT_FAILURE;
  }
  arena.pos = (uintptr_t)arena.data;
  return EXIT_SUCCESS;
}

void arena_destroy() {
  if (munmap(arena.data, MB(512))) log_msg(LOGLEVEL_ERROR, "munmap() failed\n");
}

void arena_scratch_push(ScratchArena *scratch) {
  assert(scratch != NULL);
  *scratch = (ScratchArena){
      .arena         = &arena,
      .start_address = arena.pos,
      .size          = 0,
  };
}

void arena_scratch_pop(ScratchArena *scratch) {
  scratch->arena->pos = scratch->start_address;
}

void *arena_push(ScratchArena *scratch, uint64_t size) {
  uint64_t newSize = ALIGN_UP_POW2(size, 0x10);
  if ((uintptr_t)scratch->arena->data + MB(512) <
      (uintptr_t)scratch->start_address + scratch->size + newSize) {
    return NULL;
  }

  void *data = (void *)(scratch->start_address + scratch->size);
  scratch->size += newSize;
  scratch->arena->pos += newSize;
  return data;
}
