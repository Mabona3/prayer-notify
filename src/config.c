#include "config.h"

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "logger.h"

const char *CONFIG_FILE = ".config/prayer-notify/config.json";
const char *TEMP_FILE   = "prayer-notify.json";

/**
 * Creating the parent dir of the config file if the parent dir is not found.
 * returns 0 on success otherwise it will return 1.
 */
static int create_parent_dir(const char *config_file);

static int get_config_dir_child(const char *file_name, char **output);

int get_config_file(char **config_file) {
  log_msg(LOGLEVEL_DEBUG, "Searching the PRAYER_NOTIFY_CONFIG env");
  int offset         = strlen(".config/prayer-notify/");
  char *default_home = getenv("PRAYER_NOTIFY_CONFIG");
  if (default_home == NULL) {
    offset -= strlen("prayer-notify/");
    log_msg(LOGLEVEL_DEBUG,
            "PRAYER_NOTIFY_CONFIG not found trying XDG_CONFIG_HOME");
    default_home = getenv("XDG_CONFIG_HOME");
    if (default_home == NULL) {
      offset       = 0;
      default_home = getenv("HOME");
      if (default_home == NULL) {
        log_msg(LOGLEVEL_ERROR, "cannot find home dir");
        return -1;
      }
    }
  }

  log_msg(LOGLEVEL_DEBUG, "Current CONFIG_FILE %s and Current offset is %d",
          CONFIG_FILE, offset);
  size_t len   = strlen(default_home) + strlen(CONFIG_FILE) - offset + 2;
  *config_file = malloc(len);
  if (*config_file == NULL) {
    log_msg(LOGLEVEL_ERROR, "Cannot malloc for config file");
    return -1;
  }
  snprintf(*config_file, len, "%s/%s", default_home, CONFIG_FILE + offset);
  log_msg(LOGLEVEL_DEBUG, "Current CONFIG_FILE %s", *config_file);

  if (create_parent_dir(*config_file) != 0) {
    free(*config_file);
    *config_file = NULL;
    return -1;
  }

  return 0;
}

int get_icon_file(char **output) {
  return get_config_dir_child("icon.jpg", output);
}

int get_audio_file(char **output) {
  return get_config_dir_child("audio.mp3", output);
}

int get_temp_file(char **temp_dir) {
  if (temp_dir == NULL) {
    return -1;
  }

  log_msg(LOGLEVEL_DEBUG, "Trying the 'PRAYER_NOTIFY_TEMP' env variable");
  char *default_temp = getenv("PRAYER_NOTIFY_TEMP");
  if (default_temp != NULL) {
    log_msg(LOGLEVEL_DEBUG, "'PRAYER_NOTIFY_TEMP' found as '%s'", default_temp);
    int len   = strlen(default_temp);
    *temp_dir = malloc(sizeof(char) * (len + 1));
    if (!*temp_dir) {
      log_msg(LOGLEVEL_ERROR, "Memory allocation failed");
      return -1;
    }
    strcpy(*temp_dir, default_temp);

    return 0;
  }

  log_msg(LOGLEVEL_DEBUG, "Trying the 'TEMP' env variable");
  default_temp = getenv("TEMP");
  if (default_temp == NULL) {
    log_msg(LOGLEVEL_DEBUG, "'TEMP' env variable not found falling to '/tmp'");
    default_temp = "/tmp";
  }

  size_t len = strlen(default_temp) + strlen(TEMP_FILE) + 2;
  *temp_dir  = (char *)malloc(len * sizeof(char));
  if (*temp_dir == NULL) {
    log_msg(LOGLEVEL_ERROR, "Failed to allocate memory for 'temp_dir'");
    return -1;
  }

  snprintf(*temp_dir, len, "%s/%s", default_temp, TEMP_FILE);
  log_msg(LOGLEVEL_DEBUG, "Setting the temp file info '%s'", *temp_dir);
  return 0;
}

int create_parent_dir(const char *config_file) {
  if (!config_file) return 1;

  int len          = strlen(config_file);
  char *config_dup = malloc(sizeof(char) * (len + 1));
  if (!config_dup) {
    log_msg(LOGLEVEL_ERROR, "Memory allocation failed");
    return 1;
  }
  strcpy(config_dup, config_file);

  char *config_dir = dirname(config_dup);
  struct stat st;
  if (stat(config_dir, &st) == -1) {
    log_msg(LOGLEVEL_DEBUG, "directory '%s' not found creating", config_dir);
    if (mkdir(config_dir, 0755) == -1) {
      log_msg(LOGLEVEL_ERROR, "Could not create directory '%s': %s", config_dir,
              strerror(errno));
      free(config_dup);
      return 1;
    }
  }

  free(config_dup);
  return 0;
}

int get_config_dir_child(const char *file_name, char **output) {
  char *config_file;
  if (get_config_file(&config_file)) {
    return -1;
  }

  int config_file_len = strlen(config_file) + 1;
  char *config_dir    = malloc(sizeof(char) * config_file_len);
  if (config_dir == NULL) {
    free(config_dir);
    log_msg(LOGLEVEL_ERROR, "Memory allocation failed");
    return -1;
  }
  strcpy(config_dir, config_file);
  free(config_file);
  char *parent_config_dir = dirname(config_dir);
  size_t len              = strlen(parent_config_dir) + strlen(file_name) + 1;
  *output                 = malloc(sizeof(char) * len);
  if (*output == NULL) {
    free(config_dir);
    return -1;
  }
  snprintf(*output, len, "%s/%s", parent_config_dir, file_name);
  free(config_dir);

  struct stat buf;
  if (stat(*output, &buf) == -1) {
    free(*output);
    *output = NULL;
    return -1;
  }
  return 0;
}
