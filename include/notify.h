#ifndef NOTIFY_H
#define NOTIFY_H

#include "prayerTimes.h"

#define NOTIFICATION_NAME "Prayer Times"

// Send notification using the required prayer time id
void send_notification(TimeID current_time);

#endif // NOTIFY_H
