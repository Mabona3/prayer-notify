#include "common.h"
#include "config.h"
#include "timeHandle.h"
#include "version.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

// print basic config data to stdout
void print_data(PrayerTimes *prayerTimes) {
  struct tm *t = localtime(&prayerTimes->time);
  const char *config_file = get_config_file();
  printf("Current config file is: %s\n"
         "Times for current is: "
         "\n%02d/%02d/%d %2.2d:%2.2d:%2.2d"
         "\n{"
         "\n\tlng: %lf,"
         "\n\tlat: %lf,"
         "\n\tcalc-method: %s,"
         "\n\tjuristic: %s,"
         "\n\tadjusting: %s"
         "\n}\n",
         config_file, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour,
         t->tm_min, t->tm_sec, prayerTimes->longitude, prayerTimes->latitude,
         Calculation[prayerTimes->calc_method],
         Juristic[prayerTimes->asr_juristic],
         Adjusting[prayerTimes->adjust_high_lats]);

  double times[TIMEID_TimesCount];
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude,
                        get_effective_timezone_time(prayerTimes->time), times);

  struct tm times_dates[TIMEID_TimesCount];
  prayerTimes->time = time(NULL);
  struct tm *date = localtime(&prayerTimes->time);

  // preparing the times
  for (TimeID timeid = 0; timeid < TIMEID_TimesCount; timeid++) {
    times_dates[timeid] = *date;
    times_dates[timeid].tm_sec = 0;
    get_float_time_parts(times[timeid], &times_dates[timeid].tm_hour,
                         &times_dates[timeid].tm_min);
    printf("%s : %2.2d:%2.2d\n", TimeName[timeid], times_dates[timeid].tm_hour,
           times_dates[timeid].tm_min);
  }
}

// print the help msg to the stdout.
void print_help(FILE *f) {
  fputs(PROG_NAME_FRIENDLY " " APP_VERSION "\n\n", f);
  fputs("Usage: " PROG_NAME " options...\n"
        "\n"
        " Options\n"
        "    --help                      -h  you're reading it\n"
        "    --stdout                    -o  print the times into stdout \n"
        "    --version                   -v  prints name and version, then "
        "exits\n"
        "    --date arg                  -d  get prayer times for arbitrary "
        "date\n"
        "    --timezone arg              -z  get prayer times for arbitrary "
        "timezone\n"
        "  * --latitude arg              -l  latitude of desired location\n"
        "  * --longitude arg             -n  longitude of desired location\n"
        "    --calc-method arg           -c  select prayer time calculation "
        "method\n"
        "    --asr-juristic-method arg   -a  select Juristic method for "
        "calculating Asr prayer time\n"
        "    --high-lats-method arg      -i  select adjusting method for "
        "higher latitude\n"
        "    --dhuhr-minutes arg             minutes after mid-way for "
        "calculating Dhuhr prayer time\n"
        " ** --maghrib-minutes arg           minutes after sunset for "
        "calculating Maghrib prayer time\n"
        " ** --isha-minutes arg              minutes after Maghrib for "
        "calculating Isha prayer time\n"
        " ** --fajr-angle arg                angle for calculating Fajr prayer "
        "time\n"
        " ** --maghrib-angle arg             angle for calculating Maghrib "
        "prayer time\n"
        " ** --isha-angle arg                angle for calculating Isha prayer "
        "time\n"
        "\n"
        "  * These options are required\n"
        " ** By providing any of these options the calculation method is set "
        "to custom\n"
        "\n"
        " Possible arguments for --calc-method\n"
        "    jafari        Ithna Ashari\n"
        "    karachi       University of Islamic Sciences, Karachi\n"
        "    isna          Islamic Society of North America (ISNA)\n"
        "    mwl           Muslim World League (MWL)\n"
        "    makkah        Umm al-Qura, Makkah\n"
        "    egypt         Egyptian General Authority of Survey\n"
        "    custom        Custom Setting\n"
        "\n"
        " Possible arguments for --asr-juristic-method\n"
        "    shafii        Shafii (standard)\n"
        "    hanafi        Hanafi\n"
        "\n"
        " Possible arguments for --high-lats-method\n"
        "    none          No adjustment\n"
        "    midnight      middle of night\n"
        "    oneseventh    1/7th of night\n"
        "    anglebased    angle/60th of night\n",
        f);
}

