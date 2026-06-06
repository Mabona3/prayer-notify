#include "daemon.h"

#include <unistd.h>

#include "logger.h"
#include "notify.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "writer.h"

void main_func(PrayerTimes *prayerTimes) {
  struct tm times_dates[TIMEID_TimesCount];
  double times[TIMEID_TimesCount];
  struct tm *date;

  update_times(prayerTimes, times_dates, times);

  while (1) {
    prayerTimes->time = time(NULL);
    for (TimeID timeid = TIMEID_Fajr; timeid < TIMEID_TimesCount; ++timeid) {
      if (timeid == TIMEID_Sunset) continue;
      time_t dtime = mktime(&times_dates[timeid]) - prayerTimes->time;
      log_msg(LOGLEVEL_INFO, "%s is from %d seconds", TimeName[timeid], dtime);
      if (dtime > 0) {
        write_current(times_dates, timeid);
        while (dtime > 0) dtime = sleep(dtime);
        send_notification(timeid);
      }
      prayerTimes->time = time(NULL);
    }
    date = localtime(&prayerTimes->time);
    time_add_day(date);

    prayerTimes->time = mktime(date);
    update_times(prayerTimes, times_dates, times);
  }
}
