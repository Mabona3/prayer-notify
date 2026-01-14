/*-------------------------- In the name of God ----------------------------*\

    libprayertimes 1.0
    Islamic prayer times calculator library
    Based on PrayTimes 1.1 JavaScript library

----------------------------- Copyright Block --------------------------------

Copyright (C) 2007-2010 PrayTimes.org

Developed By: Mohammad Ebrahim Mohammadi Panah <ebrahim at mohammadi dot ir>
Based on a JavaScript Code By: Hamid Zarrabi-Zadeh

License: GNU LGPL v3.0

TERMS OF USE:
    Permission is granted to use this code, with or
    without modification, in any website or application
    provided that credit is given to the original work
    with a link back to PrayTimes.org.

This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY.

PLEASE DO NOT REMOVE THIS COPYRIGHT BLOCK.

------------------------------------------------------------------------------

User's Manual:
http://praytimes.org/manual

Calculating Formulas:
http://praytimes.org/calculation

Code Repository:
http://code.ebrahim.ir/prayertimes/

\*--------------------------------------------------------------------------*/

#ifndef PRAYERTIMES_H
#define PRAYERTIMES_H

#include <stdbool.h>
#include <time.h>

/* compute equation of time */
#define equation_of_time(jd) sun_position(jd).lat

/* compute declination angle of sun */
#define sun_declination(jd) sun_position(jd).lng

/* degree sin */
double dsin(double d);

/* degree cos */
double dcos(double d);

/* degree tan */
double dtan(double d);

/* degree arcsin */
double darcsin(double x);

/* degree arccos */
double darccos(double x);

/* degree arctan */
double darctan(double x);

/* degree arctan2 */
double darctan2(double y, double x);

/* degree arccot */
double darccot(double x);

/* degree to radian */
double deg2rad(double d);

/* radian to degree */
double rad2deg(double r);

typedef struct {
  double lng;
  double lat;
} Position;

typedef enum {
  TIMEID_Fajr,
  TIMEID_Sunrise,
  TIMEID_Dhuhr,
  TIMEID_Asr,
  TIMEID_Sunset,
  TIMEID_Maghrib,
  TIMEID_Isha,

  TIMEID_TimesCount
} TimeID;

typedef enum {
  JURISTIC_Shafi,   // Shafii (standard)
  JURISTIC_Hanafi,  // Hanafi
                    //
  JURISTICMETHOD_COUNT,
} JuristicMethod;

typedef enum {
  CALCULATION_Jafari,   // Ithna Ashari
  CALCULATION_Karachi,  // University of Islamic Sciences, Karachi
  CALCULATION_ISNA,     // Islamic Society of North America (ISNA)
  CALCULATION_MWL,      // Muslim World League (MWL)
  CALCULATION_Makkah,   // Umm al-Qura, Makkah
  CALCULATION_Egypt,    // Egyptian General Authority of Survey
  CALCULATION_Custom,   // Custom Setting

  CALCULATIONMETHOD_COUNT
} CalculationMethod;

typedef enum {
  ADJUSTING_MidNight,    // middle of night
  ADJUSTING_OneSeventh,  // 1/7th of night
  ADJUSTING_AngleBased,  // angle/60th of night
  ADJUSTING_None,        // No adjustment
                         //
  ADJUSTINGMETHOD_COUNT,
} AdjustingMethod;

typedef struct {
  double fajr_angle;
  bool maghrib_is_minutes;
  double maghrib_value;  // angle or minutes
  bool isha_is_minutes;
  double isha_value;  // angle or minutes
} MethodConfig;

typedef struct {
  CalculationMethod calc_method;     // caculation method
  JuristicMethod asr_juristic;       // Juristic method for Asr
  AdjustingMethod adjust_high_lats;  // adjusting method for higher latitudes
  double dhuhr_minutes;              // minutes after mid-day for Dhuhr

  time_t time;  // The day in which it is will be processed.

  double longitude;
  double latitude;
  double timezone;
  double julian_date;
  MethodConfig method_params[CALCULATIONMETHOD_COUNT];
} PrayerTimes;

/*            Enum To String Section                */

static const char *Juristic[JURISTICMETHOD_COUNT] = {"Shafii", "Hanafi"};

static const char *TimeName[TIMEID_TimesCount] = {
    "Fajr", "Sunrise", "Dhuhr", "Asr", "Sunset", "Maghrib", "Isha",
};

