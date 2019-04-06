/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007-2009 Tomasz Mąka <pasp@users.sourceforge.net>
 *               2007-2009 Piotr Mąka <silloz@users.sourceforge.net>
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

#ifndef _UTILS_DATE_H
#define _UTILS_DATE_H

#include "gui.h"

#define LEAP_YEAR 2008

enum {
    DATE_DD_MM_YYYY = 0,    /* do not change order */
    DATE_MM_DD_YYYY,
    DATE_YYYY_MM_DD,
    DATE_YYYY_DD_MM,
    DATE_NAME_DAY,
    DATE_DAY_OF_WEEK_NAME,
    DATE_FULL,              /* last entry */
	DATE_LOCALE
};

enum {
	D_BAD_DAY   = 0,
	D_MONDAY    = 1,
	D_TUESDAY   = 2,
	D_WEDNESDAY = 4,
	D_THURSDAY  = 8,
	D_FRIDAY    = 16,
	D_SATURDAY  = 32,
	D_SUNDAY    = 64,
	D_WEEK      = 127
};

GDate *     utl_date_new_current            (void);
guint       utl_date_get_current_day        (void);
guint       utl_date_get_current_month      (void);
guint       utl_date_get_current_year       (void);
guint32     utl_date_get_current_julian     (void);
void        utl_date_get_current_dmy        (gint *day, gint *month, gint *year);
guint       utl_date_get_days_in_month      (const GDate *date);
gboolean    utl_date_set_valid_day          (GDate *date, gint day);
gboolean    utl_date_set_valid_dmy          (gint *day, gint month, gint year);
void        utl_date_diff                   (const GDate *date1, const GDate *date2, gint *day, gint *month, gint *year);
gboolean    utl_date_order                  (GDate *date1, GDate *date2);
void        utl_date_set_nearest_weekday    (GDate *date, gint weekdays, gboolean month_mode);
gchar *     utl_date_print                  (const GDate *d, gint date_format, gint override_locale);
gchar *     utl_date_print_j                (guint32 julian, gint date_format, gint override_locale);
gchar *     utl_date_get_format_str         (gint date_format, gint override_locale);
void        utl_date_get_dmy                (const GDate *date, gint *day, gint *month, gint *year);
void        utl_date_julian_to_dmy          (guint32 julian, gint *day, gint *month, gint *year);
guint32     utl_date_dmy_to_julian          (guint day, guint month, guint year);

/* Deprecated: */
guint       utl_get_month_length            (guint leap_year, guint month);
guint       utl_get_days_in_months          (guint leap_year, guint month);
glong       utl_year_to_days                (guint year);
glong       utl_calc_days                   (guint year, guint mm, guint dd);
glong       utl_dates_difference            (guint year1, guint mm1, guint dd1,
                                             guint year2, guint mm2, guint dd2);
guint       utl_day_of_week                 (guint year, guint mm, guint dd);
guint       utl_weeks_in_year               (guint year);
guint       utl_get_week_number             (guint year, guint mm, guint dd);
gboolean    utl_week_of_year                (guint *week, guint *year, guint mm, guint dd);

#endif /* _UTILS_DATE_H */

