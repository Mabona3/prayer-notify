#include "config.h"

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "logger.h"

const char *CONFIG_FILE = "prayer-notify/config.json";
const char *TEMP_FILE   = "prayer-notify.json";

// Creating the parent dir of the config file if the parent dir is not found. it
// returns 0 on success otherwise it will return 1.
int create_parent_dir(const char *config_file) {
  char *config_dup = strdup(config_file);
  if (!config_dup) {
    log_msg(LOGLEVEL_ERROR, "Memory allocation failed\n");
    return 1;
  }

  char *config_dir = dirname(config_dup);
  struct stat st;
  if (stat(config_dir, &st) == -1) {
    log_msg(LOGLEVEL_DEBUG, "directory '%s' not found creating\n", config_dir);
    if (mkdir(config_dir, 0755) == -1) {
      log_msg(LOGLEVEL_ERROR, "Could not create directory '%s': %s\n",
              config_dir, strerror(errno));
      free(config_dup);
      return 1;
    }
  }

  free(config_dup);
  return 0;
}

int get_temp_file(char **temp_dir) {
  if (temp_dir == NULL) {
    return -1;
  }

  log_msg(LOGLEVEL_DEBUG, "Trying the 'PRAYER_NOTIFY_TEMP' env variable\n");
  char *default_temp = getenv("PRAYER_NOTIFY_TEMP");
  if (default_temp != NULL) {
    log_msg(LOGLEVEL_DEBUG, "'PRAYER_NOTIFY_TEMP' found as '%s'\n",
            default_temp);
    *temp_dir = strdup(default_temp);
    if (!*temp_dir) {
      log_msg(LOGLEVEL_ERROR, "Memory allocation failed\n");
      return -1;
    }

    return 0;
  }

  log_msg(LOGLEVEL_DEBUG, "Trying the 'TEMP' env variable\n");
  default_temp = getenv("TEMP");
  if (default_temp == NULL) {
    log_msg(LOGLEVEL_DEBUG,
            "'TEMP' env variable not found falling to '/tmp'\n");
    default_temp = "/tmp";
  }

  size_t len = strlen(default_temp) + strlen(TEMP_FILE) + 2;
  *temp_dir  = (char *)malloc(len * sizeof(char));
  if (*temp_dir == NULL) {
    log_msg(LOGLEVEL_ERROR, "Failed to allocate memory for 'temp_dir'\n");
    return -1;
  }

  sprintf(*temp_dir, "%s/%s", default_temp, TEMP_FILE);
  log_msg(LOGLEVEL_DEBUG, "Setting the temp file info '%s'\n", *temp_dir);
  return 0;
}

int get_config_dir_child(const char *file_name, char **output) {
  char *config_file;
  if (get_config_file(&config_file)) {
    return -1;
  }

  char *config_dir        = strdup(config_file);
  char *parent_config_dir = dirname(config_dir);
  size_t len              = strlen(parent_config_dir) + strlen(file_name) + 1;
  *output                 = malloc(sizeof(char) * len);
  sprintf(*output, "%s/%s", parent_config_dir, file_name);
  free(config_dir);
  struct stat buf;
  if (stat(*output, &buf) == -1) {
    free(*output);
    output = NULL;
    return -1;
  }
  return 0;
}

int get_audio_file(char **output) {
  return get_config_dir_child("audio.mp3", output);
}

int get_icon_file(char **output) {
  return get_config_dir_child("icon.jpg", output);
}

int get_config_file(char **config_file) {
  log_msg(LOGLEVEL_DEBUG, "Searching the PRAYER_NOTIFY_CONFIG env");
  char *default_home = getenv("PRAYER_NOTIFY_CONFIG");
  if (default_home != NULL) {
    *config_file = strdup(default_home);
    if (!*config_file) {
      log_msg(LOGLEVEL_ERROR, "Memory allocation failed\n");
      return -1;
    }

    if (create_parent_dir(*config_file) != 0) {
      free(*config_file);
      *config_file = NULL;
      return -1;
    }
    return 0;
  }
  log_msg(LOGLEVEL_DEBUG,
          "PRAYER_NOTIFY_CONFIG not found trying XDG_CONFIG_HOME\n");

  default_home = getenv("XDG_CONFIG_HOME");
  if (default_home != NULL) {
    size_t len   = strlen(default_home) + strlen("/") + strlen(CONFIG_FILE) + 1;
    *config_file = malloc(len);
    if (!*config_file) {
      log_msg(LOGLEVEL_ERROR, "cannot allocate memory for config file");
      return -1;
    }
    snprintf(*config_file, len, "%s/%s", default_home, CONFIG_FILE);

    if (create_parent_dir(*config_file) != 0) {
      free(*config_file);
      *config_file = NULL;
      return -1;
    }
    return 0;
  }

  default_home = getenv("HOME");
  if (default_home == NULL) {
    log_msg(LOGLEVEL_ERROR, "cannot find home dir\n");
    return -1;
  }

  size_t len =
      strlen(default_home) + strlen("/.config/") + strlen(CONFIG_FILE) + 1;
  *config_file = malloc(len);
  if (*config_file == NULL) {
    log_msg(LOGLEVEL_ERROR, "Cannot malloc for config file\n");
    return -1;
  }
  snprintf(*config_file, len, "%s/.config/%s", default_home, CONFIG_FILE);

  if (create_parent_dir(*config_file) != 0) {
    free(*config_file);
    *config_file = NULL;
    return -1;
  }

  return 0;
}
