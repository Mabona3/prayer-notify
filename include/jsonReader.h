#ifndef JSONREADER_H
#define JSONREADER_H

#include "prayerTimes.h"

// Takes the config file and return JsonData.
PrayerTimes *read_config();

// return the default prayertimes struct
PrayerTimes *get_default_config();

#endif  // JSONREADER_H
