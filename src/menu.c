#include "menu.h"

#include <bits/getopt_core.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "prayerTimes.h"
#include "timeHandle.h"

int print_help(const char *arg) {
  if (!arg) {
    puts("Usage: " PROG_NAME
         " [options]"
         "\nOptions:\n"
         "    --help, -h                  Display this help message\n"
         "    --version, -v               Show the name and version "
         "of the program\n"
         "    --options, -o                Print available options\n"
         "For detailed help, use: --help <section>\n"
         "\nSections:\n"
         "    prayer\n"
         "    date\n"
         "    location\n"
         "    calculation\n"
         "    juristic\n"
         "    adjustment\n");
    return 1;
  } else if (strcmp(arg, "prayer") == 0) {
    printf(
        "Prayer Options:\n"
        "    --next, -n                  Print the next prayer time\n"
        "    --previous, -p              Print the previous prayer time.\n");
    return 1;
  } else if (strcmp(arg, "date") == 0) {
    printf(
        "Date Options:\n"
        "    --date <arg>, -d <arg>      Get prayer times for a specific date\n"
        "    --timezone <arg>, -z <arg>  Get prayer times for a specific "
        "timezone\n");
    return 1;
  } else if (strcmp(arg, "location") == 0) {
    printf(
        "Location Options:\n"
        "    --latitude <arg>, -t <arg>  Latitude of the desired location.\n"
        "    --longitude <arg>, -g <arg>  Longitude of the desired location.\n"
        "\n");
    return 1;
  } else if (strcmp(arg, "calculation") == 0) {
    printf(
        "Calculation Method Options:\n"
        "    --calc-method <arg>, -c <arg>          Select prayer time "
        "calculation method.\n\n"
        "Calculation Method Options:\n"
        "    Jafari            Ithna Ashari\n"
        "    Karachi           Islamic University of Sciences, Karachi\n"
        "    Isna              Islamic Society of North America (ISNA)\n"
        "    MWL               Muslim World League (MWL)\n"
        "    Makkah           Umm al-Qura, Makkah\n"
        "    Egypt             Egyptian General Authority of Survey\n"
        "    Custom            Custom Setting\n"
        "\n");
    return 1;
  } else if (strcmp(arg, "juristic") == 0) {
    printf(
        "Asr Juristic Method Options:\n"
        "    --asr-juristic-method <arg>, -a <arg>  Select Juristic method for "
        "Asr prayer time.\n\n"
        "Asr Juristic Method Options:\n"
        "    Shafii            Standard Shafii method\n"
        "    Hanafi            Hanafi method\n"
        "\n");
    return 1;
  } else if (strcmp(arg, "adjustment") == 0) {
    printf(
        "Latitude Adjustment Options:\n"
        "    --high-lats-method <arg>, -i <arg>      Select adjustment method "
        "for higher latitudes.\n"
        "\n"
        "    Dhuhr and Maghrib Adjustments:\n"
        "    --dhuhr-minutes <arg>              Minutes after mid-day to "
        "adjust Dhuhr prayer time.\n"
        "    --maghrib-minutes <arg>            Minutes after sunset to adjust "
        "Maghrib prayer time.\n"
        "    --isha-minutes <arg>               Minutes after Maghrib to "
        "adjust Isha prayer time.\n"
        "\n"
        "    Angle Options for Prayer Times:\n"
        "    --fajr-angle <arg>                 Angle for calculating Fajr "
        "prayer time.\n"
        "    --maghrib-angle <arg>              Angle for calculating Maghrib "
        "prayer time.\n"
        "    --isha-angle <arg>                 Angle for calculating Isha "
        "prayer time.\n"
        "\n"
        "High Latitude Adjustment Options:\n"
        "    MidNight          Middle of the night\n"
        "    OneSeventh        One-seventh of the night\n"
        "    AngleBased        Angle/60th of the night\n"
        "    None              No adjustment");
    return 1;
  }
  fprintf(stderr, "Unknown help Option: %s\n", arg);
  return 2;
}

