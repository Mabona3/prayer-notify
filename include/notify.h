#ifndef NOTIFY_H
#define NOTIFY_H

#include "prayerTimes.h"

#include <pthread.h>

extern pthread_mutex_t notify_mutex;
extern TimeID notify_current;

// initialize the gmain context
void *send_notification_daemon(void *arg);

#endif  // NOTIFY_H
