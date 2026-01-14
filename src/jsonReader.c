#include "jsonReader.h"

#include <cjson/cJSON.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "prayerTimes.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BUFFER_SIZE 512

#define JSON_RETURN_WITH_ERROR(jsondata, json, file, name, error_string) \
  do {                                                                   \
    fprintf(stderr, error_string, name);                                 \
    cJSON_Delete(json);                                                  \
    fclose(file);                                                        \
    free(jsondata);                                                      \
    return NULL;                                                         \
  } while (0)

// PrayerTimes is the struct type that holds the data
// json is the cJson struct that holds the json file data
// location is the field in PrayerTimes struct that should be retrieved
// tmp is a tmp cJson that holds the retrieved from the cjson.
// name is the name of the field that should be retrieved from the struct
#define JSON_RETRIEVE_DOUBLE(jsondata, json, location, tmp, name) \
  do {                                                            \
    jsondata->location = NAN;                                     \
    tmp = cJSON_GetObjectItemCaseSensitive(json, name);           \
    if (cJSON_IsNumber(tmp)) {                                    \
      jsondata->location = tmp->valuedouble;                      \
    } else {                                                      \
      JSON_RETURN_WITH_ERROR(                                     \
          jsondata, json, file, name,                             \
          "Error parsing config file '%s'\nInvalid type\n");      \
    }                                                             \
  } while (0)

// PrayerTimes is the struct type that holds the data
// json is the cJson struct that holds the json file data
// location is the field in PrayerTimes struct that should be retrieved
// tmp is a tmp cJson that holds the retrieved from the cjson.
// arr is an array that holds the string that should compared to find the enum
// name is the name of the field that should be retrieved from the struct
#define JSON_RETRIEVE_ENUM_ARRAY(jsondata, json, location, tmp, arr, type, \
                                 name)                                     \
  do {                                                                     \
    tmp = cJSON_GetObjectItemCaseSensitive(json, name);                    \
    jsondata->location = (type)ARRAY_SIZE(arr);                            \
    if (cJSON_IsString(tmp)) {                                             \
      for (unsigned int i = 0; i < ARRAY_SIZE(arr); ++i) {                 \
        if (strcmp(arr[i], tmp->valuestring) == 0) {                       \
          jsondata->location = (type)i;                                    \
          break;                                                           \
        }                                                                  \
      }                                                                    \
      if (jsondata->location == (type)ARRAY_SIZE(arr)) {                   \
        JSON_RETURN_WITH_ERROR(                                            \
            jsondata, json, file, name,                                    \
            "Error parsing config file '%s'\nInvalid value\n");            \
      }                                                                    \
    } else {                                                               \
      JSON_RETURN_WITH_ERROR(                                              \
          jsondata, json, file, name,                                      \
          "Error parsing config file '%s'\nInvalid type\n");               \
    }                                                                      \
  } while (0)

PrayerTimes *get_default_config() {
  PrayerTimes *data = create_prayer_times(CALCULATION_Jafari, JURISTIC_Shafi,
                                          ADJUSTING_MidNight, 0);
  return data;
}

// Construct the config to carry the default config based on the PrayerTimes.
// the config_file is assumed to be in .config and has a parent dir that could
// be not found so when failing the parent dir will be created and then the file
// will be created
int build_default_config(PrayerTimes *data, const char *config_file);

