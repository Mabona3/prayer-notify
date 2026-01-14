#include "common.h"

#include <bits/getopt_core.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "prayerTimes.h"
#include "version.h"

#define CHECK_METHOD(method_array, method_count, optarg, result) \
  do {                                                           \
    result = method_count;                                       \
    for (int i = 0; i < method_count; i++) {                     \
      if (strcmp(optarg, method_array[i]) == 0) {                \
        result = i;                                              \
        break;                                                   \
      }                                                          \
    }                                                            \
    if (result == method_count) {                                \
      fprintf(stderr, "Error: Unknown method %s\n", optarg);     \
      return 2;                                                  \
    }                                                            \
  } while (0)

int handle_double_option(const char *optarg, double *value,
                         const char *option_name) {
  if (sscanf(optarg, "%lf", value) != 1) {
    fprintf(stderr, "Error: Invalid value for '%s': '%s'\n", option_name,
            optarg);
    return -1;
  }
  return 0;
}

// print basic config data to stdout
int parse_inputs(PrayerTimes *prayerTimes, int argc, char *argv[]) {
  opterr = 0;
  for (;;) {
    static struct option long_options[] = {
        {"help", optional_argument, NULL, 'h'},
        {"option", no_argument, NULL, 'o'},
        {"prayer", no_argument, NULL, 'r'},
        {"next", no_argument, NULL, 'n'},
        {"previous", no_argument, NULL, 'p'},
        {"version", no_argument, NULL, 'v'},
        {"date", required_argument, NULL, 'd'},
        {"timezone", required_argument, NULL, 'z'},
        {"latitude", required_argument, NULL, 't'},
        {"longitude", required_argument, NULL, 'g'},
        {"calc-method", required_argument, NULL, 'c'},
        {"asr-juristic-method", required_argument, NULL, 'a'},
        {"high-lats-method", required_argument, NULL, 'i'},
        {"dhuhr-minutes", required_argument, NULL, 0},
        {"maghrib-minutes", required_argument, NULL, 0},
        {"isha-minutes", required_argument, NULL, 0},
        {"fajr-angle", required_argument, NULL, 0},
        {"maghrib-angle", required_argument, NULL, 0},
        {"isha-angle", required_argument, NULL, 0},
        {0, 0, 0, 0}};

    enum {
      DHUHR_MINUTES = 9,
      MAGHRIB_MINUTES,
      ISHA_MINUTES,
      FAJR_ANGLE,
      MAGHRIB_ANGLE,
      ISHA_ANGLE,
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "h:ornpvd:z:t:g:c:a:i:", long_options,
                        &option_index);

    if (c == -1) break;  // Last option

    // if (!optarg && c != 'h' && c != 'v' && c != 'o' && c != 'n' && c != 'r'
    // &&
    //     c != 'p') {
    //   fprintf(stderr, "Error: %s option requires an argument\n",
    //           long_options[option_index].name);
    //   return 2;
    // }

    double arg;
    switch (c) {
      case 0:
        if (sscanf(optarg, "%lf", &arg) != 1) {
          fprintf(stderr, "Error: Invalid number '%s'\n", optarg);
          return 2;
        }
        switch (option_index) {
          case DHUHR_MINUTES:
            prayerTimes->dhuhr_minutes = arg;
            break;
          case MAGHRIB_MINUTES:
            set_maghrib_minutes(prayerTimes, arg);
            break;
          case ISHA_MINUTES:
            set_isha_minutes(prayerTimes, arg);
            break;
          case FAJR_ANGLE:
            set_fajr_angle(prayerTimes, arg);
            break;
          case MAGHRIB_ANGLE:
            set_maghrib_angle(prayerTimes, arg);
            break;
          case ISHA_ANGLE:
            set_isha_angle(prayerTimes, arg);
            break;
          default:
            fprintf(stderr, "Error: Invalid command line option\n");
            return 2;
        }
        break;
      case 'h':  // --help
        return print_help(optarg);
      case 'o':  // --stdout
        print_debug_help(prayerTimes);
        return 1;
      case 'r':  // --prayer
        print_prayer_times_help(prayerTimes);
        return 1;
      case 'n':  // --prayer
        print_next_prayer(prayerTimes);
        return 1;
      case 'p':  // --prayer
        print_previous_prayer(prayerTimes);
        return 1;
      case 'v':  // --version
        puts(PROG_NAME_FRIENDLY " " APP_VERSION);
        return 1;
      case 'd':  // --date
      {
        struct tm new_date = {0};
        if (sscanf(optarg, "%d-%d-%d", &new_date.tm_mday, &new_date.tm_mon,
                   &new_date.tm_year) != 3) {
          fprintf(stderr, "Error: Failed to parse '%s' as date\n", optarg);
          return 2;
        }
        new_date.tm_year -= 1900;
        new_date.tm_mon -= 1;
        prayerTimes->time = mktime(&new_date);
        break;
      }
      case 'z':  // --timezone
        if (handle_double_option(optarg, &prayerTimes->timezone, "timezone"))
          return 2;
        break;
      case 't':  // --latitude
        if (handle_double_option(optarg, &prayerTimes->latitude, "latitude"))
          return 2;
        break;
      case 'g':  // --longitude
        if (handle_double_option(optarg, &prayerTimes->longitude, "longitude"))
          return 2;
        break;
      case 'c':  // --calc-method
        prayerTimes->calc_method = CALCULATIONMETHOD_COUNT;
        for (int i = 0; i < CALCULATIONMETHOD_COUNT; i++) {
          if (strcmp(optarg, Calculation[i]) == 0) {
            prayerTimes->calc_method = i;
            break;
          }
        }
        if (prayerTimes->calc_method == CALCULATIONMETHOD_COUNT) {
          fprintf(stderr, "Error: Unknown method '%s'\n", optarg);
          return 2;
        }
        break;
      case 'a':  // --asr-juristic-method
        prayerTimes->asr_juristic = JURISTICMETHOD_COUNT;
        for (int i = 0; i < JURISTICMETHOD_COUNT; i++) {
          if (strcmp(optarg, Juristic[i]) == 0) {
            prayerTimes->asr_juristic = JURISTIC_Shafi;
            break;
          }
        }
        if (prayerTimes->asr_juristic == JURISTICMETHOD_COUNT) {
          fprintf(stderr, "Error: Unknown method '%s'\n", optarg);
          return 2;
        }
        break;
      case 'i':  // --high-lats-method
        CHECK_METHOD(Adjusting, ADJUSTINGMETHOD_COUNT, optarg,
                     prayerTimes->adjust_high_lats);
        break;
      case '?':
        if (optopt == 'h') {
          print_help(NULL);
          return 1;
        }
        break;
      default:
        fprintf(stderr, "Error: Unknown option '%c'\n", c);
        return 2;
    }
  }
  return 0;
}
