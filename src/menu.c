#include "menu.h"
#include "config.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "version.h"

void print_help(FILE *f) {
  fputs(PROG_NAME_FRIENDLY " " APP_VERSION "\n\n", f);
  fputs(
      "Usage: " PROG_NAME " options...\n"
      "\n"
      " Options\n"
      "    --help                      -h  you're reading it\n"
      "    --option                    -o  print the options into stdout \n"
      "    --prayer                    -r  print the options into stdout \n"
      "    --next                      -n  print the next prayer into stdout \n"
      "    --previous                  -p  print the previous prayer into "
      "stdout \n"
      "    --version                   -v  prints name and version, then "
      "exits\n"
      "    --date arg                  -d  get prayer times for arbitrary "
      "date\n"
      "    --timezone arg              -z  get prayer times for arbitrary "
      "timezone\n"
      "  * --latitude arg              -t  latitude of desired location\n"
      "  * --longitude arg             -g  longitude of desired location\n"
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
      "    Jafari        Ithna Ashari\n"
      "    Karachi       University of Islamic Sciences, Karachi\n"
      "    Isna          Islamic Society of North America (ISNA)\n"
      "    Mwl           Muslim World League (MWL)\n"
      "    Makkah        Umm al-Qura, Makkah\n"
      "    Egypt         Egyptian General Authority of Survey\n"
      "    Custom        Custom Setting\n"
      "\n"
      " Possible arguments for --asr-juristic-method\n"
      "    Shafii        Shafii (standard)\n"
      "    Hanafi        Hanafi\n"
      "\n"
      " Possible arguments for --high-lats-method\n"
      "    MidNight      middle of night\n"
      "    OneSeventh    1/7th of night\n"
      "    AngleBased    angle/60th of night\n"
      "    None          No adjustment\n",
      f);
}

void print_debug_help(PrayerTimes *prayerTimes) {
  struct tm *t = localtime(&prayerTimes->time);
  const char *config_file = get_config_file();
  printf("Prayer Notification App\nDate: %02d/%02d/%04d\nTime: "
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
    int dtime = timelocal(&times_dates[timeid]) - prayerTimes->time;
    if (dtime > 0) {
      int hours = dtime / 3600;
      dtime -= hours * 3600;
      int minutes = dtime / 60;
      dtime -= minutes * 60;
      printf("Upcoming Notification:\nNext Prayer: %s: %2.2d:%2.2d\nTime "
             "Remaining: %2.2d:%2.2d:%2.2d\n",
             TimeName[timeid], times_dates[timeid].tm_hour,
             times_dates[timeid].tm_min, hours, minutes, dtime);
      break;
    }
  }
}

void print_previous_prayer(PrayerTimes *prayerTimes) {
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
  for (TimeID timeid = TIMEID_TimesCount - 1; timeid > 0; timeid--) {
    times_dates[timeid] = *date;
    times_dates[timeid].tm_sec = 0;
    get_float_time_parts(times[timeid], &times_dates[timeid].tm_hour,
                         &times_dates[timeid].tm_min);
    int dtime = timelocal(&times_dates[timeid]) - prayerTimes->time;
    if (dtime < 0) {
      dtime = -dtime;
      int hours = dtime / 3600;
      dtime -= hours * 3600;
      int minutes = dtime / 60;
      dtime -= minutes * 60;
      printf("Previous Prayer: %s: %2.2d:%2.2d\nTime "
             "Elapsed: %2.2d:%2.2d:%2.2d\n",
             TimeName[timeid], times_dates[timeid].tm_hour,
             times_dates[timeid].tm_min, hours, minutes, dtime);
      break;
    }
  }
}
