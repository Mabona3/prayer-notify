#include "notify.h"

#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "logger.h"

void init_notify() {
  if (!notify_init(NOTIFICATION_NAME)) {
    log_msg(LOGLEVEL_ERROR, "failed to init libnotify\n");
    exit(1);
  }
}

void send_notification(TimeID current_time) {
  NotifyNotification *notif;
  char notif_name[16];

  snprintf(notif_name, sizeof(notif_name), "%s Time", TimeName[current_time]);

  char *icon;
  if (get_icon_file(&icon) == EXIT_FAILURE) {
    icon = NULL;
  }

  notif = notify_notification_new("Prayer Times", notif_name, icon);

  log_msg(LOGLEVEL_DEBUG, "notifying for %s\n", TimeName[current_time]);

  if (!notify_notification_show(notif, NULL)) {
    log_msg(LOGLEVEL_ERROR, "failed to show notification!\n");
    exit(1);
  }

  if (icon) {
    free(icon);
    icon = NULL;
  }

  g_object_unref(notif);
}

void deinit_notify() {
  notify_uninit();
}
