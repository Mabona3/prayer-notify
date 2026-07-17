#ifndef NOTIFY_H
#define NOTIFY_H

#include <pthread.h>

#define NOTIFICATION_NAME "Prayer Times"

extern pthread_mutex_t notify_mutex;
extern pthread_cond_t notify_cond;

// initialize the gmain context
unsigned long int init_notify(char **icon);

// deinitialize the gmain context
void deinit_notify();

#endif  // NOTIFY_H
