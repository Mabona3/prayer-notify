#include "config.h"

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define CONFIG_FILE "prayer-notify/config.json"

static char *config_file = NULL;

// Creating the parent dir of the config file if the parent dir is not found. it
// returns 0 on success otherwise it will return 1.
int create_parent_dir(const char *config_file) {
  char *config_dup = strdup(config_file);
  if (!config_dup) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  char *config_dir = dirname(config_dup);
  struct stat st;
  if (stat(config_dir, &st) == -1) {
    if (mkdir(config_dir, 0755) == -1) {
      fprintf(stderr, "Could not create directory '%s': %s\n", config_dir,
              strerror(errno));
      free(config_dup);
      return 1;
    }
  }

  free(config_dup);
  return 0;
}

const char *get_config_file() {
  if (config_file != NULL) {
    return config_file;
  }

  char *default_home = getenv("PRAYER_NOTIFY_CONFIG");
  if (default_home != NULL) {
    config_file = strdup(default_home);
    if (!config_file) {
      fprintf(stderr, "Memory allocation failed\n");
      return NULL;
    }

    if (create_parent_dir(config_file) != 0) {
      free(config_file);
      config_file = NULL;
      return NULL;
    }
    return config_file;
  }

  default_home = getenv("XDG_CONFIG_HOME");
  if (default_home != NULL) {
    size_t len = strlen(default_home) + strlen("/") + strlen(CONFIG_FILE) + 1;
    config_file = malloc(len);
    if (!config_file) {
      fprintf(stderr, "cannot allocate memory for config file");
      return NULL;
    }
    snprintf(config_file, len, "%s/%s", default_home, CONFIG_FILE);

    if (create_parent_dir(config_file) != 0) {
      free(config_file);
      config_file = NULL;
      return NULL;
    }
    return config_file;
  }

  default_home = getenv("HOME");
  if (default_home == NULL) {
    fprintf(stderr, "cannot find home dir\n");
    return NULL;
  }

  size_t len = strlen(default_home) + strlen("/.config/" CONFIG_FILE) + 1;
  config_file = malloc(len);
  if (config_file == NULL) {
    fprintf(stderr, "Cannot malloc for config file\n");
  }
  snprintf(config_file, len, "%s/.config/%s", default_home, CONFIG_FILE);

  if (create_parent_dir(config_file) != 0) {
    free(config_file);
    config_file = NULL;
    return NULL;
  }

  return config_file;
}

void free_config_file() {
  if (config_file) {
    free((void *)config_file);
    config_file = NULL;
  }
}
