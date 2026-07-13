#ifndef DAEMON_H
#define DAEMON_H

#include "prayerTimes.h"

typedef enum {
  EXIT_STATE,
  RUNNING_STATE,
  RELOAD_STATE,
} DaemonState;

void main_func(PrayerTimes *prayerTimes);

#endif  // !DAEMON_H
