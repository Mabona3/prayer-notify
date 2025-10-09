#ifndef COMMON_H
#define COMMON_H

#include "prayerTimes.h"

/*
 * parse the args to the main file.
 * @params prayerTimes PrayerTimes*
 * @params argc int the main argc
 * @params argv char *[]
 * @return 0 if the main has to continue 1 otherwise 2 if an error occured
 */
int parse_inputs(PrayerTimes *prayerTimes, int argc, char *argv[]);

#endif // COMMON_H
