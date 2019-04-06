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

#include "options_prefs.h"
#include "utils_date_time.h"

/*============================================================================*/

gint
utl_date_time_compare (GDate *date1, TIME *time1, GDate *date2, TIME *time2)
{
	g_return_val_if_fail (g_date_valid (date1), 0);
	g_return_val_if_fail (g_date_valid (date2), 0);
	g_return_val_if_fail (utl_time_valid (time1), 0);
	g_return_val_if_fail (utl_time_valid (time2), 0);

	gint n;

	if ((n = g_date_compare (date1, date2)) == 0)
		return utl_time_compare (time1, time2);

	return n;
}

/*============================================================================*/

gint
utl_date_time_compare_js (guint32 julian1, gint seconds1, guint32 julian2, gint seconds2)
{
	if (julian1 == julian2)
		return ((seconds1 > seconds2) ? 1 : ((seconds1 < seconds2) ? -1 : 0));

	return ((julian1 > julian2) ? 1 : -1);
}

/*============================================================================*/

gboolean
utl_date_time_in_the_past_js (guint32 julian, gint seconds)
{
	guint32 cj = utl_date_get_current_julian ();
	gint cs = utl_time_get_current_seconds ();

	if (utl_date_time_compare_js (julian, seconds, cj, cs) <= 0)
		return TRUE;
	else
		return FALSE;
}

/*============================================================================*/

gchar *
utl_date_time_print (GDate *d, gint date_format, TIME *t, gint time_format, gint override_locale)
{
	g_return_val_if_fail (g_date_valid (d), NULL);
	g_return_val_if_fail (utl_time_valid (t), NULL);

	gchar *date_str = utl_date_print (d, date_format, override_locale);
	gchar *time_str = utl_time_print (t, time_format, override_locale);

	gchar *date_time_str = g_strdup_printf ("%s, %s", date_str, time_str);
	g_free (date_str);
	g_free (time_str);

	return date_time_str;
}

/*============================================================================*/

gchar *
utl_date_time_print_js (guint32 julian, gint date_format, gint seconds, gint time_format, gint override_locale)
{
	gchar *date_time_str, *date_str, *time_str;

	if (utl_time_valid_seconds (seconds)) {
		date_str = utl_date_print_j (julian, date_format, override_locale);
		time_str = utl_time_print_s (seconds, time_format, override_locale);
		date_time_str = g_strdup_printf ("%s, %s", date_str, time_str);
		g_free (date_str);
		g_free (time_str);
	} else
		date_time_str = utl_date_print_j (julian, date_format, override_locale);

	return date_time_str;
}

/*============================================================================*/

gchar *
utl_date_time_print_default (guint32 julian, gint seconds, gboolean with_sec)
{
	gchar *str = utl_date_time_print_js (julian, config.date_format,
	                                     seconds, config.time_format + (with_sec ? 2 : 0),
/*	                                              TIME_HH_MM -> TIME_HH_MM_SS */
	                                     config.override_locale_settings);

	return str;
}

/*============================================================================*/

gboolean
utl_date_time_order (GDate *date1, TIME *time1, GDate *date2, TIME *time2)
{
	g_return_val_if_fail (g_date_valid (date1), FALSE);
	g_return_val_if_fail (g_date_valid (date2), FALSE);
	g_return_val_if_fail (utl_time_valid (time1), FALSE);
	g_return_val_if_fail (utl_time_valid (time2), FALSE);

	if (utl_date_order (date1, date2)) {

		TIME tmp = *time1;
		*time1 = *time2;
		*time2 = tmp;
		return TRUE;

	} else if (g_date_compare (date1, date2) == 0 && utl_time_order (time1, time2))
		return TRUE;
	else
		return FALSE;
}

/*============================================================================*/

