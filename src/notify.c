#include "notify.h"

#include <assert.h>
#include <glib.h>
#include <libnotify/notification.h>
#include <pthread.h>
#include <stdio.h>

#include "athan.h"
#include "glib-object.h"
#include "logger.h"

extern TimeID current_time;

void *notify_thread(void *args);

static void on_notification_closed(NotifyNotification *notification,
                                   __attribute__((unused)) gpointer user_data)
{
    gint ret = notify_notification_get_closed_reason(notification);
    char *reason;
    switch ((NotifyClosedReason)ret) {
        case NOTIFY_CLOSED_REASON_UNSET:
            reason = "unset";
            break;
        case NOTIFY_CLOSED_REASON_EXPIRED:
            reason = "expired";
            break;
        case NOTIFY_CLOSED_REASON_DISMISSED:
            reason = "dismissed";
            break;
        case NOTIFY_CLOSED_REASON_API_REQUEST:
            reason = "api request";
            break;
        case NOTIFY_CLOSED_REASON_UNDEFINED:
        default:
            reason = "undefined";
            break;
    }
    log_msg(LOGLEVEL_DEBUG, "Notification closed with reason code: %s", reason);
}

static void on_notification_mute(NotifyNotification *notification,
                                 __attribute__((unused)) char *action,
                                 gpointer user_data)
{
    notify_notification_show(notification, NULL);
    Athan_ctx *athan = (Athan_ctx *)user_data;
    stop_athan(athan);
}

static gboolean notification_update(gpointer data)
{
    NotificationContext *notif = (NotificationContext *)data;
    log_msg(LOGLEVEL_DEBUG, "invoking the update_show function");
    notify_notification_update(notif->notify, NOTIFICATION_NAME,
                               TimeName[notif->current_time], notif->icon);

    notify_notification_show(notif->notify, NULL);

    return G_SOURCE_REMOVE;
}

void close_notify(NotificationContext *notif)
{
    g_main_loop_quit(notif->main_loop);
}

int init_notify(AppContext *app_context)
{
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

    notify_notification_add_action(app_context->notif.notify, "mute", "mute",
                                   on_notification_mute, &app_context->athan,
                                   NULL);

    if (!app_context->notif.notify) {
        log_msg(LOGLEVEL_ERROR, "notify notification creation failed");
        return -1;
    }

    if (pthread_create(&app_context->notif.thread_id, NULL, notify_thread,
                       app_context)) {
        log_msg(LOGLEVEL_ERROR, "notify thread creation failed");
        return -1;
    }

    g_signal_connect(app_context->notif.notify, "closed",
                     G_CALLBACK(on_notification_closed),
                     ((void *)&app_context->athan));

    return 0;
}

void send_notification(AppContext *app_context)
{
    log_msg(LOGLEVEL_DEBUG, "Sending notification for %s",
            TimeName[app_context->notif.current_time]);

    while (!g_main_loop_is_running(app_context->notif.main_loop));
    g_main_context_invoke(app_context->notif.main_context, notification_update,
                          &app_context->notif);
    g_main_context_invoke(app_context->notif.main_context,
                          (GSourceFunc)start_athan, &app_context->athan);
}

void deinit_notify(AppContext *app_context)
{
    log_msg(LOGLEVEL_DEBUG, "destroying notification objects");
    notify_uninit();
    g_clear_object(&app_context->notif.notify);
    g_main_context_unref(app_context->notif.main_context);
    g_main_loop_unref(app_context->notif.main_loop);
}

void *notify_thread(void *args)
{
    AppContext *app_context = (AppContext *)args;
    g_main_loop_run(app_context->notif.main_loop);
    return NULL;
}
