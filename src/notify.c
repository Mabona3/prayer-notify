#include "notify.h"

#include <libnotify/notify.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "logger.h"

#define NOTIFICATION_NAME "Prayer Times"

pthread_mutex_t notify_mutex = PTHREAD_MUTEX_INITIALIZER;
TimeID notify_current        = -1;

extern int running;

void *send_notification_daemon(void *arg) {
  (void)arg;
  NotifyNotification *notify = NULL;
  char *icon                 = NULL;
  if (!notify_init(NOTIFICATION_NAME)) {
    log_msg(LOGLEVEL_ERROR, "failed to init libnotify");
    exit(1);
  }

  notify = notify_notification_new("Prayer Times", NULL, NULL);

  char notif_name[16];

  while (running) {
    log_msg(LOGLEVEL_INFO, "waiting for the mutex\n");
    pthread_mutex_lock(&notify_mutex);
    if (notify_current >= TIMEID_TimesCount) {
      log_msg(LOGLEVEL_ERROR, "TimeID is not recognized!\n");
      exit(1);
    }

    snprintf(notif_name, sizeof(notif_name), "%s Time",
             TimeName[notify_current]);

    notify_notification_update(notify, "Prayer Times", notif_name, icon);

    if (!notify_notification_show(notify, NULL)) {
      exit(1);
    }
    pthread_mutex_unlock(&notify_mutex);
  }

  pthread_mutex_destroy(&notify_mutex);
  g_object_unref(notify);
  notify_uninit();
  return NULL;
}
