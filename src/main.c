/*
 * The prayer-notify application.
 *
 * Copyright (C) 2025 Mabona <mazenkhaled2003@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <unistd.h>

#include "arena.h"
#include "config.h"
#include "daemon.h"
#include "jsonReader.h"
#include "logger.h"
#include "notify.h"
#include "option.h"
#include "writer.h"

volatile sig_atomic_t running = RUNNING_STATE;

int main(int argc, char *argv[]) {
  init_logger();
  if (arena_create()) {
    log_msg(LOGLEVEL_ERROR, "arena_create() failed\n");
    exit(1);
  }

  // block all signals
  sigset_t set;
  sigemptyset(&set);

  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGUSR1);

  pthread_sigmask(SIG_BLOCK, &set, NULL);

  PrayerTimes prayerTimes = create_prayer_times(
      CALCULATION_Jafari, JURISTIC_Shafi, ADJUSTING_MidNight, 0);

  if (read_config(&prayerTimes)) {
    return EXIT_FAILURE;
  }

  prayerTimes.time = time(NULL);

  int parse_status;
  if ((parse_status = parse_inputs(&prayerTimes, argc, argv)) != 0) {
    return parse_status == -1;
  }

  if (check_temp_file()) {
    log_msg(LOGLEVEL_ERROR, "Another instance is running\n");
    return 1;
  }

  char *icon = NULL;

  ScratchArena scratch;
  arena_scratch_push(&scratch);

  if (get_icon_file(&scratch, &icon)) {
    log_msg(LOGLEVEL_ERROR, "get_icon_file failed!");
    return 1;
  }

  pthread_t notify_thread_id = init_notify(&icon);
  pthread_t daemon_thread_id = daemon_init(&prayerTimes);

  pthread_join(notify_thread_id, NULL);
  pthread_join(daemon_thread_id, NULL);

  atexit(deinit_notify);
  atexit(arena_destroy);

  arena_scratch_pop(&scratch);

  close_current_writer();
  log_msg(LOGLEVEL_DEBUG, "Destroying synchronization primitaves...");
  pthread_mutex_destroy(&notify_mutex);
  pthread_cond_destroy(&notify_cond);
  return 0;
}
