#include "option.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "logger.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "version.h"

#define PROG_NAME_FRIENDLY "prayer-notify"
#define PROG_NAME "prayer-notify"

#define VALUE_REQUIRED(i, narg, name)                                  \
  do                                                                   \
    if ((i) >= (narg)) {                                               \
      log_msg(LOGLEVEL_ERROR, "Error: option '%s' requires a value\n", \
              (name));                                                 \
      return -1;                                                       \
    }                                                                  \
  while (0)

static int handle_array_options(const char *option_name,
                                const char *method_array[],
                                unsigned int method_count, const char *arg,
                                unsigned int *result);

static int handle_double_option(int argc, int *i, char *argv[], double *value,
                                const char *option_name);

static int print_help(const char *arg);
static void print_debug_help(PrayerTimes *prayerTimes);
static void print_next_prayer(PrayerTimes *prayerTimes);
static void print_all_prayers(PrayerTimes *prayerTimes);
static void print_previous_prayer(PrayerTimes *prayerTimes);

int parse_inputs(PrayerTimes *prayerTimes, int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (!(strcmp(argv[i], "--help") && strcmp(argv[i], "-h"))) {
      if (i + 1 == argc || argv[i + 1][0] == '-') {
        return print_help(NULL);
      } else {
        ++i;
        return print_help(argv[i]);
      }
    } else if (!(strcmp(argv[i], "--option") && strcmp(argv[i], "-o"))) {
      print_debug_help(prayerTimes);
      return 1;
    } else if (!(strcmp(argv[i], "--next") && strcmp(argv[i], "-n"))) {
      print_next_prayer(prayerTimes);
      return 1;
    } else if (!(strcmp(argv[i], "--all") && strcmp(argv[i], "-a"))) {
      print_all_prayers(prayerTimes);
      return 1;
    } else if (!(strcmp(argv[i], "--log-level") && strcmp(argv[i], "-l"))) {
      ++i;
      VALUE_REQUIRED(i, argc, argv[i - 1]);
      if (!strcmp(argv[i], "Info")) {
        set_log_level(LOGLEVEL_INFO);
      } else if (!strcmp(argv[i], "Debug")) {
        set_log_level(LOGLEVEL_DEBUG);
      } else if (!strcmp(argv[i], "Warn")) {
        set_log_level(LOGLEVEL_WARN);
      } else if (!strcmp(argv[i], "Error")) {
        set_log_level(LOGLEVEL_ERROR);
      } else {
        log_msg(LOGLEVEL_ERROR, "Unknown level %s\n", argv[i]);
        return -1;
      }
    } else if (!(strcmp(argv[i], "--previous") && strcmp(argv[i], "-p"))) {
      print_previous_prayer(prayerTimes);
      return 1;
    } else if (!(strcmp(argv[i], "--version") && strcmp(argv[i], "-v"))) {
      puts(PROG_NAME_FRIENDLY " " APP_VERSION);
      return 1;
    } else if (!(strcmp(argv[i], "--time") && strcmp(argv[i], "-t"))) {
      struct tm *new_date = localtime(&prayerTimes->time);
      ++i;
      VALUE_REQUIRED(i, argc, argv[i - 1]);
      if (sscanf(argv[i], "%d:%d", &new_date->tm_hour, &new_date->tm_min) !=
          2) {
        log_msg(LOGLEVEL_ERROR, "Error: Failed to parse '%s' as time\n",
                argv[i]);
        return -1;
      }

      new_date->tm_sec  = 0;
      prayerTimes->time = mktime(new_date);
    } else if (!(strcmp(argv[i], "--date") && strcmp(argv[i], "-d"))) {
      struct tm *new_date = localtime(&prayerTimes->time);
      ++i;
      VALUE_REQUIRED(i, argc, argv[i - 1]);
      if (sscanf(argv[i], "%d-%d-%d", &new_date->tm_mday, &new_date->tm_mon,
                 &new_date->tm_year) != 3) {
        log_msg(LOGLEVEL_ERROR, "Error: Failed to parse '%s' as date\n",
                argv[i]);
        return -1;
      }

      new_date->tm_year -= 1900;
      new_date->tm_mon -= 1;
      prayerTimes->time = mktime(new_date);
    } else if (!(strcmp(argv[i], "--timezone") && strcmp(argv[i], "-z"))) {
      if (handle_double_option(argc, &i, argv, &prayerTimes->timezone, argv[i]))
        return -1;
    } else if (!(strcmp(argv[i], "--latitude") && strcmp(argv[i], "-u"))) {
      if (handle_double_option(argc, &i, argv, &prayerTimes->latitude, argv[i]))
        return -1;
    } else if (!(strcmp(argv[i], "--longitude") && strcmp(argv[i], "-g"))) {
      if (handle_double_option(argc, &i, argv, &prayerTimes->longitude,
                               argv[i]))
        return -1;
    } else if (!(strcmp(argv[i], "--calc-method") && strcmp(argv[i], "-c"))) {
      ++i;
      VALUE_REQUIRED(i, argc, argv[i - 1]);
      if (handle_array_options(argv[i - 1], Calculation,
                               CALCULATIONMETHOD_COUNT, argv[i],
                               &prayerTimes->calc_method))
        return -1;
    } else if (!(strcmp(argv[i], "--asr-juristic-method") &&
                 strcmp(argv[i], "-j"))) {
      ++i;
      VALUE_REQUIRED(i, argc, argv[i - 1]);
      if (handle_array_options(argv[i - 1], Juristic, JURISTICMETHOD_COUNT,
                               argv[i], &prayerTimes->asr_juristic))
        return -1;
    } else if (!(strcmp(argv[i], "--high-lats-method") &&
                 strcmp(argv[i], "-i"))) {
      ++i;
      VALUE_REQUIRED(i, argc, argv[i - 1]);
      if (handle_array_options(argv[i - 1], Adjusting, ADJUSTINGMETHOD_COUNT,
                               argv[i], &prayerTimes->adjust_high_lats))
        return -1;
    } else if (!strcmp(argv[i], "--dhuhr-minutes")) {
      if (handle_double_option(argc, &i, argv, &prayerTimes->dhuhr_minutes,
                               argv[i - 1]))
        return -1;
    } else {
      log_msg(LOGLEVEL_WARN, "Unknown option: %s", argv[i]);
    }
  }
  return 0;
}

