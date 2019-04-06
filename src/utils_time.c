/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Maka <pasp@users.sourceforge.net>
 *           (C) 2007-2009 Piotr Maka <silloz@users.sourceforge.net>
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

#include "options_prefs.h"
#include "utils_time.h"

/*============================================================================*/

TIME *
utl_time_new (void)
{
	TIME *t = g_slice_new (TIME);

	return t;
}

/*============================================================================*/

TIME *
utl_time_new_hms (gint hour, gint minute, gint second)
{
	TIME *t;
	g_return_val_if_fail (utl_time_valid_hms (hour, minute, second), NULL);

	t = g_slice_new (TIME);

	t->hour = hour;
	t->minute = minute;
	t->second = second;

	return t;
}

/*============================================================================*/

TIME *
utl_time_new_seconds (gint seconds)
{
	TIME *t;
	g_return_val_if_fail (utl_time_valid_seconds (seconds), NULL);

	t = g_slice_new (TIME);
	utl_time_set_seconds (t, seconds);

	return t;
}

/*============================================================================*/

TIME *
utl_time_new_now (void)
{
	TIME *t;
	struct tm *ptm;
	time_t tmm;

	tmm = time (NULL);
	ptm = localtime (&tmm);

	t = g_slice_new (TIME);
	utl_time_set_hms (t, ptm->tm_hour, ptm->tm_min, (ptm->tm_sec < 60) ? ptm->tm_sec : 59);

	return t;
}

/*============================================================================*/

void
utl_time_set_hms (TIME *t, gint hour, gint minute, gint second)
{
	g_return_if_fail (t != NULL);
	g_return_if_fail (utl_time_valid_hms (hour, minute, second));

	t->hour = hour;
	t->minute = minute;
	t->second = second;
}

/*============================================================================*/

void
utl_time_set_hour (TIME *t, gint hour)
{
	g_return_if_fail (t != NULL);
	g_return_if_fail (utl_time_valid_hour (hour));

	t->hour = hour;
}

/*============================================================================*/

void
utl_time_set_minute (TIME *t, gint minute)
{
	g_return_if_fail (t != NULL);
	g_return_if_fail (utl_time_valid_minute (minute));

	t->minute = minute;
}

/*============================================================================*/

void
utl_time_set_second (TIME *t, gint second)
{
	g_return_if_fail (t != NULL);
	g_return_if_fail (utl_time_valid_second (second));

	t->second = second;
}

/*============================================================================*/

void
utl_time_set_seconds (TIME *t, gint seconds)
{
	g_return_if_fail (t != NULL);
	g_return_if_fail (utl_time_valid_seconds (seconds));

	t->hour = seconds / 3600;
	seconds %= 3600;

	t->minute = seconds / 60;
	seconds %= 60;

	t->second = seconds;
}

/*============================================================================*/

void
utl_time_get_hms (const TIME *t, gint *hour, gint *minute, gint *second)
{
	g_return_if_fail (utl_time_valid (t));

	if (hour != NULL) *hour = t->hour;
	if (minute != NULL) *minute = t->minute;
	if (second != NULL) *second = t->second;
}

/*============================================================================*/

gint
utl_time_get_hour (const TIME *t)
{
	g_return_val_if_fail (utl_time_valid (t), 0);

	return t->hour;
}

/*============================================================================*/

gint
utl_time_get_minute (const TIME *t)
{
	g_return_val_if_fail (utl_time_valid (t), 0);

	return t->minute;
}

/*============================================================================*/

gint
utl_time_get_second (const TIME *t)
{
	g_return_val_if_fail (utl_time_valid (t), 0);

	return t->second;
}

/*============================================================================*/

gint
utl_time_get_seconds (const TIME *t)
{
	g_return_val_if_fail (utl_time_valid (t), 0);

	return t->hour * 3600 + t->minute * 60 + t->second;
}

/*============================================================================*/

gint
utl_time_get_current_hour ()
{
	TIME *time;
	gint current_hour;

	time = utl_time_new_now ();
	current_hour = time->hour;
	utl_time_free (time);

	return current_hour;
}

