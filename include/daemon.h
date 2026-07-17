#ifndef DAEMON_H
#define DAEMON_H

#include "prayerTimes.h"

extern TimeID current_time;

unsigned long int daemon_init(PrayerTimes *prayerTimes);

typedef enum {
  EXIT_STATE,
  RUNNING_STATE,
  RELOAD_STATE,
  NOTIFY_STATE,
} DaemonState;

#endif  // !DAEMON_H
