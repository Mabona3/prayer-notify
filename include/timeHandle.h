#ifndef TIMEHANDLE_H
#define TIMEHANDLE_H

#include <stdbool.h>
#include <time.h>

/* return the hour and minutes parts of the returned double times[] of
 * get_prayer_times() */
void get_float_time_parts(double times, int *hours, int *minutes);

/* convert float hours to 24h format */
void float_time_to_time24(double time, char *time24);

/* convert float hours to 12h format */
void float_time_to_time12(double time, bool no_suffix, char *time12);

/* convert float hours to 12h format with no suffix */
void float_time_to_time12ns(double time, char *time12);

/* range reduce hours to 0..23 */
double fix_hour(double a);

/* atoi but for time */
#define two_digits_format(num, time) sprintf(time, "%2.2d", num)

/* compute the difference between two times  */
double time_diff(double time1, double time2);

/* add one day to the struct used in the midtime handler. */
void time_add_day(struct tm *date);

#endif // TIMEHANDLE_H
