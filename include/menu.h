#include "prayerTimes.h"

#define PROG_NAME_FRIENDLY "prayer-notify"
#define PROG_NAME "prayer-notify"

// print the prayer times
void print_prayer_times_help(PrayerTimes *prayerTimes);

// print the debug options.
void print_debug_help(PrayerTimes *prayerTimes);

// print the help msg to the stdout.
int print_help(const char *arg);

void print_next_prayer(PrayerTimes *prayerTimes);

void print_previous_prayer(PrayerTimes *prayerTimes);
