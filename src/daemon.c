#include "daemon.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <bits/types/siginfo_t.h>
#include <bits/types/sigset_t.h>
#include <libnotify/notify.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "jsonReader.h"
#include "logger.h"
#include "notify.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "writer.h"

TimeID current_time = TIMEID_Fajr;

extern volatile sig_atomic_t running;

extern pthread_mutex_t notify_mutex;
extern pthread_cond_t notify_cond;

pthread_t daemon_thread_id;

void *daemon_thread(void *args);

unsigned long int daemon_init(PrayerTimes *prayerTimes) {
  if (pthread_create(&daemon_thread_id, NULL, daemon_thread,
                     (void *)prayerTimes)) {
    log_msg(LOGLEVEL_ERROR, "daemon thread couldn't be created");
    exit(1);
  }
  return daemon_thread_id;
}

void *daemon_thread(void *args) {
  PrayerTimes *prayerTimes = (PrayerTimes *)args;
  struct tm times_dates[TIMEID_TimesCount];
  double times[TIMEID_TimesCount];
  struct tm *date;

  update_times(prayerTimes, times_dates, times);

  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, SIGINT);
  sigaddset(&sigs, SIGTERM);
  sigaddset(&sigs, SIGUSR1);

  while (running) {
    prayerTimes->time = time(NULL);
    for (current_time = TIMEID_Fajr; current_time < TIMEID_TimesCount;
         ++current_time) {
      if (current_time == TIMEID_Sunset) continue;
      time_t dtime = mktime(&times_dates[current_time]) - prayerTimes->time;
      log_msg(LOGLEVEL_INFO, "%s: %d seconds", TimeName[current_time], dtime);

      if (running == RUNNING_STATE && dtime > 0) {
        log_msg(LOGLEVEL_DEBUG, "Logging current time %s",
                TimeName[current_time], dtime);

        write_current(times_dates, current_time);

        siginfo_t info;
        int ret =
            sigtimedwait(&sigs, &info, &(struct timespec){.tv_sec = dtime});
        if (ret < 0) {
          if (errno == EAGAIN)
            log_msg(LOGLEVEL_DEBUG, "TIMEDOUT from the sleep");
        } else {
          if (ret == SIGUSR1) {
            running = RELOAD_STATE;
            log_msg(LOGLEVEL_DEBUG, "Signalled out from the sleep reloading");
          } else {
            running = EXIT_STATE;
            log_msg(LOGLEVEL_DEBUG, "Signalled out from the sleep terminating");
          }
          break;
        }

        running = NOTIFY_STATE;
        log_msg(LOGLEVEL_DEBUG, "signalling...");
        pthread_cond_signal(&notify_cond);
        log_msg(LOGLEVEL_DEBUG, "waiting for the notifying...");
        while (running == NOTIFY_STATE)
          pthread_cond_wait(&notify_cond, &notify_mutex);
        log_msg(LOGLEVEL_DEBUG, "continuing...");
      }
      if (running == EXIT_STATE) break;
      prayerTimes->time = time(NULL);
    }
    if (running == EXIT_STATE) break;

    if (running == RELOAD_STATE) {
      prayerTimes->time = time(NULL);
      if (read_config(prayerTimes) == -1) {
        running = EXIT_STATE;
        break;
      }
      update_times(prayerTimes, times_dates, times);
      running = RUNNING_STATE;
    } else {
      date = localtime(&prayerTimes->time);
      time_add_day(date);

      prayerTimes->time = mktime(date);
      update_times(prayerTimes, times_dates, times);
    }
  }
  pthread_mutex_unlock(&notify_mutex);
  log_msg(LOGLEVEL_DEBUG, "daemon Signalling...");
  pthread_cond_signal(&notify_cond);
  log_msg(LOGLEVEL_DEBUG, "daemon ending...");
  return NULL;
}
