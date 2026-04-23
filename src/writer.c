#include "writer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "logger.h"
#include "prayerTimes.h"

void close_current_writer() {
  char *temp_file;
  if (get_temp_file(&temp_file) || remove(temp_file) != EXIT_SUCCESS) {
    return;
  }
  log_msg(LOGLEVEL_DEBUG, "file %s removed successfully", temp_file);

  free(temp_file);
  return;
}

int write_current(struct tm *times, int current) {
  log_msg(LOGLEVEL_DEBUG, "Writing current json file\n");
  char *temp_file;
  if (get_temp_file(&temp_file)) {
    return EXIT_FAILURE;
  }

  char *temp_write =
      malloc(sizeof(char) * (strlen(temp_file) + strlen(".tmp") + 1));
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

  if (fputs(buffer, file) == EOF) {
    log_msg(LOGLEVEL_ERROR, "Error writing to the file '%s': %s\n", temp_write,
            strerror(errno));
    free(temp_write);
    return EXIT_FAILURE;
  }

  if (fclose(file) == EOF) {
    log_msg(LOGLEVEL_ERROR, "Error closing file '%s': %s\n", temp_write,
            strerror(errno));
    free(temp_write);
    return EXIT_FAILURE;
  }

  log_msg(LOGLEVEL_DEBUG, "Renaming '%s' into '%s'\n", temp_write, temp_file);
  if (rename(temp_write, temp_file) == -1) {
    log_msg(LOGLEVEL_ERROR, "Error closing file '%s': %s\n", temp_write,
            strerror(errno));
    free(temp_write);
    free(temp_file);
    return EXIT_FAILURE;
  }

  free(temp_write);
  free(temp_file);
  return EXIT_SUCCESS;
}

bool check_temp_file() {
  char *temp_file;
  if (get_temp_file(&temp_file)) {
    log_msg(LOGLEVEL_ERROR, "Error retreiving the temp file\n");
    return true;
  }

  struct stat file_stat;
  if (!stat(temp_file, &file_stat)) {
    log_msg(LOGLEVEL_DEBUG, "Temp file found another instance is running\n");
    return true;
  }

  return false;
}
