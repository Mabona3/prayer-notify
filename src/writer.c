#include "writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "prayerTimes.h"

int close_current_writer() {
  char *temp_file;
  if (get_temp_file(&temp_file) || remove(temp_file) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  free(temp_file);
  return 0;
}

int write_current(struct tm *times, int current) {
  char *temp_file;
  if (get_temp_file(&temp_file)) {
    return EXIT_FAILURE;
  }

  char *temp_write = malloc(sizeof(char) * (strlen(temp_file) + 5));
  sprintf(temp_write, "%s.tmp", temp_file);

  FILE *file = fopen(temp_write, "w");
  if (file == NULL) {
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

  fputs(buffer, file);
  fclose(file);

  if (rename(temp_write, temp_file)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

bool check_temp_file() {
  char *temp_file;
  if (get_temp_file(&temp_file)) {
    fputs("Error retreiving the temp file\n", stderr);
    return true;
  }

  struct stat file_stat;
  if (!stat(temp_file, &file_stat)) {
    return true;
  }

  return false;
}
