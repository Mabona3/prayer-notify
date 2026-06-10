#ifndef DAEMON_H
#define DAEMON_H

#include "prayerTimes.h"

void main_func(PrayerTimes *prayerTimes);

typedef enum {
  EXIT_STATE,
  RUNNING_STATE,
  RELOAD_STATE,
} DaemonState;

#endif  // !DAEMON_H
