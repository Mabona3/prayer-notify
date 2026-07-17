#include "notify.h"

#include <libnotify/notify.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "daemon.h"
#include "logger.h"
#include "prayerTimes.h"

pthread_t notify_thread;
extern volatile sig_atomic_t running;
extern TimeID current_time;

pthread_mutex_t notify_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notify_cond   = PTHREAD_COND_INITIALIZER;

static NotifyNotification *notify;

void *send_notification(void *args);

unsigned long int init_notify(char **icon) {
  if (!notify_init(NOTIFICATION_NAME)) {
    log_msg(LOGLEVEL_ERROR, "failed to init libnotify");
    exit(1);
  }
  notify = notify_notification_new("Prayer Times", NULL, NULL);
  if (pthread_create(&notify_thread, NULL, send_notification,
                     icon ? icon : NULL)) {
    log_msg(LOGLEVEL_ERROR, "notify thread creation failed");
    exit(1);
  }

  return notify_thread;
}

void *send_notification(void *args) {
  char *icon = NULL;
  if (args != NULL) {
    icon = *((char **)args);
    log_msg(LOGLEVEL_INFO, "Icon set to %s", icon);
  }

  log_msg(LOGLEVEL_DEBUG, "Notification thread running...");
  pthread_mutex_lock(&notify_mutex);
  while (running != EXIT_STATE) {
    pthread_cond_wait(&notify_cond, &notify_mutex);
    log_msg(LOGLEVEL_DEBUG, "unlocked condition");
    if (running == EXIT_STATE) {
      log_msg(LOGLEVEL_DEBUG, "notify thread exiting...");
      break;
    }
    char notify_name[16];
    int current = current_time;

    if (current >= TIMEID_TimesCount) {
      log_msg(LOGLEVEL_ERROR, "TimeID is not recognized!\n");
      running = EXIT_STATE;
    }
    log_msg(LOGLEVEL_DEBUG, "unlocked mutex");
    running = RUNNING_STATE;
    pthread_cond_signal(&notify_cond);

    snprintf(notify_name, sizeof(notify_name), "%s Time", TimeName[current]);

    log_msg(LOGLEVEL_DEBUG, "sending notification for %s", notify_name);

    notify_notification_update(notify, "Prayer Times", notify_name, icon);

    GError *gError = NULL;
    if (!notify_notification_show(notify, &gError)) {
      log_msg(LOGLEVEL_ERROR, "Failed to show notification '%s'",
              gError->message);
      running = EXIT_STATE;
      break;
    }
  }
  running = EXIT_STATE;
  return NULL;
}

void deinit_notify() {
  log_msg(LOGLEVEL_DEBUG, "Destroying Notification");
  g_object_unref(notify);
  notify_uninit();
}
