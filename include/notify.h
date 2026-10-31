#ifndef NOTIFY_H
#define NOTIFY_H

#include <glib.h>
#include <libnotify/notify.h>
#include <pthread.h>

#include "state_def.h"

#define NOTIFICATION_NAME "Prayer Times"

// initialize the gmain context
int init_notify(AppContext *app_context);

void send_notification(AppContext *app_context);

// deinitialize the gmain context
void deinit_notify(AppContext *app_context);

// end the notification thread
void close_notify(NotificationContext *notif);

#endif  // NOTIFY_H
