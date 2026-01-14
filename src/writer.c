#include "writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "prayerTimes.h"

int close_current_writer() {
  char *temp_file;
  if (get_temp_file(&temp_file)) {
    return EXIT_FAILURE;
  }

  if (remove(temp_file) != EXIT_SUCCESS) {
    return 1;
  }

  free(temp_file);
  return 0;
}

int write_current(struct tm *t, TimeID current) {
  char *temp_file;
  if (get_temp_file(&temp_file)) {
    return EXIT_FAILURE;
  }

  unsigned long len = strlen(temp_file) + 5;
  char *temp_write = malloc(sizeof(char) * len);

  sprintf(temp_write, "%s.tmp", temp_file);

  FILE *file = fopen(temp_write, "w");
  // the buffer should contains only 3 sections
  // MAGHRIB 12:00
  char buffer[16];
  sprintf(buffer, "%s %2.2d:%2.2d", TimeName[current], t->tm_hour, t->tm_min);
  fwrite(buffer, sizeof(char), strlen(buffer), file);
  fclose(file);

  rename(temp_write, temp_file);

  return EXIT_SUCCESS;
}
