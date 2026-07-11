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
#include <stdlib.h>
#include <unistd.h>

#include "arena.h"
#include "daemon.h"
#include "jsonReader.h"
#include "logger.h"
#include "notify.h"
#include "option.h"
#include "writer.h"

// handler for resetting the flag to exit the main func loop
void handle_signal(int sig) {
  (void)sig;
  exit(0);
}

int main(int argc, char *argv[]) {
  init_logger();
  if (arena_create()) {
    log_msg(LOGLEVEL_ERROR, "mmap() failed\n");
    exit(1);
  }
  atexit(arena_destroy);
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
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
    log_msg(LOGLEVEL_ERROR, "Another instance is running");
    return 1;
  }

  atexit(close_current_writer);
  pthread_t notify_thread;
  pthread_create(&notify_thread, NULL, send_notification_daemon, NULL);
  main_func(&prayerTimes);

  return 0;
}
