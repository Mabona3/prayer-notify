#include "notify.h"

#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "glib.h"
#include "logger.h"

static NotifyNotification *notify;
static char *icon = NULL;

void init_notify() {
  if (!notify_init(NOTIFICATION_NAME)) {
    log_msg(LOGLEVEL_ERROR, "failed to init libnotify");
    exit(1);
  }

  if (get_icon_file(&icon) == EXIT_FAILURE) {
    icon = NULL;
  }

  log_msg(LOGLEVEL_INFO, "Using the icon '%s'", icon);
  notify = notify_notification_new("Prayer Times", NULL, icon);
}

void send_notification(TimeID current_time) {
  char notif_name[16];

  snprintf(notif_name, sizeof(notif_name), "%s Time", TimeName[current_time]);

  notify_notification_update(notify, "Prayer Times", notif_name, icon);
  log_msg(LOGLEVEL_INFO, "Using the icon '%s'", icon);

  GError *gError;
  if (!notify_notification_show(notify, &gError)) {
    log_msg(LOGLEVEL_ERROR, "failed to show notification '%s'",
            gError->message);
    exit(1);
  }
}

void deinit_notify() {
  if (icon) {
    free(icon);
    icon = NULL;
  }
  g_object_unref(notify);
  notify_uninit();
}
