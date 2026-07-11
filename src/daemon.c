#include "daemon.h"

#include <unistd.h>

#include "arena.h"
#include "config.h"
#include "logger.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "writer.h"

void *main_func(void *arg) {
  PrayerTimes *prayerTimes = (PrayerTimes *)arg;
  struct tm times_dates[TIMEID_TimesCount];
  double times[TIMEID_TimesCount];
  struct tm *date;

  update_times(prayerTimes, times_dates, times);

  ScratchArena scratch;
  arena_scratch_push(&scratch);

  char *icon = NULL;
  get_icon_file(&scratch, &icon);

  while (1) {
    prayerTimes->time = time(NULL);
    for (TimeID timeid = TIMEID_Fajr; timeid < TIMEID_TimesCount; ++timeid) {
      if (timeid == TIMEID_Sunset) continue;
      time_t dtime = mktime(&times_dates[timeid]) - prayerTimes->time;
      log_msg(LOGLEVEL_INFO, "%s is from %d seconds", TimeName[timeid], dtime);
      if (dtime > 0) {
        write_current(times_dates, timeid);
        while (dtime > 0) dtime = sleep(dtime);
      }
      prayerTimes->time = time(NULL);
    }
    date = localtime(&prayerTimes->time);
    time_add_day(date);

    prayerTimes->time = mktime(date);
    update_times(prayerTimes, times_dates, times);
  }
  arena_scratch_push(&scratch);
}