PrayerTimes *read_config() {
  PrayerTimes *jsondata = get_default_config();
  if (jsondata == NULL) {
    return NULL;
  }
  char buffer[BUFFER_SIZE];
  FILE *file;
  char *config_file;
  if (get_config_file(&config_file)) {
    return NULL;
  }

  if (!(file = fopen(config_file, "r"))) {
    return (build_default_config(jsondata, config_file) != 0) ? NULL : jsondata;
  }

  fread(buffer, sizeof(*buffer), ARRAY_SIZE(buffer), file);
  cJSON *json = cJSON_Parse(buffer);

  if (json == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
      printf("Error: %s\n", error_ptr);
    }
    free(jsondata);
    cJSON_Delete(json);
    return NULL;
  }
  cJSON *tmp;

  JSON_RETRIEVE_ENUM_ARRAY(jsondata, json, asr_juristic, tmp, Juristic,
                           JuristicMethod, "Juristic");
  JSON_RETRIEVE_ENUM_ARRAY(jsondata, json, adjust_high_lats, tmp, Adjusting,
                           AdjustingMethod, "Adjusting");
  JSON_RETRIEVE_ENUM_ARRAY(jsondata, json, calc_method, tmp, Calculation,
                           CalculationMethod, "Calculation");
  if (strcmp(Calculation[jsondata->calc_method], "Custom") == 0) {
    JSON_RETRIEVE_DOUBLE(jsondata, json,
                         method_params[CALCULATION_Custom].fajr_angle, tmp,
                         "fajr_angle");
    set_fajr_angle(jsondata,
                   jsondata->method_params[CALCULATION_Custom].fajr_angle);
    JSON_RETRIEVE_DOUBLE(jsondata, json,
                         method_params[CALCULATION_Custom].isha_value, tmp,
                         "isha_angle");
    if (jsondata->method_params[CALCULATION_Custom].isha_value != 0) {
      set_isha_angle(jsondata,
                     jsondata->method_params[CALCULATION_Custom].isha_value);
    } else {
      JSON_RETRIEVE_DOUBLE(jsondata, json,
                           method_params[CALCULATION_Custom].isha_value, tmp,
                           "isha_minutes");
      set_isha_minutes(jsondata,
                       jsondata->method_params[CALCULATION_Custom].isha_value);
    }
    JSON_RETRIEVE_DOUBLE(jsondata, json,
                         method_params[CALCULATION_Custom].maghrib_value, tmp,
                         "maghrib_angle");
    if (jsondata->method_params[CALCULATION_Custom].maghrib_value != 0) {
      set_maghrib_angle(
          jsondata, jsondata->method_params[CALCULATION_Custom].maghrib_value);
    } else {
      JSON_RETRIEVE_DOUBLE(jsondata, json,
                           method_params[CALCULATION_Custom].maghrib_value, tmp,
                           "maghrib_minutes");
      set_maghrib_minutes(
          jsondata, jsondata->method_params[CALCULATION_Custom].isha_value);
    }
  }
  JSON_RETRIEVE_DOUBLE(jsondata, json, longitude, tmp, "lng");
  JSON_RETRIEVE_DOUBLE(jsondata, json, latitude, tmp, "lat");
  JSON_RETRIEVE_DOUBLE(jsondata, json, dhuhr_minutes, tmp, "dhuhr_minutes");

  // delete the JSON object
  cJSON_Delete(json);
  fclose(file);
  return jsondata;
}

int build_default_config(PrayerTimes *data, const char *config_file) {
  char buffer[BUFFER_SIZE];
  FILE *file = fopen(config_file, "w");
  if (!file) {
    fprintf(stderr, "Could not create file '%s': %s\n", config_file,
            strerror(errno));
    return -1;
  }

  int written = snprintf(
      buffer, sizeof(buffer),
      "{\n"
      "\t\"lat\": %lf,\n"
      "\t\"lng\": %lf,\n"
      "\t\"Juristic\": \"%s\",\n"
      "\t\"Calculation\": \"%s\",\n"
      "\t\"Adjusting\": \"%s\",\n"
      "\t\"dhuhr_minutes\": %lf,\n"
      "\t\"fajr_angle\": %lf,\n"
      "\t\"isha_angle\": %lf,\n"
      "\t\"maghrib_angle\": %lf,\n"
      "\t\"isha_minutes\": %lf,\n"
      "\t\"maghrib_minutes\": %lf\n"
      "}\n",
      data->latitude, data->longitude, Juristic[data->asr_juristic],
      Calculation[data->calc_method], Adjusting[data->adjust_high_lats],
      data->dhuhr_minutes, 0.f, 0.f, 0.f, 0.f, 0.f);

  if (written < 0 || (size_t)written >= sizeof(buffer)) {
    fprintf(stderr, "Config buffer overflow or encoding error\n");
    fclose(file);
    return -1;
  }

  if (fwrite(buffer, 1, written, file) != (size_t)written) {
    fprintf(stderr, "Failed to write config file '%s': %s\n", config_file,
            strerror(errno));
    fclose(file);
    return -1;
  }

  fclose(file);
  return 0;
}
