#include "writer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "arena.h"
#include "config.h"
#include "logger.h"
#include "prayerTimes.h"

void close_current_writer() {
  char *temp_file = NULL;
  ScratchArena scratch;
  arena_scratch_push(&scratch);
  if (get_temp_file(&scratch, &temp_file) ||
      remove(temp_file) != EXIT_SUCCESS) {
    log_msg(LOGLEVEL_ERROR, "Error removing %s: '%s'", temp_file,
            strerror(errno));
    arena_scratch_pop(&scratch);
    return;
  }

  log_msg(LOGLEVEL_DEBUG, "file %s removed successfully", temp_file);
  arena_scratch_pop(&scratch);
  return;
}

int write_current(struct tm *times, int current) {
  log_msg(LOGLEVEL_DEBUG, "Writing current json file");
  char *temp_file = NULL;

  ScratchArena scratch;
  arena_scratch_push(&scratch);
  if (get_temp_file(&scratch, &temp_file)) {
    arena_scratch_pop(&scratch);
    return EXIT_FAILURE;
  }

  size_t len       = strlen(temp_file) + strlen(".tmp") + 1;
  char *temp_write = arena_push(&scratch, sizeof(char) * len);
  if (temp_write == NULL) {
    arena_scratch_pop(&scratch);
    return EXIT_FAILURE;
  }
  snprintf(temp_write, len, "%s.tmp", temp_file);

  FILE *file = fopen(temp_write, "w");
  if (file == NULL) {
    arena_scratch_pop(&scratch);
    return EXIT_FAILURE;
  }

  int buffer_size = 160;
  char buffer[160];
  int count = snprintf(
      buffer, buffer_size, "{\"text\":\"%s %02d:%02d\",\"tooltip\":\"",
      TimeName[current], times[current].tm_hour, times[current].tm_min);
  for (int i = 0; i < TIMEID_TimesCount; i++) {
    count += snprintf(buffer + count, buffer_size - count, "%s %02d:%02d%s",
                      TimeName[i], times[i].tm_hour, times[i].tm_min,
                      i == TIMEID_Isha ? "" : "\\n");
  }

  snprintf(buffer + count, buffer_size - count, "\"}");

  if (fputs(buffer, file) == EOF) {
    log_msg(LOGLEVEL_ERROR, "Error writing to the file '%s': %s", temp_write,
            strerror(errno));
    arena_scratch_pop(&scratch);
    return EXIT_FAILURE;
  }

  if (fclose(file) == EOF) {
    log_msg(LOGLEVEL_ERROR, "Error closing file '%s': %s", temp_write,
            strerror(errno));
    arena_scratch_pop(&scratch);
    return EXIT_FAILURE;
  }

  log_msg(LOGLEVEL_DEBUG, "Renaming '%s' into '%s'", temp_write, temp_file);
  if (rename(temp_write, temp_file) == -1) {
    log_msg(LOGLEVEL_ERROR, "Error closing file '%s': %s", temp_write,
            strerror(errno));
    arena_scratch_pop(&scratch);
    return EXIT_FAILURE;
  }

  arena_scratch_pop(&scratch);
  return EXIT_SUCCESS;
}

bool check_temp_file() {
  char *temp_file = NULL;
  ScratchArena scratch;
  arena_scratch_push(&scratch);
  if (get_temp_file(&scratch, &temp_file)) {
    log_msg(LOGLEVEL_ERROR, "Error retreiving the temp file");
    arena_scratch_pop(&scratch);
    return true;
  }

  struct stat file_stat;
  if (!stat(temp_file, &file_stat)) {
    log_msg(LOGLEVEL_DEBUG, "Temp file found another instance is running");
    arena_scratch_pop(&scratch);
    return true;
  }

  arena_scratch_pop(&scratch);
  return false;
}