int handle_array_options(const char *option_name, const char *method_array[],
                         unsigned int method_count, const char *arg,
                         unsigned int *result) {
  log_msg(LOGLEVEL_DEBUG, "Handling %s with arg %s\n", option_name, arg);
  *result = method_count;
  for (unsigned int i = 0; i < method_count; i++) {
    if (strcmp(arg, method_array[i]) == 0) {
      *result = i;
      break;
    }
  }

  if (*result == method_count) {
    log_msg(LOGLEVEL_ERROR, "Invalid option %s for %s\n", arg, option_name);
    return 1;
  }
  return 0;
}

int handle_double_option(int argc, int *i, char *argv[], double *value,
                         const char *option_name) {
  ++(*i);
  VALUE_REQUIRED((*i), argc, option_name);
  if (sscanf(argv[*i], "%lf", value) != 1) {
    log_msg(LOGLEVEL_ERROR, "Error: Invalid value for '%s': '%s'\n",
            option_name, argv[*i]);
    return -1;
  }
  return 0;
}

int print_help(const char *arg) {
  if (!arg) {
    puts("Usage: " PROG_NAME
         " [options]"
         "\nOptions:\n"
         "    -h, --help                      Display this help message\n"
         "    -v, --version                   Show the name and version of the "
         "program\n"
         "    -o, --option                    Print current options\n"
         "For detailed help, use: -h, --help <section>\n"
         "\nSections:\n"
         "    prayer\n"
         "    date\n"
         "    location\n"
         "    calculation\n"
         "    juristic\n"
         "    logging\n"
         "    adjustment\n");

    return 1;
  } else if (strcmp(arg, "prayer") == 0) {
    printf(
        "Prayer Options:\n"
        "    -n, --next                      Print the next prayer time\n"
        "    -p, --previous                  Print the previous prayer time.\n"
        "    -a, --all                       list all the prayers time.\n");
    return 1;
  } else if (strcmp(arg, "date") == 0) {
    printf(
        "Date Options:\n"
        "    -d, --date <arg>      Get prayer times for a specific date arg "
        "format: "
        "DD-MM-YYYY\n"
        "    -t, --time <arg>      Get prayer times for a specific time arg "
        "format: "
        "HH:MM\n"
        "    -z, --timezone <arg>      Get prayer times for a specific "
        "timezone\n");
    return 1;
  } else if (strcmp(arg, "location") == 0) {
    printf(
        "Location Options:\n"
        "    -u, --latitude  <arg>      Latitude of the desired location.\n"
        "    -g, --longitude <arg>      Longitude of the desired location.\n"
        "\n");
    return 1;
  } else if (strcmp(arg, "calculation") == 0) {
    printf(
        "Calculation Method Options:\n"
        "    -c, --calc-method <arg>          Select prayer time calculation "
        "method.\n\n"
        "Calculation Method Options:\n"
        "    Jafari            Ithna Ashari\n"
        "    Karachi           Islamic University of Sciences, Karachi\n"
        "    Isna              Islamic Society of North America (ISNA)\n"
        "    MWL               Muslim World League (MWL)\n"
        "    Makkah            Umm al-Qura, Makkah\n"
        "    Egypt             Egyptian General Authority of Survey\n"
        "    Custom            Custom Setting\n"
        "\n");
    return 1;
  } else if (strcmp(arg, "juristic") == 0) {
    printf(
        "Asr Juristic Method Options:\n"
        "    -j, --asr-juristic <arg>          Select Juristic method for Asr "
        "prayer time.\n\n"
        "Asr Juristic Method Options:\n"
        "    Shafii            Standard Shafii method\n"
        "    Hanafi            Hanafi method\n"
        "\n");
    return 1;
  } else if (strcmp(arg, "adjustment") == 0) {
    printf(
        "High Latitude Adjustment Options:\n"
        "    -i, --high-lats-method <arg>          Select adjustment method "
        "for "
        "higher latitudes.\n"
        "\n"
        "Dhuhr Adjustments:\n"
        "    --dhuhr-minutes <arg>              Minutes after mid-day to "
        "adjust Dhuhr prayer time.\n"
        "\n"
        "High Latitude Adjustment Options:\n"
        "    MidNight          Middle of the night\n"
        "    OneSeventh        One-seventh of the night\n"
        "    AngleBased        Angle/60th of the night\n"
        "    None              No adjustment");
    return 1;
  } else if (strcmp(arg, "logging") == 0) {
    printf(
        "Logging Options:\n"
        "    -l, --log-level <arg>          Selecting logging level\n"
        "\n"
        "Log Level Options: \n"
        "    Info           \n"
        "    Warn           \n"
        "    Error          \n");
    return 1;
  }
  log_msg(LOGLEVEL_ERROR, "Unknown help Option: %s\n", arg);
  return -1;
}

