#include "prayerTimes.h"

#include <math.h>
#include <stdlib.h>

#include "timeHandle.h"

MethodConfig create_method_config(double fajr_angle, bool maghrib_is_minutes,
                                  double maghrib_value, bool isha_is_minutes,
                                  double isha_value) {
  MethodConfig config;
  config = (MethodConfig){fajr_angle, maghrib_is_minutes, maghrib_value,
                          isha_is_minutes, isha_value};
  return config;
}

PrayerTimes *create_prayer_times(CalculationMethod calc_method,
                                 JuristicMethod asr_juristic,
                                 AdjustingMethod adjust_high_lats,
                                 double dhuhr_minutes) {
  PrayerTimes *prayerTimes = malloc(sizeof(PrayerTimes));
  *prayerTimes = (PrayerTimes){
      .calc_method = calc_method,    // caculation method
      .asr_juristic = asr_juristic,  // Juristic method for Asr
      .adjust_high_lats =
          adjust_high_lats,            // adjusting method for higher latitudes
      .dhuhr_minutes = dhuhr_minutes,  // minutes after mid-day for Dhuhr
  };

  // Actual Constants
  prayerTimes->method_params[CALCULATION_Jafari] =
      create_method_config(16.0, false, 4.0, false, 14.0);  // Jafari
  prayerTimes->method_params[CALCULATION_Karachi] =
      create_method_config(18.0, true, 0.0, false, 18.0);  // Karachi
  prayerTimes->method_params[CALCULATION_ISNA] =
      create_method_config(15.0, true, 0.0, false, 15.0);  // ISNA
  prayerTimes->method_params[CALCULATION_MWL] =
      create_method_config(18.0, true, 0.0, false, 17.0);  // MWL
  prayerTimes->method_params[CALCULATION_Makkah] =
      create_method_config(19.0, true, 0.0, true, 90.0);  // Makkah
  prayerTimes->method_params[CALCULATION_Egypt] =
      create_method_config(19.5, true, 0.0, false, 17.5);  // Egypt
  prayerTimes->method_params[CALCULATION_Custom] =
      create_method_config(18.0, true, 0.0, false, 17.0);  // Custom

  return prayerTimes;
}

void get_prayer_times_date(PrayerTimes *prayerTimes, int year, int month,
                           int day, double latitude, double longitude,
                           double timezone, double times[]) {
  prayerTimes->longitude = longitude;
  prayerTimes->latitude = latitude;
  prayerTimes->timezone = timezone;
  prayerTimes->julian_date = get_julian_date(year, month, day) -
                             prayerTimes->longitude / (double)(15 * 24);
  compute_day_times(prayerTimes, times);
}

void get_prayer_times_time(PrayerTimes *prayerTimes, double latitude,
                           double longitude, double timezone, double times[]) {
  struct tm *t = localtime(&prayerTimes->time);
  get_prayer_times_date(prayerTimes, 1900 + t->tm_year, t->tm_mon + 1,
                        t->tm_mday, latitude, longitude, timezone, times);
}

double get_effective_timezone_time(time_t local_time) {
  struct tm *tmp = localtime(&local_time);
  tmp->tm_isdst = 0;
  time_t local = mktime(tmp);
  tmp = gmtime(&local_time);
  tmp->tm_isdst = 0;
  time_t gmt = mktime(tmp);
  return (local - gmt) / 3600.0;
}

/* compute local time-zone for a specific date */
double get_effective_timezone_date(int year, int month, int day) {
  struct tm date = {0};
  date.tm_year = year - 1900;
  date.tm_mon = month - 1;
  date.tm_mday = day;
  date.tm_isdst = -1;            // determine it yourself from system
  time_t local = mktime(&date);  // seconds since midnight Jan 1, 1970
  return get_effective_timezone_time(local);
}

double fix_angle(double a) {
  a = a - 360.0 * floor(a / 360.0);
  a = a < 0.0 ? a + 360.0 : a;
  return a;
}

double get_julian_date(int year, int month, int day) {
  if (month <= 2) {
    year -= 1;
    month += 12;
  }

  double a = floor(year / 100.0);
  double b = 2 - a + floor(a / 4.0);

  return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day +
         b - 1524.5;
}

double compute_mid_day(PrayerTimes *prayerTimes, double _t) {
  double t = equation_of_time(prayerTimes->julian_date + _t);
  double z = fix_hour(12 - t);
  return z;
}

/* compute time for a given angle G */
double compute_time(PrayerTimes *prayerTimes, double g, double t) {
  double d = sun_declination(prayerTimes->julian_date + t);
  double z = compute_mid_day(prayerTimes, t);
  double v = 1.0 / 15.0 *
             darccos((-dsin(g) - dsin(d) * dsin(prayerTimes->latitude)) /
                     (dcos(d) * dcos(prayerTimes->latitude)));
  return z + (g > 90.0 ? -v : v);
}

