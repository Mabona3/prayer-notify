#ifndef STATE_DEF_H
#define STATE_DEF_H

#include <glib.h>
#include <libnotify/notification.h>

#include "arena.h"
#include "prayerTimes.h"

typedef struct {
    char *icon;
    pthread_t thread_id;
    TimeID current_time;
    GMainLoop *main_loop;
    GMainContext *main_context;
    NotifyNotification *notify;
} NotificationContext;

typedef enum {
    EXIT_STATE,
    RUNNING_STATE,
    RELOAD_STATE,
} AppState;

typedef struct {
    pthread_t thread_id;
    PrayerTimes prayer_times;
    AppState state;
} SchedContext;

typedef struct {
    NotificationContext notif;
    SchedContext sched;
    ScratchArena scratch_arena;
} AppContext;

#endif  // STATE_DEF_H
