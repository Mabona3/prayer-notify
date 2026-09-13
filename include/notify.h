#ifndef NOTIFY_H
#define NOTIFY_H

#include <glib.h>
#include <libnotify/notify.h>
#include <pthread.h>

#include "state_def.h"

#define NOTIFICATION_NAME "Prayer Times"

// initialize the gmain context
unsigned long int init_notify(AppContext *app_context);

void send_notification(NotificationContext *notif);

// deinitialize the gmain context
void deinit_notify(AppContext *app_context);

// end the notification thread
void close_notify(NotificationContext *notif);

#endif  // NOTIFY_H