/* compute the time of Asr */
double compute_asr(PrayerTimes *prayerTimes, int step, double t) {
  // Shafi: step=1, Hanafi: step=2
  double d = sun_declination(prayerTimes->julian_date + t);
  double g = -darccot(step + dtan(fabs(prayerTimes->latitude - d)));
  return compute_time(prayerTimes, g, t);
}

/* ---------------------- Compute Prayer Times ----------------------- */

// array parameters must be at least of size TimesCount

/* compute prayer times at given julian date */
void compute_times(PrayerTimes *prayerTimes, double times[]) {
  day_portion(times);

  times[TIMEID_Fajr] = compute_time(
      prayerTimes,
      180.0 - prayerTimes->method_params[prayerTimes->calc_method].fajr_angle,
      times[TIMEID_Fajr]);
  times[TIMEID_Sunrise] =
      compute_time(prayerTimes, 180.0 - 0.833, times[TIMEID_Sunrise]);
  times[TIMEID_Dhuhr] = compute_mid_day(prayerTimes, times[TIMEID_Dhuhr]);
  times[TIMEID_Asr] = compute_asr(prayerTimes, 1 + prayerTimes->asr_juristic,
                                  times[TIMEID_Asr]);
  times[TIMEID_Sunset] = compute_time(prayerTimes, 0.833, times[TIMEID_Sunset]);
  times[TIMEID_Maghrib] = compute_time(
      prayerTimes,
      prayerTimes->method_params[prayerTimes->calc_method].maghrib_value,
      times[TIMEID_Maghrib]);
  times[TIMEID_Isha] = compute_time(
      prayerTimes,
      prayerTimes->method_params[prayerTimes->calc_method].isha_value,
      times[TIMEID_Isha]);
}

/* compute prayer times at given julian date */
void compute_day_times(PrayerTimes *prayerTimes, double times[]) {
  double default_times[] = {5, 6, 12, 13, 18, 18, 18};  // default times
  for (int i = 0; i < TIMEID_TimesCount; ++i) times[i] = default_times[i];

  for (int i = 0; i < 1; ++i) compute_times(prayerTimes, times);

  adjust_times(prayerTimes, times);
}

/* set the angle for calculating Fajr */
void set_fajr_angle(PrayerTimes *prayerTimes, double angle) {
  prayerTimes->method_params[CALCULATION_Custom].fajr_angle = angle;
  prayerTimes->calc_method = CALCULATION_Custom;
}

/* set the angle for calculating Maghrib */
void set_maghrib_angle(PrayerTimes *prayerTimes, double angle) {
  prayerTimes->method_params[CALCULATION_Custom].maghrib_is_minutes = false;
  prayerTimes->method_params[CALCULATION_Custom].maghrib_value = angle;
  prayerTimes->calc_method = CALCULATION_Custom;
}

void set_maghrib_minutes(PrayerTimes *prayerTimes, double minutes) {
  prayerTimes->method_params[CALCULATION_Custom].maghrib_is_minutes = true;
  prayerTimes->method_params[CALCULATION_Custom].maghrib_value = minutes;
  prayerTimes->calc_method = CALCULATION_Custom;
}

/* set the angle for calculating Isha */
void set_isha_angle(PrayerTimes *prayerTimes, double angle) {
  prayerTimes->method_params[CALCULATION_Custom].isha_is_minutes = false;
  prayerTimes->method_params[CALCULATION_Custom].isha_value = angle;
  prayerTimes->calc_method = CALCULATION_Custom;
}

/* set the minutes after Maghrib for calculating Isha */
void set_isha_minutes(PrayerTimes *prayerTimes, double minutes) {
  prayerTimes->method_params[CALCULATION_Custom].isha_is_minutes = true;
  prayerTimes->method_params[CALCULATION_Custom].isha_value = minutes;
  prayerTimes->calc_method = CALCULATION_Custom;
}

/* adjust times in a prayer time array */
void adjust_times(PrayerTimes *prayerTimes, double times[]) {
  for (int i = 0; i < TIMEID_TimesCount; ++i)
    times[i] += prayerTimes->timezone - (prayerTimes->longitude / 15.0);
  times[TIMEID_Dhuhr] += prayerTimes->dhuhr_minutes / 60.0;  // Dhuhr
  if (prayerTimes->method_params[prayerTimes->calc_method]
          .maghrib_is_minutes)  // Maghrib
    times[TIMEID_Maghrib] =
        times[TIMEID_Sunset] +
        prayerTimes->method_params[prayerTimes->calc_method].maghrib_value /
            60.0;
  if (prayerTimes->method_params[prayerTimes->calc_method]
          .isha_is_minutes)  // Isha
    times[TIMEID_Isha] =
        times[TIMEID_Maghrib] +
        prayerTimes->method_params[prayerTimes->calc_method].isha_value / 60.0;

  if (prayerTimes->adjust_high_lats != ADJUSTING_None)
    adjust_high_lat_times(prayerTimes, times);
}

