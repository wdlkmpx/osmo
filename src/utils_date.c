/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Maka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Maka <silloz@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "i18n.h"
#include "utils.h"
#include "utils_date.h"

/*============================================================================*/

GDate *
utl_date_new_current (void)
{
	GDate *d = g_date_new ();
	g_date_set_time_t (d, time (NULL));

	return d;
}

/*============================================================================*/

guint
utl_date_get_current_day (void)
{
	GDate *d = utl_date_new_current ();
	guint day = g_date_get_day (d);
	g_date_free (d);

	return day;
}

/*============================================================================*/

guint
utl_date_get_current_month (void)
{
	GDate *d = utl_date_new_current ();
	guint month = g_date_get_month (d);
	g_date_free (d);

	return month;
}

/*============================================================================*/

guint
utl_date_get_current_year (void)
{
	GDate *d = utl_date_new_current ();
	guint year = g_date_get_year (d);
	g_date_free (d);

	return year;
}

/*============================================================================*/

guint32
utl_date_get_current_julian (void)
{
	GDate *d = utl_date_new_current ();
	guint32 julian = g_date_get_julian (d);
	g_date_free (d);

	return julian;
}

/*============================================================================*/

void
utl_date_get_current_dmy (gint *day, gint *month, gint *year)
{
	GDate *d = utl_date_new_current ();

	if (day != NULL) *day = g_date_get_day (d);
	if (month != NULL) *month = g_date_get_month (d);
	if (year != NULL) *year = g_date_get_year (d);

	g_date_free (d);
}

/*============================================================================*/

guint
utl_date_get_days_in_month (const GDate *date)
{
	g_return_val_if_fail (g_date_valid (date), 0);

	return g_date_get_days_in_month (g_date_get_month (date), g_date_get_year (date));
}

/*============================================================================*/

gboolean
utl_date_set_valid_day (GDate *date, gint day)
{
    gint days = utl_date_get_days_in_month (date);

    if (day > days) {
        g_date_set_day (date, days);
		return TRUE;
	}

	g_date_set_day (date, day);
	return FALSE;
}

/*============================================================================*/

gboolean
utl_date_set_valid_dmy (gint *day, gint month, gint year)
{
    gint days = g_date_get_days_in_month (month, year);

    if (*day > days) {
        *day = days;
		return TRUE;
	}

	return FALSE;
}

/*============================================================================*/

void
utl_date_diff (const GDate *date1, const GDate *date2, gint *day, gint *month, gint *year)
{
	g_return_if_fail (g_date_valid (date1));
	g_return_if_fail (g_date_valid (date2));
	g_return_if_fail (g_date_compare (date1, date2) <= 0);

    *day = g_date_get_day (date2) - g_date_get_day (date1);
    *month = g_date_get_month (date2) - g_date_get_month (date1);
    *year = g_date_get_year (date2) - g_date_get_year (date1);

    if (*day < 0) {
        *day += utl_date_get_days_in_month (date1);
		*month -= 1;
    }

    if (*month < 0) {
        *month += 12;
		*year -= 1;
    }
}

/*============================================================================*/

gboolean
utl_date_order (GDate *date1, GDate *date2)
{
	g_return_val_if_fail (g_date_valid (date1), FALSE);
	g_return_val_if_fail (g_date_valid (date2), FALSE);

	if (g_date_compare (date1, date2) > 0)
	{
		GDate tmp = *date1;
		*date1 = *date2;
		*date2 = tmp;
		return TRUE;
	} else
		return FALSE;
}

/*============================================================================*/

void
utl_date_set_nearest_weekday (GDate *date, gint weekdays, gboolean month_mode)
{
	gint day, start_day, days_in_month;
	gint i, j;

	if (weekdays == D_WEEK) return;
	g_return_if_fail (weekdays > D_BAD_DAY && weekdays <= D_WEEK);
	g_return_if_fail (g_date_valid (date));

	day = g_date_get_weekday (date) - 1;

	if (month_mode == TRUE) {

		days_in_month = utl_date_get_days_in_month (date);
		start_day = g_date_get_day (date);

		for (i = 0; i < 7; i++) {
			if (weekdays & (1 << ((day + i) % 7))) break;
		}

		for (j = 0; j < 7; j++) {
			if (weekdays & (1 << ((day + 7 - j) % 7))) break;
		}

		if (start_day + i > days_in_month) i = 7;
		if (start_day - j < 1) j = 7;

		if (i <= j) {
			if (i > 0) g_date_add_days (date, i);
		} else {
			if (j > 0) g_date_subtract_days (date, j);
		}

	} else {

		for (i = 0; i < 7; i++) {
			if (weekdays & (1 << ((day + i) % 7))) break;
		}
		if (i > 0) g_date_add_days (date, i);

	}

}

/*============================================================================*/

