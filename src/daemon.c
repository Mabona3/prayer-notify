#include "daemon.h"

#include <pthread.h>
#include <unistd.h>

#include "arena.h"
#include "config.h"
#include "jsonReader.h"
#include "logger.h"
#include "notify.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "writer.h"

extern int running;

void main_func(PrayerTimes *prayerTimes) {
  struct tm times_dates[TIMEID_TimesCount];
  double times[TIMEID_TimesCount];
  struct tm *date;

  update_times(prayerTimes, times_dates, times);

  ScratchArena scratch;
  arena_scratch_push(&scratch);

  char *icon = NULL;
  get_icon_file(&scratch, &icon);

  log_msg(LOGLEVEL_INFO, "acquiring mutex\n");
  pthread_mutex_lock(&notify_mutex);
  while (running != EXIT_STATE) {
    prayerTimes->time = time(NULL);
    for (notify_current = TIMEID_Fajr; notify_current < TIMEID_TimesCount;
         ++notify_current) {
      if (notify_current == TIMEID_Sunset) continue;
      time_t dtime = mktime(&times_dates[notify_current]) - prayerTimes->time;
      log_msg(LOGLEVEL_INFO, "%s is from %d seconds\n",
              TimeName[notify_current], dtime);
      if (running == RUNNING_STATE && dtime > 0) {
        write_current(times_dates, notify_current);
        // while (running == RUNNING_STATE && dtime > 0) dtime = sleep(dtime);
        if (running == RUNNING_STATE) {
          pthread_mutex_unlock(&notify_mutex);
          log_msg(LOGLEVEL_INFO, "waiting for the notification\n");
          pthread_mutex_lock(&notify_mutex);
          log_msg(LOGLEVEL_INFO, "notified\n");
        } else
          break;
      }
      prayerTimes->time = time(NULL);
    }
    if (running == EXIT_STATE) return;

    if (running == RELOAD_STATE) {
      prayerTimes->time = time(NULL);
      read_config(prayerTimes);
      update_times(prayerTimes, times_dates, times);
      running = RUNNING_STATE;
    } else {
      date = localtime(&prayerTimes->time);
      time_add_day(date);

      prayerTimes->time = mktime(date);
      update_times(prayerTimes, times_dates, times);
    }
  }
  arena_scratch_push(&scratch);
}
