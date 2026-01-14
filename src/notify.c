#include "notify.h"

#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

void send_notification(TimeID current_time) {
  NotifyNotification *notif;

  if (!notify_init(NOTIFICATION_NAME)) {
    fprintf(stderr, "failed to init libnotify\n");
    exit(1);
  }

  char notif_name[16];

  snprintf(notif_name, sizeof(notif_name), "%s Time", TimeName[current_time]);

  char *icon;
  if (get_icon_file(&icon) == EXIT_FAILURE) {
    icon = NULL;
  }

  notif = notify_notification_new("Prayer Times", notif_name, icon);
  if (icon) {
    free(icon);
    icon = NULL;
  }

  if (!notify_notification_show(notif, NULL)) {
    fprintf(stderr, "failed to show notification!\n");
    exit(1);
  }

  g_object_unref(notif);

  notify_uninit();
}