void print_debug_help(PrayerTimes *prayerTimes) {
  struct tm *t = localtime(&prayerTimes->time);
  char *config_file;
  if (get_config_file(&config_file)) {
    return;
  }
  printf(
      "Prayer Notification App\nDate: %02d/%02d/%04d\nTime: "
      "%02d:%02d:%02d\nconfig_file: "
      "%s\nLocation: (%lf, "
      "%lf)\nCalculation Method: %s\nJuristic Method: %s\nAdjustment "
      "Method: %s\n",
      t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min,
      t->tm_sec, config_file, prayerTimes->latitude, prayerTimes->longitude,
      Calculation[prayerTimes->calc_method],
      Juristic[prayerTimes->asr_juristic],
      Adjusting[prayerTimes->adjust_high_lats]);
}

void print_next_prayer(PrayerTimes *prayerTimes) {
  double times[TIMEID_TimesCount];
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);

  struct tm *date = localtime(&prayerTimes->time);

  struct tm times_dates[TIMEID_TimesCount];

  for (TimeID timeid = 0; timeid < TIMEID_TimesCount; timeid++) {
    times_dates[timeid]        = *date;
    times_dates[timeid].tm_sec = 0;
    get_float_time_parts(times[timeid], &times_dates[timeid].tm_hour,
                         &times_dates[timeid].tm_min);
    if (mktime(&times_dates[timeid]) > prayerTimes->time) {
      Time diff =
          convert_time_hms(mktime(&times_dates[timeid]) - prayerTimes->time);
      printf(
          "Upcoming Notification:\nNext Prayer: %s: %2.2d:%2.2d\nTime "
          "Remaining: %2.2d:%2.2d:%2.2d\n",
          TimeName[timeid], times_dates[timeid].tm_hour,
          times_dates[timeid].tm_min, diff.hours, diff.minutes, diff.seconds);
      return;
    }
  }

  time_add_day(date);

  PrayerTimes temp = *prayerTimes;
  temp.time        = mktime(date);
  get_prayer_times_time(&temp, temp.latitude, temp.longitude, temp.timezone,
                        times);

  times_dates[TIMEID_Fajr]        = *date;
  times_dates[TIMEID_Fajr].tm_sec = 0;
  get_float_time_parts(times[TIMEID_Fajr], &times_dates[TIMEID_Fajr].tm_hour,
                       &times_dates[TIMEID_Fajr].tm_min);
  Time diff = convert_time_hms(mktime(&times_dates[TIMEID_Fajr]) - time(NULL));
  printf(
      "Upcoming Notification:\nNext Prayer: %s: %2.2d:%2.2d\nTime "
      "Remaining: %2.2d:%2.2d:%2.2d\n",
      TimeName[TIMEID_Fajr], times_dates[TIMEID_Fajr].tm_hour,
      times_dates[TIMEID_Fajr].tm_min, diff.hours, diff.minutes, diff.seconds);
}

