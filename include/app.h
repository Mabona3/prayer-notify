#ifndef APP_H
#define APP_H

#include <glib.h>
#include <libnotify/notification.h>
#include <pthread.h>

#include "state_def.h"

int app_init(AppContext *app_context, int argc, char *argvh[]);
void app_run(AppContext *app_context);
void app_quit(AppContext *app_context);

#endif  // APP_H
