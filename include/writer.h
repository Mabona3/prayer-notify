#ifndef JSONWRITER_H
#define JSONWRITER_H

#include <stdio.h>
#include <time.h>

#include "prayerTimes.h"

// check for the temp file existence no more than one instance to run and
// work within the file this shall return the file descriptor of the temp
FILE *check_temp_file(void);

// Write the json file into the /tmp/prayer-notify.json
int write_current(FILE *file, struct tm *times, TimeID current);

// delete and remove the cache from the temp file
void close_current_writer(FILE *file);

#endif  // !JSONWRITER_H
