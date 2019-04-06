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

#ifndef _UTILS_DATE_TIME_H
#define _UTILS_DATE_TIME_H

#include "utils_date.h"
#include "utils_time.h"

gint     utl_date_time_compare      (GDate *date1, TIME *time1, GDate *date2, TIME *time2);
gint     utl_date_time_compare_js   (guint32 julian1, gint seconds1, guint32 julian2, gint seconds2);

gboolean utl_date_time_in_the_past_js (guint32 julian, gint seconds);

gchar*     utl_date_time_print            (GDate *d, gint date_format, TIME *t, gint time_format, gint override_locale);
gchar*     utl_date_time_print_js         (guint32 julian, gint date_format, gint seconds, gint time_format, gint override_locale);
gchar*     utl_date_time_print_default    (guint32 julian, gint seconds, gboolean with_sec);

gboolean utl_date_time_order        (GDate *date1, TIME *time1, GDate *date2, TIME *time2);

#endif /* _UTILS_DATE_TIME_H */

