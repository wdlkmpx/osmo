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

#ifndef _UTILS_TIME_H
#define _UTILS_TIME_H

#include "gui.h"

typedef struct {
	gint hour;
	gint minute;
	gint second;
} TIME;

enum {
	TIME_HH_MM = 0,         /* don't change order */
	TIME_HH_MM_AMPM,
	TIME_HH_MM_SS,
	TIME_HH_MM_SS_AMPM,
	TIME_TIMEZONE,          /* last entry */
	TIME_LOCALE
};

TIME* utl_time_new         (void);
TIME* utl_time_new_hms     (gint hour, gint minute, gint second);
TIME* utl_time_new_seconds (gint seconds);
TIME* utl_time_new_now     (void);

void utl_time_set_hms     (TIME *time, gint hour, gint minute, gint second);
void utl_time_set_hour    (TIME *time, gint hour);
void utl_time_set_minute  (TIME *time, gint minute);
void utl_time_set_second  (TIME *time, gint second);
void utl_time_set_seconds (TIME *time, gint seconds);

void utl_time_get_hms     (const TIME *time, gint *hour, gint *minute, gint *second);
gint utl_time_get_hour    (const TIME *time);
gint utl_time_get_minute  (const TIME *time);
gint utl_time_get_second  (const TIME *time);
gint utl_time_get_seconds (const TIME *time);

gint utl_time_get_current_hour (void);
gint utl_time_get_current_minute (void);

gint utl_time_get_current_seconds (void);

gint utl_time_add         (TIME *time1, TIME *time2);
gint utl_time_add_hours   (TIME *time, gint hours);
gint utl_time_add_minutes (TIME *time, gint minutes);
gint utl_time_add_seconds (TIME *time, gint seconds);

gint utl_time_subtract         (TIME *time1, TIME *time2);
gint utl_time_subtract_hours   (TIME *time, gint hours);
gint utl_time_subtract_minutes (TIME *time, gint minutes);
gint utl_time_subtract_seconds (TIME *time, gint seconds);

void     utl_time_clamp           (TIME *time, const TIME *min_time, const TIME *max_time);
gint     utl_time_compare         (const TIME *time1, const TIME *time2);
gboolean utl_time_order           (TIME *time1, TIME *time2);
gint     utl_time_seconds_between (const TIME *time1, const TIME *time2);

gchar* utl_time_print          (const TIME *time, gint time_format, gint override_locale);
gchar* utl_time_print_s        (gint seconds, gint time_format, gint override_locale);
gchar* utl_time_print_default  (gint seconds, gboolean with_sec);
gchar* utl_time_get_format_str (gint time_format, gint override_locale);

gint utl_time_hms_to_seconds (gint hour, gint minute, gint second);

gboolean utl_time_valid         (const TIME *time);
gboolean utl_time_valid_hms     (gint hour, gint minute, gint second);
gboolean utl_time_valid_hour    (gint hour);
gboolean utl_time_valid_minute  (gint minute);
gboolean utl_time_valid_second  (gint second);
gboolean utl_time_valid_seconds (gint seconds);

void utl_time_free (TIME *time);

#endif /* _UTILS_TIME_H */

