#include "notify.h"

#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "prayerTimes.h"

#define NOTIFICATION_NAME "Prayer Times"

void *send_notification_daemon(void *arg) {
  (void)arg;
  NotifyNotification *notify = NULL;
  TimeID current_time        = 0;
  char *icon                 = NULL;
  if (!notify_init(NOTIFICATION_NAME)) {
    log_msg(LOGLEVEL_ERROR, "failed to init libnotify");
    exit(1);
  }

  notify = notify_notification_new("Prayer Times", NULL, NULL);

  char notif_name[16];

  if (current_time >= TIMEID_TimesCount) {
    log_msg(LOGLEVEL_ERROR, "TimeID is not recognized!\n");
    exit(1);
  }

  snprintf(notif_name, sizeof(notif_name), "%s Time", TimeName[current_time]);

  notify_notification_update(notify, "Prayer Times", notif_name, icon);

  GError *gError;
  if (!notify_notification_show(notify, &gError)) {
    log_msg(LOGLEVEL_ERROR, "failed to show notification '%s'",
            gError->message);
    exit(1);
  }

  g_object_unref(notify);
  notify_uninit();
  return NULL;
}