/*============================================================================*/

gint
utl_time_get_current_minute ()
{
	TIME *time;
	gint current_minute;

	time = utl_time_new_now ();
	current_minute = time->minute;
	utl_time_free (time);

	return current_minute;
}

/*============================================================================*/

gint
utl_time_get_current_seconds ()
{
	TIME *t = utl_time_new_now ();
	gint seconds = utl_time_get_seconds (t);
	utl_time_free (t);

	return seconds;
}

/*============================================================================*/

gint
utl_time_add (TIME *f_time, TIME *s_time)
{
	gint days;

	g_return_val_if_fail (utl_time_valid (s_time), 0);

	days = utl_time_add_hours (f_time, s_time->hour);
	days += utl_time_add_minutes (f_time, s_time->minute);
	days += utl_time_add_seconds (f_time, s_time->second);

	return days;
}

/*============================================================================*/

gint
utl_time_add_hours (TIME *t, gint hours)
{
	gint days;

	hours += utl_time_get_hour (t);

	days = hours / 24;
	utl_time_set_hour (t, hours % 24);
	
	return days;
}

/*============================================================================*/

gint
utl_time_add_minutes (TIME *t, gint minutes)
{
	gint hours;

	minutes += utl_time_get_minute (t);

	hours = minutes / 60;
	utl_time_set_minute (t, minutes % 60);

	return (hours > 0 ? utl_time_add_hours (t, hours) : 0);
}

/*============================================================================*/

gint
utl_time_add_seconds (TIME *t, gint seconds)
{
	gint minutes;

	seconds += utl_time_get_second (t);

	minutes = seconds / 60;
	utl_time_set_second (t, seconds % 60);

	return (minutes > 0 ? utl_time_add_minutes (t, minutes) : 0);
}

/*============================================================================*/

gint
utl_time_subtract (TIME *f_time, TIME *s_time)
{
	gint days;

	g_return_val_if_fail (utl_time_valid (s_time), 0);

	days = utl_time_subtract_hours (f_time, s_time->hour);
	days += utl_time_subtract_minutes (f_time, s_time->minute);
	days += utl_time_subtract_seconds (f_time, s_time->second);

	return days;
}

/*============================================================================*/

gint
utl_time_subtract_hours (TIME *time, gint hours)
{
	gint days, h;

	days = hours / 24;
	hours %= 24;

	h = utl_time_get_hour (time);

	if (h < hours) {
		h += 24;
		days++;
	}

	utl_time_set_hour (time, h - hours);

	return days;
}

/*============================================================================*/

gint
utl_time_subtract_minutes (TIME *time, gint minutes)
{
	gint hours, m;

	hours = minutes / 60;
	minutes %= 60;

	m = utl_time_get_minute (time);

	if (m < minutes) {
		m += 60;
		hours++;
	}

	utl_time_set_minute (time, m - minutes);

	return (hours > 0 ? utl_time_subtract_hours (time, hours) : 0);
}

/*============================================================================*/

gint
utl_time_subtract_seconds (TIME *t, gint seconds)
{
	gint minutes, s;

	minutes = seconds / 60;
	seconds %= 60;

	s = utl_time_get_second (t);

	if (s < seconds) {
		s += 60;
		minutes++;
	}

	utl_time_set_second (t, s - seconds);

	return (minutes > 0 ? utl_time_subtract_minutes (t, minutes) : 0);
}

/*============================================================================*/

void
utl_time_clamp (TIME *t, const TIME *tmin, const TIME *tmax)
{
	if (tmin != NULL && tmax != NULL)
		g_return_if_fail (utl_time_compare (tmin, tmax) <= 0);

	gint s = utl_time_get_seconds (t);

	if (tmin != NULL)
		if (s < utl_time_get_seconds (tmin))
			*t = *tmin;

	if (tmax != NULL)
		if (s > utl_time_get_seconds (tmax))
			*t = *tmax;
}

/*============================================================================*/

