#ifndef JSONWRITER_H
#define JSONWRITER_H

#include "prayerTimes.h"

#include <time.h>

// Write the next_prayer_time
int write_current(struct tm *t, TimeID current);

// delete and remove the cache from the temp file
int close_current_writer();

#endif // !JSONWRITER_H