gchar *
utl_date_print (const GDate *d, gint date_format, gint override_locale)
{
	gchar date_str[BUFFER_SIZE], *format;

	g_return_val_if_fail (g_date_valid (d), NULL);

	format = utl_date_get_format_str (date_format, override_locale);
	g_date_strftime (date_str, BUFFER_SIZE, format, d);

	return g_strdup (date_str);
}

/*============================================================================*/

gchar *
utl_date_print_j (guint32 julian, gint date_format, gint override_locale)
{
	gchar *date_str;

	if (g_date_valid_julian (julian)) {
		GDate *d = g_date_new_julian (julian);
		date_str = utl_date_print (d, date_format, override_locale);
		g_date_free (d);
	} else
		date_str = g_strdup (_("No date"));

	return date_str;
}

/*============================================================================*/

gchar *
utl_date_get_format_str (gint date_format, gint override_locale)
{
	gchar *date_format_str[] = {
		"%d-%m-%Y", "%m-%d-%Y", "%Y-%m-%d", "%Y-%d-%m", "%e %B", "%A", "%e %B %Y", "%x"
	};

	if (!override_locale)
		return date_format_str[DATE_LOCALE];

	g_return_val_if_fail (date_format >= DATE_DD_MM_YYYY &&
	                      date_format <= DATE_FULL, NULL);

	return date_format_str[date_format];
}

/*============================================================================*/

void
utl_date_get_dmy (const GDate *date, gint *day, gint *month, gint *year)
{
	g_return_if_fail (g_date_valid (date));

	if (day != NULL) *day = g_date_get_day (date);
	if (month != NULL) *month = g_date_get_month (date);
	if (year != NULL) *year = g_date_get_year (date);
}

/*============================================================================*/

void
utl_date_julian_to_dmy (guint32 julian, gint *day, gint *month, gint *year)
{
	g_return_if_fail (g_date_valid_julian (julian));

	GDate *d = g_date_new_julian (julian);
	if (day != NULL) *day = g_date_get_day (d);
	if (month != NULL) *month = g_date_get_month (d);
	if (year != NULL) *year = g_date_get_year (d);

	g_date_free (d);
}

/*============================================================================*/

guint32
utl_date_dmy_to_julian (guint day, guint month, guint year)
{
	g_return_val_if_fail (g_date_valid_dmy (day, month, year), 0);

	GDate *d = g_date_new_dmy (day, month, year);
	guint32 julian = g_date_get_julian (d);
	g_date_free (d);

	return julian;
}

/*============================================================================*/
/* 
 * lib_date routines
 * Copyright (c) 1995, 1996, 1997, 1998 by Steffen Beyer
 *
 */

guint
utl_get_month_length (guint leap_year, guint month) {

const guint month_length[2][13] = {
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

	return month_length[leap_year][month];
}

guint
utl_get_days_in_months (guint leap_year, guint month) {

const guint days_in_months[2][14] = {
    { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

	return days_in_months[leap_year][month];
}

glong
utl_year_to_days (guint year) {
    return( year * 365L + (year / 4) - (year / 100) + (year / 400) );
}

glong
utl_calc_days (guint year, guint mm, guint dd) {

	gboolean lp;

    if (year < 1) return(0L);
    if ((mm < 1) || (mm > 12)) return(0L);
    if ((dd < 1) || (dd > utl_get_month_length((lp = g_date_is_leap_year(year)), mm))) return(0L);

	return( utl_year_to_days(--year) + utl_get_days_in_months(lp, mm) + dd );
}

glong
utl_dates_difference (guint year1, guint mm1, guint dd1,
					  guint year2, guint mm2, guint dd2) {

    return (utl_calc_days(year2, mm2, dd2) - utl_calc_days(year1, mm1, dd1));
}

guint
utl_day_of_week (guint year, guint mm, guint dd) {

	glong  days;

    days = utl_calc_days(year, mm, dd);
    if (days > 0L) {
        days--;
        days %= 7L;
        days++;
    }
    return( (guint) days );
}

guint 
utl_weeks_in_year (guint year) {
    return(52 + ((utl_day_of_week(year, 1, 1) == 4) || (utl_day_of_week(year, 12, 31) == 4)));
}

guint
utl_get_week_number (guint year, guint mm, guint dd) {

	guint first;

    first = utl_day_of_week(year,1,1) - 1;
    return( (guint) ( (utl_dates_difference(year,1,1, year,mm,dd) + first) / 7L ) +
            (first < 4) );
}

gboolean
utl_week_of_year (guint *week, guint *year, guint mm, guint dd) {

	if (g_date_valid_dmy (dd, mm, *year)) {
        *week = utl_get_week_number(*year, mm, dd);
        if (*week == 0)
            *week = utl_weeks_in_year(--(*year));
        else if (*week > utl_weeks_in_year(*year)) {
            *week = 1;
            (*year)++;
        }
        return TRUE;
    }
    return FALSE;
}

/*------------------------------------------------------------------------------*/