gint
utl_time_compare (const TIME *time1, const TIME *time2)
{
	gint s1 = utl_time_get_seconds (time1);
	gint s2 = utl_time_get_seconds (time2);

	return (s1 > s2 ? 1 : (s1 == s2 ? 0 : -1));
}

/*============================================================================*/

gboolean
utl_time_order (TIME *time1, TIME *time2)
{
	if (utl_time_compare (time1, time2) > 0) {
		TIME tmp = *time1;
		*time1 = *time2;
		*time2 = tmp;
		return TRUE;
	} else
		return FALSE;
}

/*============================================================================*/

gint
utl_time_seconds_between (const TIME *time1, const TIME *time2)
{
	return utl_time_get_seconds (time2) - utl_time_get_seconds (time1);
}

/*============================================================================*/

gchar *
utl_time_print (const TIME *t, gint time_format, gint override_locale)
{
	gchar time_str[BUFFER_SIZE], *format;
	struct tm timer;

	g_return_val_if_fail (utl_time_valid (t), NULL);

	utl_time_get_hms (t, &(timer.tm_hour), &(timer.tm_min), &(timer.tm_sec));

	format = utl_time_get_format_str (time_format, override_locale);
	strftime (time_str, BUFFER_SIZE-1, format, &timer);

    return g_strdup (time_str);
}

/*============================================================================*/

gchar *
utl_time_print_s (gint seconds, gint time_format, gint override_locale)
{
	g_return_val_if_fail (utl_time_valid_seconds (seconds), NULL);

	TIME *t = utl_time_new_seconds (seconds);
	gchar *timestr = utl_time_print (t, time_format, override_locale);
	utl_time_free (t);

    return timestr;
}

/*============================================================================*/

gchar *
utl_time_print_default (gint seconds, gboolean with_sec)
{
	g_return_val_if_fail (utl_time_valid_seconds (seconds), NULL);

	TIME *t = utl_time_new_seconds (seconds);
	gchar *timestr = utl_time_print (t, config.time_format + (with_sec ? 2 : 0),
	                                 config.override_locale_settings);
	utl_time_free (t);

    return timestr;
}

/*============================================================================*/

gchar *
utl_time_get_format_str (gint time_format, gint override_locale)
{
	static gchar *time_format_str[] = {
	    "%R", "%I:%M %p", "%T", "%r", "%Z", "%X"
	};

	if (!override_locale)
		return time_format_str[TIME_LOCALE];

	g_return_val_if_fail (time_format >= TIME_HH_MM &&
	                      time_format <= TIME_TIMEZONE, NULL);

	return time_format_str[time_format];
}

/*============================================================================*/

gint
utl_time_hms_to_seconds (gint hour, gint minute, gint second)
{
	g_return_val_if_fail (utl_time_valid_hms (hour, minute, second), 0);

	return hour * 3600 + minute * 60 + second;
}

/*============================================================================*/

gboolean
utl_time_valid (const TIME *t)
{
	g_return_val_if_fail (t != NULL, FALSE);

	return (utl_time_valid_hms (t->hour, t->minute, t->second));
}

/*============================================================================*/

gboolean
utl_time_valid_hms (gint hour, gint minute, gint second)
{
	return (utl_time_valid_hour (hour) &&
	        utl_time_valid_minute (minute) &&
	        utl_time_valid_second (second));
}

/*============================================================================*/

gboolean
utl_time_valid_hour (gint hour)
{
	return (hour >= 0 && hour < 24);
}

/*============================================================================*/

gboolean
utl_time_valid_minute (gint minute)
{
	return (minute >= 0 && minute < 60);
}

/*============================================================================*/

gboolean
utl_time_valid_second (gint second)
{
	return (second >= 0 && second < 60);
}

/*============================================================================*/

gboolean
utl_time_valid_seconds (gint seconds)
{
	return (seconds >= 0 && seconds < 86400);
}

/*============================================================================*/

void
utl_time_free (TIME *t)
{
	g_return_if_fail (t != NULL);

	g_slice_free (TIME, t);
	t = NULL;
}

/*============================================================================*/

