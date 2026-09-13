#include "sched_task.h"

#include "jsonReader.h"
#include "logger.h"
#include "notify.h"
#include "state_def.h"
#include "timeHandle.h"
#include "writer.h"

void *sched_thread(void *args);

unsigned long int sched_init(AppContext *app_context) {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, sched_thread, (void *)app_context)) {
        log_msg(LOGLEVEL_ERROR, "sched thread couldn't be created");
        exit(1);
    }
    return thread_id;
}

void *sched_thread(void *args) {
    AppContext *app_context = (AppContext *)args;
    struct tm times_dates[TIMEID_TimesCount];
    double times[TIMEID_TimesCount];
    struct tm *date;

    update_times(&app_context->sched.prayer_times, times_dates, times);

    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGUSR1);

    app_context->sched.state = RUNNING_STATE;
    while (app_context->sched.state) {
        app_context->sched.prayer_times.time = time(NULL);
        for (TimeID current_time = TIMEID_Fajr;
             current_time < TIMEID_TimesCount; ++current_time) {
            if (current_time == TIMEID_Sunset) continue;
            time_t dtime = mktime(&times_dates[current_time]) -
                           app_context->sched.prayer_times.time;
            log_msg(LOGLEVEL_INFO, "%s: %ld seconds", TimeName[current_time],
                    dtime);
            if (app_context->sched.state == RUNNING_STATE && dtime > 0) {
                log_msg(LOGLEVEL_DEBUG, "Logging current time %s, %ld",
                        TimeName[current_time], dtime);

                write_current(times_dates, current_time);

                siginfo_t info;
                int ret = sigtimedwait(&sigs, &info,
                                       &(struct timespec){.tv_sec = dtime});
                if (ret < 0) {
                    if (errno == EAGAIN) {
                        log_msg(LOGLEVEL_DEBUG, "TIMEDOUT from the sleep");
                        app_context->notif.current_time = current_time;
                        send_notification(&app_context->notif);
                    } else {
                        app_context->sched.state = EXIT_STATE;
                        log_msg(LOGLEVEL_ERROR, "sigtimedwait error");
                    }
                } else {
                    if (ret == SIGUSR1) {
                        app_context->sched.state = RELOAD_STATE;
                        log_msg(LOGLEVEL_DEBUG,
                                "Signalled out from the sleep reloading");
                    } else {
                        app_context->sched.state = EXIT_STATE;
                        log_msg(LOGLEVEL_DEBUG,
                                "Signalled out from the sleep terminating");
                    }
                    break;
                }

                app_context->sched.prayer_times.time = time(NULL);
            }
            if (app_context->sched.state == EXIT_STATE) break;
            app_context->sched.prayer_times.time = time(NULL);
        }

        // TODO: clean this up
        if (app_context->sched.state == RELOAD_STATE) {
            app_context->sched.prayer_times.time = time(NULL);
            if (read_config(&app_context->sched.prayer_times) == -1) {
                app_context->sched.state = EXIT_STATE;
                break;
            }
            update_times(&app_context->sched.prayer_times, times_dates, times);
            app_context->sched.state = RUNNING_STATE;
        } else if (app_context->sched.state == EXIT_STATE) {
            break;
        } else {
            date = localtime(&app_context->sched.prayer_times.time);
            time_add_day(date);

            app_context->sched.prayer_times.time = mktime(date);
            update_times(&app_context->sched.prayer_times, times_dates, times);
        }
    }

    close_notify(&app_context->notif);
    return NULL;
}