/* adjust Fajr, Isha and Maghrib for locations in higher latitudes */
void adjust_high_lat_times(PrayerTimes *prayerTimes, double times[]) {
  double night_time = time_diff(times[TIMEID_Sunset],
                                times[TIMEID_Sunrise]);  // sunset to sunrise

  // Adjust Fajr
  double fajr_diff =
      night_portion(
          prayerTimes,
          prayerTimes->method_params[prayerTimes->calc_method].fajr_angle) *
      night_time;
  if (isnan(times[TIMEID_Fajr]) ||
      time_diff(times[TIMEID_Fajr], times[TIMEID_Sunrise]) > fajr_diff)
    times[TIMEID_Fajr] = times[TIMEID_Sunrise] - fajr_diff;

  // Adjust Isha
  double isha_angle =
      prayerTimes->method_params[prayerTimes->calc_method].isha_is_minutes
          ? 18.0
          : prayerTimes->method_params[prayerTimes->calc_method].isha_value;
  double isha_diff = night_portion(prayerTimes, isha_angle) * night_time;
  if (isnan(times[TIMEID_Isha]) ||
      time_diff(times[TIMEID_Sunset], times[TIMEID_Isha]) > isha_diff)
    times[TIMEID_Isha] = times[TIMEID_Sunset] + isha_diff;

  // Adjust Maghrib
  double maghrib_angle =
      prayerTimes->method_params[prayerTimes->calc_method].maghrib_is_minutes
          ? 4.0
          : prayerTimes->method_params[prayerTimes->calc_method].maghrib_value;
  double maghrib_diff = night_portion(prayerTimes, maghrib_angle) * night_time;
  if (isnan(times[TIMEID_Maghrib]) ||
      time_diff(times[TIMEID_Sunset], times[TIMEID_Maghrib]) > maghrib_diff)
    times[TIMEID_Maghrib] = times[TIMEID_Sunset] + maghrib_diff;
}

/* the night portion used for adjusting times in higher latitudes */
double night_portion(PrayerTimes *prayerTimes, double angle) {
  switch (prayerTimes->adjust_high_lats) {
    case ADJUSTING_AngleBased:
      return angle / 60.0;
    case ADJUSTING_MidNight:
      return 1.0 / 2.0;
    case ADJUSTING_OneSeventh:
      return 1.0 / 7.0;
    default:
      // Just to return something!
      // In original library nothing was returned
      // Maybe I should throw an exception
      // It must be impossible to reach here
      return 0;
  }
}

/* convert hours to day portions  */
void day_portion(double times[]) {
  for (int i = 0; i < TIMEID_TimesCount; ++i) times[i] /= 24.0;
}

/* ---------------------- Misc Functions ----------------------- */

Position sun_position(double jd) {
  double d = jd - 2451545.0;
  double g = fix_angle(357.529 + 0.98560028 * d);
  double q = fix_angle(280.459 + 0.98564736 * d);
  double l = fix_angle(q + 1.915 * dsin(g) + 0.020 * dsin(2 * g));

  // double r = 1.00014 - 0.01671 * dcos(g) - 0.00014 * dcos(2 * g);
  double e = 23.439 - 0.00000036 * d;

  double dd = darcsin(dsin(e) * dsin(l));
  double ra = darctan2(dcos(e) * dsin(l), dcos(l)) / 15.0;
  ra = fix_hour(ra);
  double eq_t = q / 15.0 - ra;

  return (Position){dd, eq_t};
}

/* degree sin */
inline double dsin(double d) { return sin(deg2rad(d)); }
/* degree cos */
inline double dcos(double d) { return cos(deg2rad(d)); }
/* degree tan */
inline double dtan(double d) { return tan(deg2rad(d)); }
/* degree arcsin */
inline double darcsin(double x) { return rad2deg(asin(x)); }
/* degree arccos */
inline double darccos(double x) { return rad2deg(acos(x)); }
/* degree arctan */
inline double darctan(double x) { return rad2deg(atan(x)); }
/* degree arctan2 */
inline double darctan2(double y, double x) { return rad2deg(atan2(y, x)); }
/* degree arccot */
inline double darccot(double x) { return rad2deg(atan(1.0 / x)); }
/* degree to radian */
inline double deg2rad(double d) { return d * M_PI / 180.0; }
/* radian to degree */
inline double rad2deg(double r) { return r * 180.0 / M_PI; }
