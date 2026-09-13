#include "app.h"

#include <signal.h>

#include "arena.h"
#include "config.h"
#include "jsonReader.h"
#include "logger.h"
#include "notify.h"
#include "option.h"
#include "sched_task.h"
#include "writer.h"

int app_init(AppContext *app_context, int argc, char *argv[]) {
    init_logger();
    if (arena_create()) {
        log_msg(LOGLEVEL_ERROR, "arena_create() failed\n");
        return -1;
    }

    // block all signals
    sigset_t set;
    sigemptyset(&set);

    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGUSR1);

    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // This should not work right ...
    app_context->sched.prayer_times = create_prayer_times(
        CALCULATION_Jafari, JURISTIC_Shafi, ADJUSTING_MidNight, 0);

    if (read_config(&app_context->sched.prayer_times)) {
        return -1;
    }

    app_context->sched.prayer_times.time = time(NULL);

    int parse_status;
    if ((parse_status =
             parse_inputs(&app_context->sched.prayer_times, argc, argv)) != 0) {
        return parse_status;
    }

    if (check_temp_file()) {
        log_msg(LOGLEVEL_ERROR, "Another instance is running\n");
        return 1;
    }

    arena_scratch_push(&app_context->scratch_arena);

    if (get_icon_file(&app_context->scratch_arena, &app_context->notif.icon)) {
        log_msg(LOGLEVEL_ERROR, "get_icon_file failed!");
        return -1;
    }

    app_context->notif.thread_id = init_notify(app_context);
    app_context->sched.thread_id = sched_init(app_context);

    return 0;
}

void app_run(AppContext *app_context) {
    pthread_join(app_context->notif.thread_id, NULL);
    pthread_join(app_context->sched.thread_id, NULL);
}

void app_quit(AppContext *app_context) {
    close_current_writer();
    deinit_notify(app_context);
    arena_destroy();
    arena_scratch_pop(&app_context->scratch_arena);
}
