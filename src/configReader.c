#include "configReader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logger.h"
#include "prayerTimes.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BUFFER_SIZE 512

void parse_ini(const char *buffer, int len);

// Construct the config to carry the default config based on the PrayerTimes.
// the config_file is assumed to be in .config and has a parent dir that could
// be not found so when failing the parent dir will be created and then the file
// will be created
int build_default_config(PrayerTimes *data, const char *config_file);

int read_config(PrayerTimes *prayerTimes) {
  assert(prayerTimes);
  if (prayerTimes == NULL) {
    return -1;
  }
  char buffer[BUFFER_SIZE];
  FILE *file;
  char *config_file;
  if (get_config_file(&config_file)) {
    log_msg(LOGLEVEL_ERROR, "Error getting the config file.");
    return -1;
  }

  if (!(file = fopen(config_file, "r"))) {
    int ret = build_default_config(prayerTimes, config_file);
    free(config_file);
    return (ret != 0) ? -1 : 0;
  }
  int len;
  while ((len = fread(buffer, sizeof(*buffer), BUFFER_SIZE, file)) != 0) {
    parse_ini(buffer, len);
  }

  free(config_file);
  return 0;
}

int build_default_config(PrayerTimes *data, const char *config_file) {
  char buffer[BUFFER_SIZE];
  FILE *file = fopen(config_file, "w");
  if (!file) {
    log_msg(LOGLEVEL_ERROR, "Could not create file '%s': %s\n", config_file,
            strerror(errno));
    return -1;
  }

  int written = snprintf(
      buffer, sizeof(buffer),
      "lat=%lf\n"
      "lng=%lf\n"
      "Juristic=%s\\n"
      "Calculation=%s\n"
      "Adjusting=%s\n"
      "dhuhr_minutes=%lf\n"
      "fajr_angle=%lf\n"
      "isha_angle=%lf\n"
      "maghrib_angle=%lf\n"
      "isha_minutes=%lf\n"
      "maghrib_minutes=%lf\n",
      data->latitude, data->longitude, Juristic[data->asr_juristic],
      Calculation[data->calc_method], Adjusting[data->adjust_high_lats],
      data->dhuhr_minutes, 0.f, 0.f, 0.f, 0.f, 0.f);

  if (written < 0 || (size_t)written >= sizeof(buffer) ||
      (fwrite(buffer, 1, written, file) != (size_t)written)) {
    log_msg(LOGLEVEL_ERROR, "Failed to write the config file '%s': %s\n",
            config_file, strerror(errno));
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}

// TODO: this should be finished
void parse_ini(const char *buffer, int len) {
}