static const char *Calculation[CALCULATIONMETHOD_COUNT] = {
    "Jafari",   // Ithna Ashari
    "Karachi",  // University of Islamic Sciences, Karachi
    "Isna",     // Islamic Society of North America (ISNA)
    "Mwl",      // Muslim World League (MWL)
    "Makkah",   // Umm al-Qura, Makkah
    "Egypt",    // Egyptian General Authority of Survey
    "Custom"    // Custom Setting
};

// Used for converting from enum to string
static const char *Adjusting[] = {
    "MidNight",    // middle of night
    "OneSeventh",  // 1/7th of night
    "AngleBased",  // angle/60th of night
    "None",        // No adjustment
};

/* --------------------- User Interface ----------------------- */
/*
        PrayerTimes(CalculationMethod calc_method = Jafari,
                        JuristicMethod asr_juristic = Shafii,
                        AdjustingMethod adjust_high_lats = MidNight,
                        double dhuhr_minutes = 0)

        get_prayer_times(date, latitude, longitude, timezone, &times)
        get_prayer_times(year, month, day, latitude, longitude, timezone,
   &times)

        set_calc_method(method_id)
        set_asr_method(method_id)
        set_high_lats_adjust_method(method_id)		// adjust method
   for higher latitudes

        set_fajr_angle(angle)
        set_maghrib_angle(angle)
        set_isha_angle(angle)
        set_dhuhr_minutes(minutes)		// minutes after mid-day
        set_maghrib_minutes(minutes)		// minutes after sunset
        set_isha_minutes(minutes)		// minutes after maghrib

        get_float_time_parts(time, &hours, &minutes)
        float_time_to_time24(time)
        float_time_to_time12(time)
        float_time_to_time12ns(time)
*/

/* -------------------- Interface Functions -------------------- */

MethodConfig create_method_config(double fajr_angle, bool maghrib_is_minutes,
                                  double maghrib_value, bool isha_is_minutes,
                                  double isha_value);

/*
 * Creating the prayer times instance to caluclate the time
 * */
PrayerTimes *create_prayer_times(CalculationMethod calc_method,
                                 JuristicMethod asr_juristic,
                                 AdjustingMethod adjust_high_lats,
                                 double dhuhr_minutes);

void get_prayer_times_date(PrayerTimes *prayerTimes, int year, int month,
                           int day, double latitude, double longitude,
                           double timezone, double times[]);

/* return prayer times for a given date */
void get_prayer_times_time(PrayerTimes *prayerTimes, double latitude,
                           double longitude, double timezone, double times[]);

/* compute local time-zone for a specific date */
double get_effective_timezone_time(time_t local_time);

/* compute local time-zone for a specific date */
double get_effective_timezone_date(int year, int month, int day);

/* degree sin */
/* range reduce angle in degrees. */
double fix_angle(double a);

/* compute declination angle of sun and equation of time */
Position sun_position(double jd);

/* compute mid-day (Dhuhr, Zawal) time */
double compute_mid_day(PrayerTimes *prayerTimes, double g);

/* compute time for a given angle G */
double compute_time(PrayerTimes *prayerTimes, double g, double t);

/* compute the time of Asr */
double compute_asr(PrayerTimes *prayerTimes, int step, double t);

/* ---------------------- Compute Prayer Times ----------------------- */

// array parameters must be at least of size TimesCount

/* compute prayer times at given julian date */
void compute_times(PrayerTimes *, double[]);

/* compute prayer times at given julian date */
void compute_day_times(PrayerTimes *prayerTimes, double times[]);

/* adjust times in a prayer time array */
void adjust_times(PrayerTimes *prayerTimes, double times[]);

/* adjust Fajr, Isha and Maghrib for locations in higher latitudes */
void adjust_high_lat_times(PrayerTimes *prayerTimes, double times[]);

/* the night portion used for adjusting times in higher latitudes */
double night_portion(PrayerTimes *, double);

/* convert hours to day portions  */
void day_portion(double times[]);

/* calculate julian date from a calendar date */
double get_julian_date(int year, int month, int day);

/* set the angle for calculating Maghrib */
void set_maghrib_minutes(PrayerTimes *prayerTimes, double minutes);

/* set the angle for calculating Fajr */
void set_fajr_angle(PrayerTimes *prayerTimes, double angle);

/* set the angle for calculating Maghrib */
void set_maghrib_angle(PrayerTimes *prayerTimes, double angle);

/* set the angle for calculating Isha */
void set_isha_angle(PrayerTimes *prayerTimes, double angle);

/* set the minutes after Maghrib for calculating Isha */
void set_isha_minutes(PrayerTimes *prayerTimes, double minutes);

#endif  // PRAYERTIMES_H