int parse_inputs(PrayerTimes *prayerTimes, int argc, char *argv[]) {
  for (;;) {
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"stdout", no_argument, NULL, 'o'},
        {"version", no_argument, NULL, 'v'},
        {"date", required_argument, NULL, 'd'},
        {"timezone", required_argument, NULL, 'z'},
        {"latitude", required_argument, NULL, 'l'},
        {"longitude", required_argument, NULL, 'n'},
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
    int c = getopt_long(argc, argv, "hovd:z:l:n:c:a:i:", long_options,
                        &option_index);

    if (c == -1)
      break; // Last option

    if (!optarg && c != 'h' && c != 'v' && c != 'o') {
      fprintf(stderr, "Error: %s option requires an argument\n",
              long_options[option_index].name);
      return 2;
    }

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
    case 'h': // --help
      print_help(stdout);
      return 1;
    case 'o': // --stdout
      print_data(prayerTimes);
      return 1;
    case 'v': // --version
      puts(PROG_NAME_FRIENDLY " " APP_VERSION);
      return 1;
    case 'd': // --date
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
    case 'z': // --timezone
      if (sscanf(optarg, "%lf", &prayerTimes->timezone) != 1) {
        fprintf(stderr, "Error: Invalid timezone '%s'\n", optarg);
        return 2;
      }
      break;
    case 'l': // --latitude
      if (sscanf(optarg, "%lf", &prayerTimes->latitude) != 1) {
        fprintf(stderr, "Error: Invalid latitude '%s'\n", optarg);
        return 2;
      }
      break;
    case 'n': // --longitude
      if (sscanf(optarg, "%lf", &prayerTimes->longitude) != 1) {
        fprintf(stderr, "Error: Invalid longitude '%s'\n", optarg);
        return 2;
      }
      break;
    case 'c': // --calc-method
      if (strcmp(optarg, "jafari") == 0)
        prayerTimes->calc_method = CALCULATION_Jafari;
      else if (strcmp(optarg, "karachi") == 0)
        prayerTimes->calc_method = CALCULATION_Karachi;
      else if (strcmp(optarg, "isna") == 0)
        prayerTimes->calc_method = CALCULATION_ISNA;
      else if (strcmp(optarg, "mwl") == 0)
        prayerTimes->calc_method = CALCULATION_MWL;
      else if (strcmp(optarg, "makkah") == 0)
        prayerTimes->calc_method = CALCULATION_Makkah;
      else if (strcmp(optarg, "egypt") == 0)
        prayerTimes->calc_method = CALCULATION_Egypt;
      else if (strcmp(optarg, "custom") == 0)
        prayerTimes->calc_method = CALCULATION_Custom;
      else {
        fprintf(stderr, "Error: Unknown method '%s'\n", optarg);
        return 2;
      }
      break;
    case 'a': // --asr-juristic-method
      if (strcmp(optarg, "shafii") == 0)
        prayerTimes->asr_juristic = JURISTIC_Shafi;
      else if (strcmp(optarg, "hanafi") == 0)
        prayerTimes->asr_juristic = JURISTIC_Hanafi;
      else {
        fprintf(stderr, "Error: Unknown method '%s'\n", optarg);
        return 2;
      }
      break;
    case 'i': // --high-lats-method
      if (strcmp(optarg, "none") == 0)
        prayerTimes->adjust_high_lats = ADJUSTING_None;
      else if (strcmp(optarg, "midnight") == 0)
        prayerTimes->adjust_high_lats = ADJUSTING_MidNight;
      else if (strcmp(optarg, "oneseventh") == 0)
        prayerTimes->adjust_high_lats = ADJUSTING_OneSeventh;
      else if (strcmp(optarg, "anglebased") == 0)
        prayerTimes->adjust_high_lats = ADJUSTING_AngleBased;
      else {
        fprintf(stderr, "Error: Unknown method '%s'\n", optarg);
        return 2;
      }
      break;
    default:
      fprintf(stderr, "Error: Unknown option '%c'\n", c);
      print_help(stderr);
      return 2;
    }
  }
  return 0;
}
