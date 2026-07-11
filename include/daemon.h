#ifndef DAEMON_H
#define DAEMON_H

void *main_func(void *arg);

typedef enum {
  EXIT_STATE,
  RUNNING_STATE,
  RELOAD_STATE,
} DaemonState;

#endif  // !DAEMON_H