void print_debug_help(PrayerTimes *prayerTimes) {
  struct tm *t = localtime(&prayerTimes->time);
  char *config_file;
  if (get_config_file(&config_file)) {
    fprintf(stderr, "error: get_config_file");
    return;
  }
  printf(
      "Prayer Notification App\nDate: %02d/%02d/%04d\nTime: "
      "%02d:%02d:%02d\nconfig_file: "
      "%s\nLocation: (%lf, "
      "%lf)\nCalculation Method: %s\nJuristic Method: %s\nAdjustment "
      "Method: %s\n",
      t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min,
      t->tm_sec, config_file, prayerTimes->longitude, prayerTimes->latitude,
      Calculation[prayerTimes->calc_method],
      Juristic[prayerTimes->asr_juristic],
      Adjusting[prayerTimes->adjust_high_lats]);
}

void print_prayer_times_help(PrayerTimes *prayerTimes) {
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

void print_next_prayer(PrayerTimes *prayerTimes) {
  double times[TIMEID_TimesCount];
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
    if (timelocal(&times_dates[timeid]) > prayerTimes->time) {
      Time diff =
          convert_time_hms(timelocal(&times_dates[timeid]) - prayerTimes->time);
      printf(
          "Upcoming Notification:\nNext Prayer: %s: %2.2d:%2.2d\nTime "
          "Remaining: %2.2d:%2.2d:%2.2d\n",
          TimeName[timeid], times_dates[timeid].tm_hour,
          times_dates[timeid].tm_min, diff.hours, diff.minutes, diff.seconds);
      return;
    }
  }

  // if the loop exited then the previous prayer is tomorrow's fajr
  // get tomorrow at 00:00:00
  time_add_day(date);

  prayerTimes->time = mktime(date);
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);
  times_dates[TIMEID_Fajr] = *date;
  times_dates[TIMEID_Fajr].tm_sec = 0;
  get_float_time_parts(times[TIMEID_Fajr], &times_dates[TIMEID_Fajr].tm_hour,
                       &times_dates[TIMEID_Fajr].tm_min);
  Time diff =
      convert_time_hms(timelocal(&times_dates[TIMEID_Fajr]) - time(NULL));
  printf(
      "Upcoming Notification:\nNext Prayer: %s: %2.2d:%2.2d\nTime "
      "Remaining: %2.2d:%2.2d:%2.2d\n",
      TimeName[TIMEID_Fajr], times_dates[TIMEID_Fajr].tm_hour,
      times_dates[TIMEID_Fajr].tm_min, diff.hours, diff.minutes, diff.seconds);
}

void print_previous_prayer(PrayerTimes *prayerTimes) {
  double times[TIMEID_TimesCount];
  prayerTimes->timezone = get_effective_timezone_time(prayerTimes->time);
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);

  struct tm times_dates[TIMEID_TimesCount];
  prayerTimes->time = time(NULL);
  struct tm *date = localtime(&prayerTimes->time);
  for (TimeID timeid = TIMEID_TimesCount - 1; timeid > 0; timeid--) {
    times_dates[timeid] = *date;
    times_dates[timeid].tm_sec = 0;
    get_float_time_parts(times[timeid], &times_dates[timeid].tm_hour,
                         &times_dates[timeid].tm_min);
    if (timelocal(&times_dates[timeid]) < prayerTimes->time) {
      Time diff =
          convert_time_hms(prayerTimes->time - timelocal(&times_dates[timeid]));
      printf(
          "Previous Prayer: %s: %2.2d:%2.2d\nTime "
          "Elapsed: %2.2d:%2.2d:%2.2d\n",
          TimeName[timeid], times_dates[timeid].tm_hour,
          times_dates[timeid].tm_min, diff.hours, diff.minutes, diff.seconds);
      return;
    }
  }
  time_sub_day(date);
  prayerTimes->time = mktime(date);
  get_prayer_times_time(prayerTimes, prayerTimes->latitude,
                        prayerTimes->longitude, prayerTimes->timezone, times);
  times_dates[TIMEID_Isha] = *date;
  get_float_time_parts(times[TIMEID_Isha], &times_dates[TIMEID_Isha].tm_hour,
                       &times_dates[TIMEID_Isha].tm_min);
  Time diff =
      convert_time_hms(time(NULL) - timelocal(&times_dates[TIMEID_Isha]));
  printf(
      "Previous Prayer: %s: %2.2d:%2.2d\nTime "
      "Elapsed: %2.2d:%2.2d:%2.2d\n",
      TimeName[TIMEID_Isha], times_dates[TIMEID_Isha].tm_hour,
      times_dates[TIMEID_Isha].tm_min, diff.hours, diff.minutes, diff.seconds);
}
