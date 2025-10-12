#include "timeHandle.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

/* convert float hours to 24h format */
void float_time_to_time24(double time, char *time24) {
  if (isnan(time)) {
    sprintf(time24, "NAN");
    return;
  }
  int hours, minutes;
  get_float_time_parts(time, &hours, &minutes);
  sprintf(time24, "%2.2d:%2.2d", hours, minutes);
}

void float_time_to_time12(double time, bool no_suffix, char *time12) {
  if (isnan(time)) {
    sprintf(time12, "NAN");
    return;
  }

  int hours, minutes;
  get_float_time_parts(time, &hours, &minutes);
  const char *suffix = hours >= 12 ? " PM" : " AM";
  hours = (hours + 12 - 1) % 12 + 1;
  sprintf(time12, "%d:%2.2d%s", hours, minutes, no_suffix ? suffix : "");
}

void get_float_time_parts(double time, int *hours, int *minutes) {
  time = fix_hour(time + 0.5 / 60); // add 0.5 minutes to round
  *hours = floor(time);
  *minutes = floor((time - *hours) * 60);
}

double fix_hour(double a) {
  a = a - 24.0 * floor(a / 24.0);
  a = a < 0.0 ? a + 24.0 : a;
  return a;
}

inline double time_diff(double time1, double time2) {
  return fix_hour(time2 - time1);
}

inline void time_add_day(struct tm *date) {
  ++date->tm_mday;
  mktime(date);
  date->tm_hour = 0;
  date->tm_min = 0;
  date->tm_sec = 0;
}

/* remove one day to the struct used in the midtime handler. */
inline void time_sub_day(struct tm *date) {
  --date->tm_mday;
  mktime(date);
  date->tm_hour = 0;
  date->tm_min = 0;
  date->tm_sec = 0;
}

inline Time convert_time_hms(int time) {
  Time result;
  result.hours = time / 3600;
  time -= result.hours * 3600;
  result.minutes = time / 60;
  result.seconds = time - result.minutes * 60;
  return result;
}
