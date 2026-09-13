#include "notify.h"

#include <assert.h>
#include <glib.h>
#include <libnotify/notification.h>
#include <pthread.h>
#include <stdio.h>

#include "logger.h"

extern TimeID current_time;

void *notify_thread(void *args);

static void on_notification_closed(NotifyNotification *notification,
                                   __attribute__((unused)) char *action,
                                   __attribute__((unused)) gpointer user_data) {
    gint ret = notify_notification_get_closed_reason(notification);
    char *reason;
    switch ((NotifyClosedReason)ret) {
        case NOTIFY_CLOSED_REASON_UNSET: reason = "unset"; break;
        case NOTIFY_CLOSED_REASON_EXPIRED: reason = "expired"; break;
        case NOTIFY_CLOSED_REASON_DISMISSED: reason = "dismissed"; break;
        case NOTIFY_CLOSED_REASON_API_REQUEST: reason = "api request"; break;
        case NOTIFY_CLOSED_REASON_UNDEFINED:
        default: reason = "undefined"; break;
    }
    log_msg(LOGLEVEL_DEBUG, "Notification closed with reason code: %s", reason);
}

static gboolean notification_update(gpointer data) {
    NotificationContext *notif = (NotificationContext *)data;
    log_msg(LOGLEVEL_DEBUG, "invoking the update_show function");
    notify_notification_update(notif->notify, NOTIFICATION_NAME,
                               TimeName[notif->current_time], notif->icon);

    notify_notification_show(notif->notify, NULL);

    return G_SOURCE_REMOVE;
}

void close_notify(NotificationContext *notif) {
    g_main_loop_quit(notif->main_loop);
}

unsigned long int init_notify(AppContext *app_context) {
    pthread_t notify_thread_id;
    if (!notify_init(NOTIFICATION_NAME)) {
        log_msg(LOGLEVEL_ERROR, "failed to init libnotify");
        return -1;
    }

    app_context->notif.main_loop = g_main_loop_new(NULL, false);
    assert(app_context->notif.main_loop);
    if (!app_context->notif.main_loop) {
        log_msg(LOGLEVEL_ERROR, "error creating main loop");
        return -1;
    }

    app_context->notif.main_context =
        g_main_loop_get_context(app_context->notif.main_loop);
    assert(app_context->notif.main_context);
    if (!app_context->notif.main_context) {
        log_msg(LOGLEVEL_ERROR, "error creating main context");
        return -1;
    }

    app_context->notif.notify =
        notify_notification_new(NOTIFICATION_NAME, NULL, NULL);
    if (!app_context->notif.notify) {
        log_msg(LOGLEVEL_ERROR, "notify notification creation failed");
        return -1;
    }

    if (pthread_create(&notify_thread_id, NULL, notify_thread, app_context)) {
        log_msg(LOGLEVEL_ERROR, "notify thread creation failed");
        return -1;
    }

    g_signal_connect(app_context->notif.notify, "closed",
                     G_CALLBACK(on_notification_closed), NULL);

    return notify_thread_id;
}

void send_notification(NotificationContext *notif) {
    log_msg(LOGLEVEL_DEBUG, "Sending notification for %s",
            TimeName[notif->current_time]);

    while (!g_main_loop_is_running(notif->main_loop));
    g_main_context_invoke(notif->main_context, notification_update, notif);
}

void deinit_notify(AppContext *app_context) {
    log_msg(LOGLEVEL_DEBUG, "Destroying Notification");
    g_object_unref(app_context->notif.notify);
    notify_uninit();
}

void *notify_thread(void *args) {
    AppContext *app_context = (AppContext *)args;
    g_main_loop_run(app_context->notif.main_loop);
    return NULL;
}
