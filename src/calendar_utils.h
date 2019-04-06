
/*
 * Osmo - a handy personal organizer
 *
 * Copyright (C) 2007 Tomasz Maka <pasp@users.sourceforge.net>
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

#ifndef _CALENDAR_UTILS_H
#define _CALENDAR_UTILS_H

#include "gui.h"
#include "utils_time.h"

#define JULIAN_GREGORIAN_YEAR   1582

enum {
    TIME_24 = 0,
    TIME_12
};

struct tm * get_tm_struct                   (void);
gchar*      current_time_to_str             (gint time_format, gint override_locale);
gchar*      time_to_str                     (TIME *time, gint time_format, gint override_locale);
gint        get_current_hour                (void);
gint        get_current_minute              (void);
gint        get_current_second              (void);

void        sync_cal_date_with_gdate        (GUI *appGUI);
guint       get_day_of_week                 (guint day, guint month, guint year);
gchar*      current_date_text               (void);
gchar*      calendar_get_date_name          (GDate *cdate);
gchar*      julian_to_str                   (guint32 julian_day, gint date_format, gint override_locale);
guint32     str_to_julian                   (gchar *date_str, gint date_format);
gint        julian_to_year                  (guint32 julian_day);
guint       month_name_to_number            (gchar *month_str);
gchar*      get_current_date_distance_str   (guint32 julian);
gchar*      get_date_time_str               (guint32 julian, gint seconds);
gchar*      get_date_time_full_str          (guint32 julian, gint seconds);
void        parse_numeric_date              (gchar *date_str, gint *first, gint *second, gint *third);
gchar*      utl_get_zodiac_name             (guint day, guint month);
gchar*      get_chinese_year_name           (guint year);

#endif /* _CALENDAR_UTILS_H */

