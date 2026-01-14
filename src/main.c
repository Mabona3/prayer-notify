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

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "jsonReader.h"
#include "notify.h"
#include "prayerTimes.h"
#include "timeHandle.h"
#include "writer.h"

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
    return 1;
  }

  prayerTimes->time = time(NULL);

  int parse_status;
  if ((parse_status = parse_inputs(prayerTimes, argc, argv)) != 0) {
    free(prayerTimes);
    return parse_status - 1;
  }

  struct tm times_dates[TIMEID_TimesCount];
  double times[TIMEID_TimesCount];
  struct tm *date;
  TimeID timeid;

  update_times(prayerTimes, times_dates, times);

  while (running) {
    prayerTimes->time = time(NULL);

    for (timeid = TIMEID_Fajr; timeid < TIMEID_TimesCount && running;
         timeid++) {
      if (timeid ==
          TIMEID_Sunset)  // Just don't want to hear the timeid of sunset
                          // as it substitute the maghrib timeid.
        continue;
      int dtime = timelocal(&times_dates[timeid]) - prayerTimes->time;
      if (dtime > 0) {
        write_current(&times_dates[timeid], timeid);
        while (dtime > 0 && running) dtime = sleep(dtime);

        if (running) send_notification(timeid);
      }
      prayerTimes->time = time(NULL);
    }
    date = localtime(&prayerTimes->time);
    time_add_day(date);

    prayerTimes->time = mktime(date);
    update_times(prayerTimes, times_dates, times);
    write_current(&times_dates[TIMEID_Fajr], TIMEID_Fajr);

    int dtime = difftime(mktime(date), prayerTimes->time);
    while (dtime > 0 && running) dtime = sleep(dtime);
  }

  free(prayerTimes);
  prayerTimes = NULL;
  close_current_writer();
}
