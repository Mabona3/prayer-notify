#ifndef NOTIFY_H
#define NOTIFY_H

#include "prayerTimes.h"

#define NOTIFICATION_NAME "Prayer Times"

// initialize the gmain context
void init_notify();

// Send notification using the required prayer time id
void send_notification(TimeID current_time);

// deinitialize the gmain context
void deinit_notify();

#endif  // NOTIFY_H