void print_all_prayers(PrayerTimes *prayerTimes) {
  double times[TIMEID_TimesCount];
  struct tm times_dates[TIMEID_TimesCount];

  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);

  struct tm *date = localtime(&prayerTimes->time);
  for (TimeID timeid = 0; timeid < TIMEID_TimesCount; ++timeid) {
    times_dates[timeid]        = *date;
    times_dates[timeid].tm_sec = 0;
    get_float_time_parts(times[timeid], &times_dates[timeid].tm_hour,
                         &times_dates[timeid].tm_min);
    printf("Prayer %s: %2.2d:%2.2d\n", TimeName[timeid],
           times_dates[timeid].tm_hour, times_dates[timeid].tm_min);
  }
}

void print_previous_prayer(PrayerTimes *prayerTimes) {
  double times[TIMEID_TimesCount];
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);

  struct tm times_dates[TIMEID_TimesCount];
  struct tm *date = localtime(&prayerTimes->time);
  for (TimeID timeid = TIMEID_TimesCount - 1; timeid > 0; timeid--) {
    times_dates[timeid]        = *date;
    times_dates[timeid].tm_sec = 0;
    get_float_time_parts(times[timeid], &times_dates[timeid].tm_hour,
                         &times_dates[timeid].tm_min);
    if (mktime(&times_dates[timeid]) < prayerTimes->time) {
      Time diff =
          convert_time_hms(prayerTimes->time - mktime(&times_dates[timeid]));
      printf(
          "Previous Prayer: %s: %2.2d:%2.2d\nTime "
          "Elapsed: %2.2d:%2.2d:%2.2d\n",
          TimeName[timeid], times_dates[timeid].tm_hour,
          times_dates[timeid].tm_min, diff.hours, diff.minutes, diff.seconds);
      return;
    }
  }

  time_sub_day(date);
  PrayerTimes temp = *prayerTimes;
  temp.time        = mktime(date);
  get_prayer_times_time(&temp, temp.latitude, temp.longitude, temp.timezone,
                        times);
  times_dates[TIMEID_Isha] = *date;
  get_float_time_parts(times[TIMEID_Isha], &times_dates[TIMEID_Isha].tm_hour,
                       &times_dates[TIMEID_Isha].tm_min);
  Time diff = convert_time_hms(time(NULL) - mktime(&times_dates[TIMEID_Isha]));
  printf(
      "Previous Prayer: %s: %2.2d:%2.2d\nTime "
      "Elapsed: %2.2d:%2.2d:%2.2d\n",
      TimeName[TIMEID_Isha], times_dates[TIMEID_Isha].tm_hour,
      times_dates[TIMEID_Isha].tm_min, diff.hours, diff.minutes, diff.seconds);
}
