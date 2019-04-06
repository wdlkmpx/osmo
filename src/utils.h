
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

#ifndef _UTILS_H
#define _UTILS_H

#include "gui.h"

enum helpers {
    UNKNOWN = 0,
    WWW,
    EMAIL,
    SOUND
};

gchar *     utl_text_replace                    (const gchar *text, const gchar *regex, const gchar *replacement);
gchar *     utl_text_to_html                    (const gchar *text, gboolean ignoreBR);
gchar *     utl_get_day_name                    (guint day, gboolean short_name);
gchar *     utl_get_julian_day_name             (guint32 julian);
gchar *     utl_get_date_name                   (const GDate *date);
gchar *     utl_get_date_name_format            (const GDate *date, gchar *fmt);
guint       utl_get_weekend_days_in_month       (const GDate *date);
guint       utl_get_weekend_days_in_month_my    (guint month, guint year);
guint       utl_get_days_per_year               (guint year);
void        utl_subtract_from_date              (guint32 date, gint time, gint days, gint seconds, guint32 *new_date, gint *new_time);
guint       utl_calc_moon_phase                 (const GDate *date);
gchar *     utl_get_moon_phase_name             (gint phase);

void        utl_xml_get_int                     (gchar *name, gint *iname, xmlNodePtr node);
void        utl_xml_get_uint                    (gchar *name, guint *uname, xmlNodePtr node);
void        utl_xml_get_char                    (gchar *name, gchar *cname, xmlNodePtr node);
void        utl_xml_get_str                     (gchar *name, gchar **sname, xmlNodePtr node);
void        utl_xml_get_strn                    (gchar *name, gchar *sname, gint buffer_size, xmlNodePtr node);
void        utl_xml_put_int                     (gchar *name, gint value, xmlNodePtr node);
void        utl_xml_put_uint                    (gchar *name, guint value, xmlNodePtr node);
void        utl_xml_put_char                    (gchar *name, gchar character, xmlNodePtr node, xmlDocPtr doc);
void        utl_xml_put_str                     (gchar *name, gchar *string, xmlNodePtr node, xmlDocPtr doc);
void        utl_xml_put_strn                    (gchar *name, gchar *string, gint buffer_size, xmlNodePtr node, xmlDocPtr doc);

void        utl_name_strcat                     (gchar *first, gchar *second, gchar *buffer);
gboolean    utl_run_helper                      (gchar *parameter, gint helper);
void        utl_run_command                     (gchar *command, gboolean sync);
gboolean    utl_is_valid_command                (gchar *command);
gint        utl_get_link_type                   (gchar *link);

void        utl_cairo_set_color                 (cairo_t *cr, GdkColor *color, gint alpha);
void        utl_cairo_draw                      (cairo_t *cr, gint stroke);
void        utl_draw_rounded_rectangle          (cairo_t *cr, gint x, gint y, gint w, gint h, gint a, gint s);
void        utl_draw_left_arrow                 (cairo_t *cr, gdouble x, gdouble y, gdouble w, gdouble h, gdouble a);

void        utl_play_alarm_sound                (guint repetitions);
gchar *     utl_add_timestamp_to_filename       (gchar *filename, gchar *extension);

#endif /* _UTILS_H */

