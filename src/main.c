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

#include "common.h"
#include "config.h"
#include "jsonReader.h"
#include "notify.h"
#include "timeHandle.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

// flag to exit with sigint and sigterm
static volatile sig_atomic_t running = 1;

// handler for resetting the flag to exit the main func loop
void handle_signal(int sig) {
  (void)sig;
  running = 0;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
  PrayerTimes *prayerTimes = read_config();
  if (prayerTimes == NULL) {
    free_config_file();
    return 1;
  }

  prayerTimes->time = time(NULL);

  int parse_status;
  if ((parse_status = parse_inputs(prayerTimes, argc, argv)) != 0) {
    free(prayerTimes);
    free_config_file();
    return parse_status - 1;
  }

  double times[TIMEID_TimesCount];
  struct tm times_dates[TIMEID_TimesCount];

  while (running) {
    prayerTimes->time = time(NULL);
    get_prayer_times_time(
        prayerTimes, prayerTimes->latitude, prayerTimes->longitude,
        get_effective_timezone_time(prayerTimes->time), times);

    struct tm *date = localtime(&prayerTimes->time);

    // preparing the times
    for (int i = 0; i < TIMEID_TimesCount; i++) {
      times_dates[i] = *date;
      times_dates[i].tm_sec = 0;
      get_float_time_parts(times[i], &times_dates[i].tm_hour,
                           &times_dates[i].tm_min);
    }

    TimeID timeid;
    for (timeid = TIMEID_Fajr; timeid < TIMEID_TimesCount && running;
         timeid++) {
      if (timeid ==
          TIMEID_Sunset) // Just don't want to hear the timeid of sunset as it
                         // substitute the maghrib timeid.
        continue;
      int dtime = timelocal(&times_dates[timeid]) - prayerTimes->time;
      if (dtime > 0) {
        while (dtime > 0 && running) {
          dtime = sleep(dtime);
        }

        if (running)
          send_notification(timeid);
      }
      prayerTimes->time = time(NULL);
    }
    date = localtime(&prayerTimes->time);
    time_add_day(date);
    int dtime = difftime(mktime(date), prayerTimes->time);
    while (dtime > 0 && running) {
      dtime = sleep(dtime);
    }
  }

  free(prayerTimes);
  prayerTimes = NULL;
  free_config_file();
}
