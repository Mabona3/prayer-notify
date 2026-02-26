#ifndef JSONWRITER_H
#define JSONWRITER_H

#include <stdbool.h>
#include <time.h>

// check for the temp file existence no more than one instance to run and
// work within the file and I don't want to make a complex way to handle this
bool check_temp_file();

// Write the json file into the /tmp/prayer-notify.json
int write_current(struct tm *times, int current);

// delete and remove the cache from the temp file
int close_current_writer();

#endif  // !JSONWRITER_H
